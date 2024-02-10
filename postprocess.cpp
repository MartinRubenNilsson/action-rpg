#include "stdafx.h"
#include "postprocess.h"
#include "shaders.h"

namespace postprocess
{
	std::vector<Shockwave> shockwaves;

	void render_copy(sf::RenderTarget& target, const sf::Texture& source){
		target.draw(sf::Sprite(source));
	}

	void render_shockwave(sf::RenderTarget& target, const sf::Texture& source, const Shockwave& shockwave)
	{
		std::shared_ptr<sf::Shader> shader = shaders::get("shockwave");
		if (!shader) return;
		shader->setUniform("resolution", sf::Vector2f(target.getSize()));
		shader->setUniform("center", shockwave.center);
		shader->setUniform("force", shockwave.force);
		shader->setUniform("size", shockwave.size);
		shader->setUniform("thickness", shockwave.thickness);
		target.draw(sf::Sprite(source), shader.get());
	}
}

