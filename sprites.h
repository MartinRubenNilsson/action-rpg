#pragma once

namespace graphics
{
	extern Handle<Shader> sprite_shader;
	extern Handle<Texture> error_texture;
}

namespace sprites
{
	enum SPRITE_FLAGS : uint8_t
	{
		SPRITE_VISIBLE		     = (1 << 0), // for use in game logic
		SPRITE_FLIP_HORIZONTALLY = (1 << 1),
		SPRITE_FLIP_VERTICALLY   = (1 << 2),
		SPRITE_FLIP_DIAGONALLY   = (1 << 3),
		SPRITE_ROTATE_90         = SPRITE_FLIP_HORIZONTALLY | SPRITE_FLIP_DIAGONALLY,
		SPRITE_ROTATE_180        = SPRITE_FLIP_HORIZONTALLY | SPRITE_FLIP_VERTICALLY,
		SPRITE_ROTATE_270        = SPRITE_FLIP_VERTICALLY | SPRITE_FLIP_DIAGONALLY,
	};

	constexpr size_t L1_CACHE_LINE_SIZE = std::hardware_constructive_interference_size;

	struct alignas(L1_CACHE_LINE_SIZE) Sprite
	{
		Handle<graphics::Shader> shader = graphics::sprite_shader;
		Handle<graphics::Texture> texture = graphics::error_texture;
		Handle<graphics::Buffer> uniform_buffer;
		uint16_t uniform_buffer_size = 0; // size in bytes of the uniform buffer
		uint16_t uniform_buffer_offset = 0; // offset in bytes into the uniform buffer; must be a multiple of 256
		uint16_t _unused = 0;
		uint8_t flags = SPRITE_VISIBLE;
		uint8_t sorting_layer = 0;
		Vector2f sorting_point;
		Vector2f position; // top-left corner position in world space
		Vector2f size; // width and height in world space
		Vector2f tex_position = { 0.f, 0.f }; // top-left corner in normalized texture coordinates
		Vector2f tex_size = { 1.f, 1.f }; // width and height in normalized texture coordinates
		Color color = colors::WHITE;
	};

	static_assert(sizeof(Sprite) == L1_CACHE_LINE_SIZE, "Sprite doesn't fit exactly in a cache line!");

	// Orders sprites by draw order. Members are compared in a cascading fashion:
	// 
	// 1. sorting_layer
	// 2. position.y + sorting_point.y
	// 3. position.x + sorting_point.x
	// 4. shader
	// 5. texture
	// 
	// In all cases, a less-than comparison is used.
	// If one case yields a tie, then the next case is considered, and so on.
	//
	bool operator<(const Sprite& left, const Sprite& right);

	// DRAWING

	void add(const Sprite& sprite); // Adds a sprite to be sorted and drawn later.
	void sort(); // Sorts all added sprites by draw order.
	void draw(); // Draws all added sprites.

	// DRAWING STATISTICS

	void clear_drawing_statistics();
	unsigned int get_sprites_drawn(); 
	unsigned int get_batches_drawn();
	unsigned int get_largest_batch_sprite_count();
	unsigned int get_largest_batch_vertex_count();
}