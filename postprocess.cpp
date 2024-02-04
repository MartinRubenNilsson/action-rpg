#include "stdafx.h"
#include "postprocess.h"
#include "shaders.h"

namespace postprocess
{
	void copy(sf::RenderTarget& target, const sf::RenderTexture& source) {
		target.draw(sf::Sprite(source.getTexture()));
	}

	void shockwave(sf::RenderTarget& target, const sf::RenderTexture& source, const sf::Vector2f& center, float force, float size, float thickness)
	{
		std::shared_ptr<sf::Shader> shader = shaders::get("shockwave");
		if (!shader) return;
		shader->setUniform("resolution", sf::Vector2f(target.getSize()));
		shader->setUniform("center", center);
		shader->setUniform("force", force);
		shader->setUniform("size", size);
		shader->setUniform("thickness", thickness);
		target.draw(sf::Sprite(source.getTexture()), shader.get());
	}
}

