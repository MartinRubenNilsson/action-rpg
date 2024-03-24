#pragma once

namespace sprites
{
	enum SortingLayer : uint8_t
	{
		SL_BACKGROUND_1,
		SL_BACKGROUND_2,
		SL_OBJECTS,
		SL_FOREGROUND_1,
		SL_FOREGROUND_2,
		SL_COLLIDERS,
		SL_VFX,
	};

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
		sf::Texture* texture = nullptr; // required
		sf::Shader* shader = nullptr; // optional; if not set, the default shader is used
		sf::Vector2f min; // top-left corner world position in pixels
		sf::Vector2f max; // bottom-right corner world position in pixels
		sf::Vector2f tex_min; // top-left corner texture coordinates in pixels
		sf::Vector2f tex_max; // bottom-right corner texture coordinates in pixels
		sf::Vector2f sorting_pos;
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

	extern bool enable_batching;

	void set_time(float time); // HACK: for shader uniforms

	// Submits a sprite for rendering. The sprite is not rendered immediately, but
	// instead copied to a list of sprites to be rendered when render() is called.
	void draw(const Sprite& sprite);
	// Renders all sprites submitted since the last call to render().
	void render(sf::RenderTarget& target);
	// Returns the number of sprites rendered during the last call to render().
	// This is also equal to the number of sprites submitted before the last call.
	uint32_t get_sprite_draw_count(); 
	// Returns the number of sprite batches rendered during the last call to render().
	// This will typically be less than get_submitted_sprite_count() and never greater.
	uint32_t get_batch_draw_count();
}