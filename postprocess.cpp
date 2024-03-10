#include "stdafx.h"
#include "postprocess.h"
#include "shaders.h"
#include "textures.h"

namespace postprocess
{
	std::vector<Shockwave> shockwaves;

	void update(float dt)
	{
		//TODO
	}

	std::unique_ptr<sf::RenderTexture> _render_shockwaves(std::unique_ptr<sf::RenderTexture> source)
	{
		assert(source);
		std::shared_ptr<sf::Shader> shader = shaders::get("shockwave");
		if (!shader) return source;
		const sf::Vector2u size = source->getSize();
		for (const Shockwave& shockwave : shockwaves) {
			std::unique_ptr<sf::RenderTexture> target = textures::take_render_texture_from_pool(size);
			target->setView(target->getDefaultView());
			shader->setUniform("resolution", sf::Vector2f(size));
			shader->setUniform("center", shockwave.center);
			shader->setUniform("force", shockwave.force);
			shader->setUniform("size", shockwave.size);
			shader->setUniform("thickness", shockwave.thickness);
			target->draw(sf::Sprite(source->getTexture()), shader.get());
			target->display();
			textures::give_render_texture_to_pool(std::move(source));
			source = std::move(target);
		}
		return source;
	}

	std::unique_ptr<sf::RenderTexture> render(std::unique_ptr<sf::RenderTexture> source)
	{
		return _render_shockwaves(std::move(source));
	}
}

