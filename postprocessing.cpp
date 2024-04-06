#include "stdafx.h"
#include "postprocessing.h"
#include "shaders.h"
#include "textures.h"

namespace postprocessing
{
	struct Shockwave
	{
		sf::Vector2f world_position;
		float force = 0.f;
		float size = 0.f;
		float thickness = 0.f;
	};

	const size_t MAX_GAUSSIAN_BLUR_ITERATIONS = 5;
	size_t _gaussian_blur_iterations = 0;
	std::vector<Shockwave> _shockwaves;

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

	void _render_shockwaves(std::unique_ptr<sf::RenderTexture>& texture)
	{
		if (_shockwaves.empty()) return;
		std::shared_ptr<sf::Shader> shader = shaders::get("shockwave");
		if (!shader) return;
		const sf::View view = texture->getView();
		const sf::Vector2u size = texture->getSize();
		for (const Shockwave& shockwave : _shockwaves) {
			shader->setUniform("resolution", sf::Vector2f(size));
			shader->setUniform("center",
				_map_world_to_target(view, size, shockwave.world_position));
			shader->setUniform("force", shockwave.force);
			shader->setUniform("size", shockwave.size);
			shader->setUniform("thickness", shockwave.thickness);
			std::unique_ptr<sf::RenderTexture> target =
				textures::take_render_texture_from_pool(size);
			target->setView(target->getDefaultView());
			target->draw(sf::Sprite(texture->getTexture()), shader.get());
			target->display();
			textures::give_render_texture_to_pool(std::move(texture));
			texture = std::move(target);
		}
		texture->setView(view);
	}

	void _render_gaussian_blur(std::unique_ptr<sf::RenderTexture>& texture)
	{
		if (_gaussian_blur_iterations == 0) return;
		std::shared_ptr<sf::Shader> shader_hor = shaders::get("gaussian_blur_hor");
		if (!shader_hor) return;
		std::shared_ptr<sf::Shader> shader_ver = shaders::get("gaussian_blur_ver");
		if (!shader_ver) return;
		const sf::Vector2 size = texture->getSize();
		// Set uniforms
		shader_hor->setUniform("tex_size", sf::Vector2f(size));
		shader_ver->setUniform("tex_size", sf::Vector2f(size));
		std::unique_ptr<sf::RenderTexture> intermediate_texture =
			textures::take_render_texture_from_pool(size);
		// Set linear filtering
		const bool texture_was_smooth = texture->getTexture().isSmooth();
		const bool intermediate_texture_was_smooth = intermediate_texture->getTexture().isSmooth();
		texture->setSmooth(true);
		intermediate_texture->setSmooth(true);
		// Apply blur
		for (size_t i = 0; i < _gaussian_blur_iterations; ++i) {
			// Horizontal pass
			intermediate_texture->setView(intermediate_texture->getDefaultView());
			intermediate_texture->draw(sf::Sprite(texture->getTexture()), shader_hor.get());
			intermediate_texture->display();
			// Vertical pass
			texture->setView(texture->getDefaultView());
			texture->draw(sf::Sprite(intermediate_texture->getTexture()), shader_ver.get());
			texture->display();
		}
		// Cleanup
		texture->setSmooth(texture_was_smooth);
		intermediate_texture->setSmooth(intermediate_texture_was_smooth);
		textures::give_render_texture_to_pool(std::move(intermediate_texture));
	}

	void render(std::unique_ptr<sf::RenderTexture>& texture)
	{
		_render_shockwaves(texture);
		_render_gaussian_blur(texture);
	}

	void create_shockwave(const sf::Vector2f& world_position)
	{
		Shockwave shockwave{};
		shockwave.world_position = world_position;
		shockwave.force = 0.2f;
		shockwave.size = 0.f;
		shockwave.thickness = 0.f;
		_shockwaves.push_back(shockwave);
	}

	void set_gaussian_blur_iterations(size_t iterations) {
		_gaussian_blur_iterations = std::min(iterations, MAX_GAUSSIAN_BLUR_ITERATIONS);
	}
}

