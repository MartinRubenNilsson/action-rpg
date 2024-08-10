#pragma once

namespace graphics
{
	extern Handle<Shader> sprite_shader;
}

namespace sprites
{
	enum SPRITE_FLAGS : uint8_t
	{
		SPRITE_FLIP_HORIZONTAL = (1 << 0),
		SPRITE_FLIP_VERTICAL   = (1 << 1),
		SPRITE_FLIP_DIAGONAL   = (1 << 2),
		SPRITE_ROTATE_90       = SPRITE_FLIP_HORIZONTAL | SPRITE_FLIP_DIAGONAL,
		SPRITE_ROTATE_180      = SPRITE_FLIP_HORIZONTAL | SPRITE_FLIP_VERTICAL,
		SPRITE_ROTATE_270      = SPRITE_FLIP_VERTICAL | SPRITE_FLIP_DIAGONAL,
	};

	struct Sprite
	{
		Handle<graphics::Shader> shader = graphics::sprite_shader;
		Handle<graphics::Texture> texture; // TODO: default to error texture
		Vector2f min; // top-left corner world position in pixels
		Vector2f max; // bottom-right corner world position in pixels
		Vector2f tex_min = { 0.f, 0.f }; // top-left corner normalized texture coordinates
		Vector2f tex_max = { 1.f, 1.f }; // bottom-right corner normalized texture coordinates
		Vector2f sorting_pos;
		Color color = colors::WHITE;
		uint8_t sorting_layer = 0;
		uint8_t flags = 0;
	};

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

	void add(const Sprite& sprite); // Adds a sprite to be drawn later.
	void draw(std::string_view debug_group_name); // Draws all added sprites in draw order.

	// DRAWING STATISTICS

	void clear_drawing_statistics();
	unsigned int get_sprites_drawn(); 
	unsigned int get_batches_drawn();
	unsigned int get_largest_batch_sprite_count();
	unsigned int get_largest_batch_vertex_count();
}