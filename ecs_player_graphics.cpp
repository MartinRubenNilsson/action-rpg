#include "stdafx.h"
#include "ecs_player_graphics.h"

namespace ecs
{
	std::shared_ptr<sf::Texture> bake_player_texture(const PlayerGraphics& graphics)
	{
		return nullptr;
		sf::RenderTexture render_texture;
		render_texture.create(1024, 1024);
		//render_texture->clear(sf::Color::Transparent);
		render_texture.clear(sf::Color::Red);
		//TODO
		render_texture.display();
		return std::make_shared<sf::Texture>(render_texture.getTexture());
	}
}