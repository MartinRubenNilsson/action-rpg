#include "stdafx.h"
#include "postprocessing.h"
#include "graphics.h"
#include "graphics_globals.h"

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

	void _render_shockwaves(const Vector2f& camera_min, const Vector2f& camera_max)
	{
		if (_shockwaves.empty()) return;
		graphics::ScopedDebugGroup debug_group("postprocessing::_render_shockwaves()");

		// Load shader
		const Handle<graphics::Shader> shader = graphics::load_shader(
			"assets/shaders/fullscreen.vert",
			"assets/shaders/shockwave.frag");
		if (shader == Handle<graphics::Shader>()) return;

		// Bind some shader uniforms
		graphics::bind_shader(shader);
		graphics::set_uniform_2f(shader, "resolution",
			GAMEWORLD_FRAMEBUFFER_WIDTH, GAMEWORLD_FRAMEBUFFER_HEIGHT);

		for (const Shockwave& shockwave : _shockwaves) {

			const Vector2f position_ts = _map_world_to_target(
				shockwave.position_ws, camera_min, camera_max,
				GAMEWORLD_FRAMEBUFFER_WIDTH, GAMEWORLD_FRAMEBUFFER_HEIGHT);
			graphics::set_uniform_2f(shader, "center", position_ts.x, position_ts.y);
			graphics::set_uniform_1f(shader, "force", shockwave.force);
			graphics::set_uniform_1f(shader, "size", shockwave.size);
			graphics::set_uniform_1f(shader, "thickness", shockwave.thickness);
			graphics::bind_texture(0, graphics::get_framebuffer_texture(graphics::gameworld_framebuffer_source));
			graphics::bind_framebuffer(graphics::gameworld_framebuffer_target);
			graphics::draw(graphics::Primitives::TriangleList, 3); // draw a fullscreen-covering triangle
			std::swap(graphics::gameworld_framebuffer_source, graphics::gameworld_framebuffer_target);
		}
	}

	void _render_darkness(const Vector2f& camera_min, const Vector2f& camera_max)
	{
		if (_darkness_intensity == 0.f) return;
		graphics::ScopedDebugGroup debug_group("postprocessing::_render_darkness()");

		// Load shader
		const Handle<graphics::Shader> shader = graphics::load_shader(
			"assets/shaders/fullscreen.vert",
			"assets/shaders/darkness.frag");
		if (shader == Handle<graphics::Shader>()) return;

		const Vector2f center_ts = _map_world_to_target(
			_darkness_center_ws, camera_min, camera_max,
			GAMEWORLD_FRAMEBUFFER_WIDTH, GAMEWORLD_FRAMEBUFFER_HEIGHT);
		graphics::bind_shader(shader);
		graphics::set_uniform_2f(shader, "resolution",
			GAMEWORLD_FRAMEBUFFER_WIDTH, GAMEWORLD_FRAMEBUFFER_HEIGHT);
		graphics::set_uniform_2f(shader, "center", center_ts.x, center_ts.y);
		graphics::set_uniform_1f(shader, "intensity", _darkness_intensity);
		graphics::bind_texture(0, graphics::get_framebuffer_texture(graphics::gameworld_framebuffer_source));
		graphics::bind_framebuffer(graphics::gameworld_framebuffer_target);
		graphics::draw(graphics::Primitives::TriangleList, 3); // draw a fullscreen-covering triangle
		std::swap(graphics::gameworld_framebuffer_source, graphics::gameworld_framebuffer_target);
	}

	void _render_screen_transition()
	{
		if (_screen_transition_progress == 0.f) return;
		graphics::ScopedDebugGroup debug_group("postprocessing::_render_screen_transition()");

		const Handle<graphics::Shader> shader = graphics::load_shader(
			"assets/shaders/fullscreen.vert",
			"assets/shaders/screen_transition.frag");
		if (shader == Handle<graphics::Shader>()) return;

		graphics::bind_shader(shader);
		graphics::set_uniform_1f(shader, "pixel_scale", _pixel_scale);
		graphics::set_uniform_1f(shader, "progress", _screen_transition_progress);
		graphics::bind_texture(0, graphics::get_framebuffer_texture(graphics::gameworld_framebuffer_source));
		graphics::bind_framebuffer(graphics::gameworld_framebuffer_target);
		graphics::draw(graphics::Primitives::TriangleList, 3); // draw a fullscreen-covering triangle
		std::swap(graphics::gameworld_framebuffer_source, graphics::gameworld_framebuffer_target);
	}

	void _render_gaussian_blur()
	{
		if (_gaussian_blur_iterations == 0) return;
		graphics::ScopedDebugGroup debug_group("postprocessing::_render_gaussian_blur()");

		// Load shaders
		const Handle<graphics::Shader> shader_hor = graphics::load_shader(
			"assets/shaders/fullscreen.vert",
			"assets/shaders/gaussian_blur_hor.frag");
		if (shader_hor == Handle<graphics::Shader>()) return;
		const Handle<graphics::Shader> shader_ver = graphics::load_shader(
			"assets/shaders/fullscreen.vert",
			"assets/shaders/gaussian_blur_ver.frag");
		if (shader_ver == Handle<graphics::Shader>()) return;

		Handle<graphics::Texture> source_texture = graphics::get_framebuffer_texture(graphics::gameworld_framebuffer_source);
		Handle<graphics::Texture> target_texture = graphics::get_framebuffer_texture(graphics::gameworld_framebuffer_target);

		// Set linear filtering
		graphics::set_texture_filter(source_texture, graphics::Filter::Linear);
		graphics::set_texture_filter(target_texture, graphics::Filter::Linear);

		graphics::set_uniform_2f(shader_hor, "tex_size", GAMEWORLD_FRAMEBUFFER_WIDTH, GAMEWORLD_FRAMEBUFFER_HEIGHT);
		graphics::set_uniform_2f(shader_ver, "tex_size", GAMEWORLD_FRAMEBUFFER_WIDTH, GAMEWORLD_FRAMEBUFFER_HEIGHT);

		// Apply blur
		for (size_t i = 0; i < _gaussian_blur_iterations; ++i) {

			// Horizontal pass
			graphics::bind_shader(shader_hor);
			graphics::bind_texture(0, source_texture);
			graphics::bind_framebuffer(graphics::gameworld_framebuffer_target);
			graphics::draw(graphics::Primitives::TriangleList, 3); // draw a fullscreen-covering triangle

			// Vertical pass
			graphics::bind_shader(shader_ver);
			graphics::bind_texture(0, target_texture);
			graphics::bind_framebuffer(graphics::gameworld_framebuffer_source);
			graphics::draw(graphics::Primitives::TriangleList, 3); // draw a fullscreen-covering triangle
		}

		// Restore nearest filtering
		graphics::set_texture_filter(source_texture, graphics::Filter::Nearest);
		graphics::set_texture_filter(target_texture, graphics::Filter::Nearest);
	}

	void render(const Vector2f& camera_min, const Vector2f& camera_max)
	{
		graphics::ScopedDebugGroup debug_group("postprocessing::render()");
		_render_shockwaves(camera_min, camera_max);
		_render_darkness(camera_min, camera_max);
		_render_screen_transition();
		_render_gaussian_blur();
	}

	void set_pixel_scale(float scale)
	{
		_pixel_scale = std::max(scale, 0.1f);
	}

	void add_shockwave(const Vector2f& position_ws)
	{
		Shockwave shockwave{};
		shockwave.position_ws = position_ws; // ws = world space
		shockwave.force = 0.2f;
		shockwave.size = 0.f;
		shockwave.thickness = 0.f;
		_shockwaves.push_back(shockwave);
	}

	void set_darkness_intensity(float intensity)
	{
		_darkness_intensity = std::clamp(intensity, 0.f, 1.f);
	}

	void set_darkness_center(const Vector2f& position_in_world_space)
	{
		_darkness_center_ws = position_in_world_space;
	}

	void set_screen_transition_progress(float progress)
	{
		_screen_transition_progress = std::clamp(progress, -1.f, 1.f);
	}

	void set_gaussian_blur_iterations(size_t iterations)
	{
		_gaussian_blur_iterations = std::min(iterations, MAX_GAUSSIAN_BLUR_ITERATIONS);
	}
}

