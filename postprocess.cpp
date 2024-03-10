#include "stdafx.h"
#include "postprocess.h"
#include "shaders.h"
#include "textures.h"

namespace postprocess
{
	struct Shockwave
	{
		sf::Vector2f world_position; // in target coordinates
		float force = 0.f;
		float size = 0.f;
		float thickness = 0.f;
	};

	std::vector<Shockwave> _shockwaves;

	void update(float dt)
	{
		//TODO
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

	void render(std::unique_ptr<sf::RenderTexture>& texture)
	{
		_render_shockwaves(texture);
	}

	void create_shockwave(const sf::Vector2f& world_position)
	{
		Shockwave shockwave{};
		shockwave.world_position = world_position;
		shockwave.force = 0.1f;
		shockwave.size = 0.1f;
		shockwave.thickness = 0.1f;
		_shockwaves.push_back(shockwave);
	}
}

