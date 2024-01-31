#include "stdafx.h"
#include "postprocess.h"
#include "shaders.h"

namespace postprocess
{
	void copy(sf::RenderTarget& target, const sf::RenderTexture& source) {
		target.draw(sf::Sprite(source.getTexture()));
	}

	void shockwave(sf::RenderTarget& target, const sf::RenderTexture& source, float time, const sf::Vector2f& center)
	{
		std::shared_ptr<sf::Shader> shader = shaders::get("shockwave");
		if (!shader) return;
		shader->setUniform("resolution", sf::Vector2f(target.getSize()));
		shader->setUniform("time", time);
		shader->setUniform("center", center);
		target.draw(sf::Sprite(source.getTexture()), shader.get());
	}
}

