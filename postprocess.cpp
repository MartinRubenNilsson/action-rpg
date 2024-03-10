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

	void _render_shockwaves(std::unique_ptr<sf::RenderTexture>& texture)
	{
		std::shared_ptr<sf::Shader> shader = shaders::get("shockwave");
		if (!shader) return;
		const sf::Vector2u size = texture->getSize();
		for (const Shockwave& shockwave : _shockwaves) {
			shader->setUniform("resolution", sf::Vector2f(size));
			shader->setUniform("center", shockwave.world_position);
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

