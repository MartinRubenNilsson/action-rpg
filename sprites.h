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
		void (*pre_render_callback)(const Sprite&) = nullptr; // optional; called just before the sprite is rendered
		void* userdata = nullptr; // optional; can be used to store additional data
		sf::Texture* texture = nullptr; // required
		int shader_id = 0; // optional; if set, the shader with this ID is used to render the sprite
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
	// If batching is enabled, then sprites that compare equal are merged into the same batch,
	// unless the pre_render_callback member is set, in which case the sprite is not batched.
	//
	bool operator<(const Sprite& left, const Sprite& right);

	extern const uint32_t MAX_SPRITES;
	extern const uint32_t MAX_SPRITES_PER_BATCH;
	extern bool enable_batching;

	// Submits a sprite for rendering. The sprite is not rendered immediately, but
	// instead copied to a buffer of sprites to be rendered when render() is called.
	void draw(const Sprite& sprite);
	void render(sf::RenderTarget& target);

	// STATISTICS FOR LAST RENDER CALL

	uint32_t get_sprites_drawn(); 
	uint32_t get_batches_drawn();
	uint32_t get_vertices_in_largest_batch();
	uint32_t get_sprites_in_largest_batch();
}