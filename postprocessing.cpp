#include "stdafx.h"
#include "postprocessing.h"
#include "graphics.h"

namespace postprocessing
{
	struct Shockwave
	{
		sf::Vector2f position_ws; // ws = world space
		float force = 0.f;
		float size = 0.f;
		float thickness = 0.f;
	};

	const size_t MAX_GAUSSIAN_BLUR_ITERATIONS = 5;

	float _pixel_scale = 1.f;
	std::vector<Shockwave> _shockwaves;
	float _darkness_intensity = 0.f;
	sf::Vector2f _darkness_center_ws; // ws = world space
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

	sf::Vector2f _map_world_to_target(const sf::View& view, const sf::Vector2u& size, const sf::Vector2f& pos)
	{
		// Convert to normalized device coordinates
		sf::Vector2f p = view.getTransform().transformPoint(pos);
		// Convert to viewport coordinates
		p.x = (p.x + 1.f) * 0.5f;
		p.y = (1.f - p.y) * 0.5f;
		// Convert to target coordinates
		const sf::FloatRect& viewport = view.getViewport();
		p.x = (viewport.left + p.x * viewport.width) * size.x;
		p.y = (viewport.top + p.y * viewport.height) * size.y;
		return p;
	}

#if 0
	int _render_shockwaves(int render_target_id)
	{
		if (_shockwaves.empty()) return;
		const int shader_id = graphics::load_shader({}, "assets/shaders/shockwave.frag");
		if (shader_id == -1) return;
		const sf::View view = texture->getView();
		const sf::Vector2u size = texture->getSize();
		graphics::bind_shader(shader_id);
		for (const Shockwave& shockwave : _shockwaves) {
			const sf::Vector2f position_ts = _map_world_to_target(view, size, shockwave.position_ws);
			graphics::set_shader_uniform_2f(shader_id, "resolution", (float)size.x, (float)size.y);
			graphics::set_shader_uniform_2f(shader_id, "center", position_ts.x, position_ts.y);
			graphics::set_shader_uniform_1f(shader_id, "force", shockwave.force);
			graphics::set_shader_uniform_1f(shader_id, "size", shockwave.size);
			graphics::set_shader_uniform_1f(shader_id, "thickness", shockwave.thickness);
			std::unique_ptr<sf::RenderTexture> target_texture =
				textures::take_render_texture_from_pool(size);
			target_texture->setView(target_texture->getDefaultView());
			target_texture->draw(sf::Sprite(texture->getTexture()));
			target_texture->display();
			textures::give_render_texture_to_pool(std::move(texture));
			texture = std::move(target_texture);
		}
		graphics::unbind_shader();
		texture->setView(view);
	}
#endif

#if 0
	int _render_darkness(int render_target_id)
	{
		if (_darkness_intensity == 0.f) return;
		const int shader_id = graphics::load_shader({}, "assets/shaders/darkness.frag");
		const sf::View view = texture->getView();
		const sf::Vector2u size = texture->getSize();
		const sf::Vector2f center_ts = _map_world_to_target(view, size, _darkness_center_ws);
		graphics::bind_shader(shader_id);
		graphics::set_shader_uniform_2f(shader_id, "resolution", (float)size.x, (float)size.y);
		graphics::set_shader_uniform_2f(shader_id, "center", center_ts.x, center_ts.y);
		graphics::set_shader_uniform_1f(shader_id, "intensity", _darkness_intensity);
		std::unique_ptr<sf::RenderTexture> target_texture =
			textures::take_render_texture_from_pool(size);
		target_texture->setView(target_texture->getDefaultView());
		target_texture->draw(sf::Sprite(texture->getTexture()));
		target_texture->display();
		graphics::unbind_shader();
		textures::give_render_texture_to_pool(std::move(texture));
		texture = std::move(target_texture);
		texture->setView(view); // Restore view
	}
#endif

	int _render_screen_transition(int render_target_id)
	{
		if (_screen_transition_progress == 0.f) return render_target_id;
		const int shader_id = graphics::load_shader({}, "assets/shaders/screen_transition.frag");
		if (shader_id == -1) return render_target_id;
		const int texture_id = graphics::get_render_target_texture(render_target_id);
		unsigned int width, height;
		graphics::get_texture_size(texture_id, width, height);
		const int new_render_target_id = graphics::acquire_pooled_render_target(width, height);
		graphics::bind_shader(shader_id);
		graphics::set_shader_uniform_1i(shader_id, "tex", 0);
		graphics::set_shader_uniform_1f(shader_id, "pixel_scale", _pixel_scale);
		graphics::set_shader_uniform_1f(shader_id, "progress", _screen_transition_progress);
		graphics::bind_texture(0, texture_id);
		graphics::bind_render_target(new_render_target_id);
		graphics::draw_triangle_strip(graphics::FULLSCREEN_QUAD_VERTICES, 4);
		return new_render_target_id;
	}

	int _render_gaussian_blur(int render_target_id)
	{
		if (_gaussian_blur_iterations == 0) return render_target_id;

		// Load shaders
		const int shader_hor_id = graphics::load_shader(
			"assets/shaders/fullscreen.vert", "assets/shaders/gaussian_blur_hor.frag");
		if (shader_hor_id == -1) return render_target_id;
		const int shader_ver_id = graphics::load_shader(
			"assets/shaders/fullscreen.vert", "assets/shaders/gaussian_blur_ver.frag");
		if (shader_ver_id == -1) return render_target_id;

		// Get texture
		const int texture_id = graphics::get_render_target_texture(render_target_id);
		unsigned int width, height;
		graphics::get_texture_size(texture_id, width, height);

		// Aquire intermediate render target
		const int intermediate_render_target_id = graphics::acquire_pooled_render_target(width, height);
		const int intermediate_texture_id = graphics::get_render_target_texture(intermediate_render_target_id);

		// Set linear filtering
		graphics::set_texture_filter(texture_id, graphics::TextureFilter::Linear);
		graphics::set_texture_filter(intermediate_texture_id, graphics::TextureFilter::Linear);

		// Apply blur
		for (size_t i = 0; i < _gaussian_blur_iterations; ++i) {

			// Horizontal pass
			graphics::bind_shader(shader_hor_id);
			graphics::set_shader_uniform_1i(shader_hor_id, "tex", 0);
			graphics::set_shader_uniform_2f(shader_hor_id, "tex_size", (float)width, (float)height);
			graphics::bind_texture(0, texture_id);
			graphics::bind_render_target(intermediate_render_target_id);
			graphics::draw_triangle_strip(4);

			// Vertical pass
			graphics::bind_shader(shader_ver_id);
			graphics::set_shader_uniform_1i(shader_ver_id, "tex", 0);
			graphics::set_shader_uniform_2f(shader_ver_id, "tex_size", (float)width, (float)height);
			graphics::bind_texture(0, intermediate_texture_id);
			graphics::bind_render_target(render_target_id);
			graphics::draw_triangle_strip(4);
		}

		// Cleanup
		graphics::set_texture_filter(texture_id, graphics::TextureFilter::Nearest);
		graphics::set_texture_filter(intermediate_texture_id, graphics::TextureFilter::Nearest);
		graphics::release_pooled_render_target(intermediate_render_target_id);

		return render_target_id;
	}

	int render(int render_target_id)
	{
		//render_target_id = _render_shockwaves(render_target_id);
		//render_target_id = _render_darkness(render_target_id);
		//render_target_id = _render_screen_transition(render_target_id);
		render_target_id = _render_gaussian_blur(render_target_id);
		return render_target_id;
	}

	void set_pixel_scale(float scale) {
		_pixel_scale = std::max(scale, 0.1f);
	}

	void create_shockwave(const sf::Vector2f& position_ws)
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

	void set_darkness_center(const sf::Vector2f& position_in_world_space) {
		_darkness_center_ws = position_in_world_space;
	}

	void set_screen_transition_progress(float progress) {
		_screen_transition_progress = std::clamp(progress, -1.f, 1.f);
	}

	void set_gaussian_blur_iterations(size_t iterations) {
		_gaussian_blur_iterations = std::min(iterations, MAX_GAUSSIAN_BLUR_ITERATIONS);
	}
}

