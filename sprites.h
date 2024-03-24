#pragma once

namespace sprites
{
	enum SpriteFlags : uint8_t
	{
		SF_NONE          = 0,
		SF_FLIP_X        = 1 << 0,
		SF_FLIP_Y        = 1 << 1,
		SF_FLIP_DIAGONAL = 1 << 2,
	};

	struct Sprite
	{
		sf::Vector2f min; // top-left corner
		sf::Vector2f max; // bottom-right corner
		sf::Vector2f tex_min; // top-left texture coordinates in pixels
		sf::Vector2f tex_max; // bottom-right texture coordinates in pixels
		sf::Vector2f sorting_position;
		std::shared_ptr<sf::Texture> texture;
		std::shared_ptr<sf::Shader> shader;
		sf::Color color = sf::Color::White;
		uint8_t sorting_layer = 0;
		uint8_t flags = 0;
	};

	bool operator<(const Sprite& left, const Sprite& right);
}