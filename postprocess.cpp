#include "stdafx.h"
#include "postprocess.h"
#include "shaders.h"
#include "textures.h"

namespace postprocess
{
	struct Shockwave
	{
		sf::Vector2f center; // in target coordinates
		float force = 0.f;
		float size = 0.f;
		float thickness = 0.f;
	};

	std::vector<Shockwave> _shockwaves;

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
		for (const Shockwave& shockwave : _shockwaves) {
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

	void create_shockwave(const sf::Vector2f& center)
	{
		Shockwave shockwave{};
		shockwave.center = center;
		shockwave.force = 0.1f;
		shockwave.size = 0.1f;
		shockwave.thickness = 0.1f;
		_shockwaves.push_back(shockwave);
	}
}

