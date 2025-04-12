#include "stdafx.h"
#include "postprocessing.h"
#include "graphics.h"
#include "graphics_globals.h"

namespace postprocessing {
	struct Shockwave {
		Vector2f position_ws; // ws = world space
		float force = 0.f;
		float size = 0.f;
		float thickness = 0.f;
	};

	const size_t MAX_GAUSSIAN_BLUR_ITERATIONS = 5;

	std::vector<Shockwave> _shockwaves;
	float _darkness_intensity = 0.f;
	Vector2f _darkness_center_ws; // ws = world space
	float _screen_transition_progress = 0.f;
	size_t _gaussian_blur_iterations = 0;

	void _update_shockwaves(float dt) {
		//TODO: this probably belongs in the ecs
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

	void update(float dt) {
		_update_shockwaves(dt);
	}

	Vector2f _map_world_to_target(
		const Vector2f& pos_ws,
		const Vector2f& camera_min_ws,
		const Vector2f& camera_max_ws,
		unsigned int target_width,
		unsigned int target_height) {
		const float x = (pos_ws.x - camera_min_ws.x) / (camera_max_ws.x - camera_min_ws.x) * target_width;
		const float y = (pos_ws.y - camera_min_ws.y) / (camera_max_ws.y - camera_min_ws.y) * target_height;
		return Vector2f(x, target_height - y);
	}

	void _render_shockwaves(const Vector2f& camera_min, const Vector2f& camera_max) {
#if 0
		if (_shockwaves.empty()) return;
		if (graphics::shockwave_shader == Handle<graphics::Shader>()) return;

		graphics::ScopedDebugGroup debug_group("postprocessing::_render_shockwaves()");

		// Bind some shader uniforms
		graphics::bind_shader(graphics::shockwave_shader);
		graphics::set_uniform_2f(graphics::shockwave_shader, "resolution",
			GAME_FRAMEBUFFER_WIDTH, GAME_FRAMEBUFFER_HEIGHT);

		for (const Shockwave& shockwave : _shockwaves) {
			std::swap(graphics::game_framebuffer_0, graphics::game_framebuffer_1);
			const Vector2f position_ts = _map_world_to_target(
				shockwave.position_ws, camera_min, camera_max,
				GAME_FRAMEBUFFER_WIDTH, GAME_FRAMEBUFFER_HEIGHT);
			graphics::set_uniform_2f(graphics::shockwave_shader, "center", position_ts.x, position_ts.y);
			graphics::set_uniform_1f(graphics::shockwave_shader, "force", shockwave.force);
			graphics::set_uniform_1f(graphics::shockwave_shader, "size", shockwave.size);
			graphics::set_uniform_1f(graphics::shockwave_shader, "thickness", shockwave.thickness);
			graphics::bind_texture(0, graphics::get_framebuffer_texture(graphics::game_framebuffer_1));
			graphics::bind_framebuffer(graphics::game_framebuffer_0);
			graphics::draw(3); // draw a fullscreen-covering triangle
		}
#endif
	}

	void _render_lighting(const Vector2f& camera_min, const Vector2f& camera_max) {
		if (_darkness_intensity == 0.f) return;
		graphics::ScopedDebugGroup debug_group("postprocessing::_render_lighting()");

#if 0
		// Load shader
		const Handle<graphics::Shader> shader = graphics::load_shader(
			"assets/shaders/fullscreen.vert",
			"assets/shaders/darkness.frag");
		if (shader == Handle<graphics::Shader>()) return;

		std::swap(graphics::game_framebuffer_0, graphics::game_framebuffer_1);
		const Vector2f center_ts = _map_world_to_target(
			_darkness_center_ws, camera_min, camera_max,
			GAME_FRAMEBUFFER_WIDTH, GAME_FRAMEBUFFER_HEIGHT);
		graphics::bind_shader(shader);
		graphics::set_uniform_2f(shader, "resolution",
			GAME_FRAMEBUFFER_WIDTH, GAME_FRAMEBUFFER_HEIGHT);
		graphics::set_uniform_2f(shader, "center", center_ts.x, center_ts.y);
		graphics::set_uniform_1f(shader, "intensity", _darkness_intensity);
		graphics::bind_texture(0, graphics::get_framebuffer_texture(graphics::game_framebuffer_1));
		graphics::bind_framebuffer(graphics::game_framebuffer_0);
		graphics::draw(3); // draw a fullscreen-covering triangle
#endif
	}

	void _render_screen_transition() {
#if 0
		if (_screen_transition_progress == 0.f) return;
		if (graphics::screen_transition_shader == Handle<graphics::Shader>()) return;

		graphics::ScopedDebugGroup debug_group("postprocessing::_render_screen_transition()");

		std::swap(graphics::game_framebuffer_0, graphics::game_framebuffer_1);
		graphics::bind_shader(graphics::screen_transition_shader);
		graphics::set_uniform_1f(graphics::screen_transition_shader, "progress", _screen_transition_progress);
		graphics::bind_texture(0, graphics::get_framebuffer_texture(graphics::game_framebuffer_1));
		graphics::bind_framebuffer(graphics::game_framebuffer_0);
		graphics::draw(graphics::Primitives::TriangleList, 3); // draw a fullscreen-covering triangle
#endif
	}

	void _render_gaussian_blur() {
		if (_gaussian_blur_iterations == 0) return;
		if (graphics::gaussian_blur_hor_shader == Handle<graphics::Shader>()) return;
		if (graphics::gaussian_blur_ver_shader == Handle<graphics::Shader>()) return;

		graphics::ScopedDebugGroup debug_group("postprocessing::_render_gaussian_blur()");

		graphics::bind_sampler(0, graphics::linear_sampler);

		for (size_t i = 0; i < _gaussian_blur_iterations; ++i) {

			// Horizontal pass
			std::swap(graphics::game_framebuffer_0, graphics::game_framebuffer_1);
			graphics::bind_shader(graphics::gaussian_blur_hor_shader);
			graphics::bind_texture(0, graphics::get_framebuffer_texture(graphics::game_framebuffer_1));
			graphics::bind_framebuffer(graphics::game_framebuffer_0);
			graphics::draw(3); // draw a fullscreen-covering triangle

			// Vertical pass
			std::swap(graphics::game_framebuffer_0, graphics::game_framebuffer_1);
			graphics::bind_shader(graphics::gaussian_blur_ver_shader);
			graphics::bind_texture(0, graphics::get_framebuffer_texture(graphics::game_framebuffer_1));
			graphics::bind_framebuffer(graphics::game_framebuffer_0);
			graphics::draw(3); // draw a fullscreen-covering triangle
		}

		graphics::bind_sampler(0, graphics::nearest_sampler);
	}

	void render(const Vector2f& camera_min, const Vector2f& camera_max) {
		graphics::ScopedDebugGroup debug_group("postprocessing::render()");
		graphics::set_primitives(graphics::Primitives::TriangleList);
		_render_shockwaves(camera_min, camera_max);
		_render_lighting(camera_min, camera_max);
		_render_screen_transition();
		_render_gaussian_blur();
	}

	void add_shockwave(const Vector2f& position_ws) {
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
