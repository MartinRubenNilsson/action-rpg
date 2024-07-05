#include "stdafx.h"
#include "postprocessing.h"
#include "graphics.h"

namespace postprocessing
{
	struct Shockwave
	{
		Vector2f position_ws; // ws = world space
		float force = 0.f;
		float size = 0.f;
		float thickness = 0.f;
	};

	const size_t MAX_GAUSSIAN_BLUR_ITERATIONS = 5;

	float _pixel_scale = 1.f;
	std::vector<Shockwave> _shockwaves;
	float _darkness_intensity = 0.f;
	Vector2f _darkness_center_ws; // ws = world space
	float _screen_transition_progress = 0.f;
	size_t _gaussian_blur_iterations = 0;

	void _update_shockwaves(float dt)
	{
		for (auto it = _shockwaves.begin(); it != _shockwaves.end();) {
			it->force -= dt * 0.4f;
			it->size += dt * 0.7f;
			it->thickness += dt * 0.2f;
			if (it->force <= 0.f) {
				it = _shockwaves.erase(it);
			} else {
				++it;
			}
		}
	}

	void update(float dt)
	{
		_update_shockwaves(dt);
	}

	Vector2f _map_world_to_target(
		const Vector2f& pos_ws,
		const Vector2f& camera_min_ws,
		const Vector2f& camera_max_ws,
		unsigned int target_width,
		unsigned int target_height)
	{
		const float x = (pos_ws.x - camera_min_ws.x) / (camera_max_ws.x - camera_min_ws.x) * target_width;
		const float y = (pos_ws.y - camera_min_ws.y) / (camera_max_ws.y - camera_min_ws.y) * target_height;
		return Vector2f(x, target_height - y);
	}

	void _render_shockwaves(graphics::RenderTargetHandle& target, const Vector2f& camera_min, const Vector2f& camera_max)
	{
		if (_shockwaves.empty()) return;
		graphics::ScopedDebugGroup debug_group("Shockwaves");

		// Load shader
		const graphics::ShaderHandle shader = graphics::load_shader(
			"assets/shaders/fullscreen_triangle.vert",
			"assets/shaders/shockwave.frag");
		if (shader == graphics::ShaderHandle::Invalid) return;

		// Get texture
		graphics::TextureHandle texture = graphics::get_render_target_texture(target);
		unsigned int width, height;
		graphics::get_texture_size(texture, width, height);

		// Bind some shader uniforms
		graphics::bind_shader(shader);
		graphics::set_uniform_1i(shader, "tex", 0);
		graphics::set_uniform_2f(shader, "resolution", (float)width, (float)height);

		for (const Shockwave& shockwave : _shockwaves) {

			// Aquire intermediate render target
			const graphics::RenderTargetHandle intermediate_target =
				graphics::acquire_pooled_render_target(width, height);

			// Render shockwave
			const Vector2f position_ts = _map_world_to_target(
				shockwave.position_ws, camera_min, camera_max, width, height);
			graphics::set_uniform_2f(shader, "center", position_ts.x, position_ts.y);
			graphics::set_uniform_1f(shader, "force", shockwave.force);
			graphics::set_uniform_1f(shader, "size", shockwave.size);
			graphics::set_uniform_1f(shader, "thickness", shockwave.thickness);
			graphics::bind_texture(0, texture);
			graphics::bind_render_target(intermediate_target);
			graphics::draw_triangles(3);

			// Interchange render targets
			graphics::release_pooled_render_target(target);
			target = intermediate_target;
			texture = graphics::get_render_target_texture(target);
		}
	}

	void _render_darkness(graphics::RenderTargetHandle& target, const Vector2f& camera_min, const Vector2f& camera_max)
	{
		if (_darkness_intensity == 0.f) return;
		graphics::ScopedDebugGroup debug_group("Darkness");

		// Load shader
		const graphics::ShaderHandle shader = graphics::load_shader(
			"assets/shaders/fullscreen_triangle.vert",
			"assets/shaders/darkness.frag");
		if (shader == graphics::ShaderHandle::Invalid) return;

		// Get texture
		const graphics::TextureHandle texture = graphics::get_render_target_texture(target);
		unsigned int width, height;
		graphics::get_texture_size(texture, width, height);

		// Aquire intermediate render target
		const graphics::RenderTargetHandle intermediate_target =
			graphics::acquire_pooled_render_target(width, height);

		const Vector2f center_ts = _map_world_to_target(
			_darkness_center_ws, camera_min, camera_max, width, height);
		graphics::bind_shader(shader);
		graphics::set_uniform_1i(shader, "tex", 0);
		graphics::set_uniform_2f(shader, "resolution", (float)width, (float)height);
		graphics::set_uniform_2f(shader, "center", center_ts.x, center_ts.y);
		graphics::set_uniform_1f(shader, "intensity", _darkness_intensity);
		graphics::bind_texture(0, texture);
		graphics::bind_render_target(intermediate_target);
		graphics::draw_triangles(3);

		// Cleanup
		graphics::release_pooled_render_target(target);
		target = intermediate_target;
	}

	void _render_screen_transition(graphics::RenderTargetHandle& target)
	{
		if (_screen_transition_progress == 0.f) return;
		graphics::ScopedDebugGroup debug_group("Screen transition");

		// Load shader
		const graphics::ShaderHandle shader = graphics::load_shader(
			"assets/shaders/fullscreen_triangle.vert",
			"assets/shaders/screen_transition.frag");
		if (shader == graphics::ShaderHandle::Invalid) return;

		// Get texture
		const graphics::TextureHandle texture = graphics::get_render_target_texture(target);
		unsigned int width, height;
		graphics::get_texture_size(texture, width, height);

		// Aquire intermediate render target
		const graphics::RenderTargetHandle intermediate_target =
			graphics::acquire_pooled_render_target(width, height);

		// Render screen transition
		graphics::bind_shader(shader);
		graphics::set_uniform_1i(shader, "tex", 0);
		graphics::set_uniform_1f(shader, "pixel_scale", _pixel_scale);
		graphics::set_uniform_1f(shader, "progress", _screen_transition_progress);
		graphics::bind_texture(0, texture);
		graphics::bind_render_target(intermediate_target);
		graphics::draw_triangles(3);

		// Cleanup
		graphics::release_pooled_render_target(target);
		target = intermediate_target;
	}

	void _render_gaussian_blur(graphics::RenderTargetHandle& target)
	{
		if (_gaussian_blur_iterations == 0) return;
		graphics::ScopedDebugGroup debug_group("Gaussian blur");

		// Load shaders
		const graphics::ShaderHandle shader_hor = graphics::load_shader(
			"assets/shaders/fullscreen_triangle.vert",
			"assets/shaders/gaussian_blur_hor.frag");
		if (shader_hor == graphics::ShaderHandle::Invalid) return;
		const graphics::ShaderHandle shader_ver = graphics::load_shader(
			"assets/shaders/fullscreen_triangle.vert",
			"assets/shaders/gaussian_blur_ver.frag");
		if (shader_ver == graphics::ShaderHandle::Invalid) return;

		// Get texture
		const graphics::TextureHandle texture = graphics::get_render_target_texture(target);
		unsigned int width, height;
		graphics::get_texture_size(texture, width, height);

		// Aquire intermediate render target
		const graphics::RenderTargetHandle intermediate_target =
			graphics::acquire_pooled_render_target(width, height);
		const graphics::TextureHandle intermediate_texture =
			graphics::get_render_target_texture(intermediate_target);

		// Set linear filtering
		graphics::set_texture_filter(texture, graphics::TextureFilter::Linear);
		graphics::set_texture_filter(intermediate_texture, graphics::TextureFilter::Linear);

		// Apply blur
		for (size_t i = 0; i < _gaussian_blur_iterations; ++i) {

			// Horizontal pass
			graphics::bind_shader(shader_hor);
			graphics::set_uniform_1i(shader_hor, "tex", 0);
			graphics::set_uniform_2f(shader_hor, "tex_size", (float)width, (float)height);
			graphics::bind_texture(0, texture);
			graphics::bind_render_target(intermediate_target);
			graphics::draw_triangles(3);

			// Vertical pass
			graphics::bind_shader(shader_ver);
			graphics::set_uniform_1i(shader_ver, "tex", 0);
			graphics::set_uniform_2f(shader_ver, "tex_size", (float)width, (float)height);
			graphics::bind_texture(0, intermediate_texture);
			graphics::bind_render_target(target);
			graphics::draw_triangles(3);
		}

		// Cleanup
		graphics::set_texture_filter(texture, graphics::TextureFilter::Nearest);
		graphics::set_texture_filter(intermediate_texture, graphics::TextureFilter::Nearest);
		graphics::release_pooled_render_target(intermediate_target);
	}

	void render(graphics::RenderTargetHandle& target, const Vector2f& camera_min, const Vector2f& camera_max)
	{
		graphics::ScopedDebugGroup debug_group("Postprocessing");
		_render_shockwaves(target, camera_min, camera_max);
		_render_darkness(target, camera_min, camera_max);
		_render_screen_transition(target);
		_render_gaussian_blur(target);
	}

	void set_pixel_scale(float scale) {
		_pixel_scale = std::max(scale, 0.1f);
	}

	void create_shockwave(const Vector2f& position_ws)
	{
		Shockwave shockwave{};
		shockwave.position_ws = position_ws; // ws = world space
		shockwave.force = 0.2f;
		shockwave.size = 0.f;
		shockwave.thickness = 0.f;
		_shockwaves.push_back(shockwave);
	}

	void set_darkness_intensity(float intensity) {
		_darkness_intensity = std::clamp(intensity, 0.f, 1.f);
	}

	void set_darkness_center(const Vector2f& position_in_world_space) {
		_darkness_center_ws = position_in_world_space;
	}

	void set_screen_transition_progress(float progress) {
		_screen_transition_progress = std::clamp(progress, -1.f, 1.f);
	}

	void set_gaussian_blur_iterations(size_t iterations) {
		_gaussian_blur_iterations = std::min(iterations, MAX_GAUSSIAN_BLUR_ITERATIONS);
	}
}

