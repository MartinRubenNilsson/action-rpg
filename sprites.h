#pragma once

namespace sprites
{
	enum SpriteFlags : uint8_t
	{
		SF_NONE          = 0,
		SF_FLIP_X        = 1 << 0,
		SF_FLIP_Y        = 1 << 1,
		SF_FLIP_DIAGONAL = 1 << 2,
		SF_ROTATE_90     = SF_FLIP_X | SF_FLIP_DIAGONAL,
		SF_ROTATE_180    = SF_FLIP_X | SF_FLIP_Y,
		SF_ROTATE_270    = SF_FLIP_Y | SF_FLIP_DIAGONAL,
	};

	struct Sprite
	{
		std::shared_ptr<sf::Texture> texture; // required
		std::shared_ptr<sf::Shader> shader; // optional, if not set, the default shader is used
		sf::Vector2f min; // top-left corner world position in pixels
		sf::Vector2f max; // bottom-right corner world position in pixels
		sf::Vector2f tex_min; // top-left corner texture coordinates in pixels
		sf::Vector2f tex_max; // bottom-right corner texture coordinates in pixels
		sf::Vector2f sorting_pos; // world position in pixels used for sorting
		sf::Color color = sf::Color::White;
		uint8_t sorting_layer = 0;
		uint8_t flags = 0;
	};

	// Orders sprites by draw order. Members are compared in a cascading fashion:
	// 
	// 1. sorting_layer
	// 2. sorting_pos.y
	// 3. sorting_pos.x
	// 4. texture
	// 5. shader
	// 
	// In all cases, a less-than comparison is used. (In 4-5, the pointers are compared.)
	// If one case yields a tie, then the next case is considered, and so on.
	//
	bool operator<(const Sprite& left, const Sprite& right);

	void set_time(float time); // HACK: for shader uniforms
	void draw(const Sprite& sprite);
	void render(sf::RenderTarget& target);
}