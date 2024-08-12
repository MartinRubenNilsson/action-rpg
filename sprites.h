#pragma once

namespace graphics
{
	extern Handle<Shader> sprite_shader;
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

	struct
		//alignas(std::hardware_constructive_interference_size)
		Sprite
	{
		Handle<graphics::Shader> shader = graphics::sprite_shader;
		Handle<graphics::Texture> texture; // TODO: default to error texture
		//TODO: store handle to uniform buffer
		//TODO: store uint16_t offset and size into uniform buffer
		Vector2f pos; // top-left corner position in world space
		Vector2f size; // width and height in world space
		Vector2f tex_pos = { 0.f, 0.f }; // top-left corner in normalized texture coordinates
		Vector2f tex_size = { 1.f, 1.f }; // width and height in normalized texture coordinates
		Vector2f sorting_pos;
		Color color = colors::WHITE;
		uint8_t sorting_layer = 0;
		uint8_t flags = SPRITE_VISIBLE;
	};

	//static_assert(sizeof(Sprite) <= std::hardware_destructive_interference_size, "Sprite doen't fit in a cache line!");

	// Orders sprites by draw order. Members are compared in a cascading fashion:
	// 
	// 1. sorting_layer
	// 2. sorting_pos.y
	// 3. sorting_pos.x
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
	void draw(std::string_view debug_group_name); // Draws all added sprites.

	// DRAWING STATISTICS

	void clear_drawing_statistics();
	unsigned int get_sprites_drawn(); 
	unsigned int get_batches_drawn();
	unsigned int get_largest_batch_sprite_count();
	unsigned int get_largest_batch_vertex_count();
}