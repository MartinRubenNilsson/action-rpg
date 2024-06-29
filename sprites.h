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
		int texture_id = -1;
		int shader_id = 0; // optional; if set, the shader with this ID is used to render the sprite
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
	// 4. texture_id
	// 5. shader_id
	// 
	// In all cases, a less-than comparison is used.
	// If one case yields a tie, then the next case is considered, and so on.
	// 
	// If batching is enabled, then sprites that compare equal are merged into the same batch,
	// unless the pre_render_callback member is set, in which case the sprite is not batched.
	//
	bool operator<(const Sprite& left, const Sprite& right);

	extern const unsigned int MAX_SPRITES;
	extern const unsigned int MAX_SPRITES_PER_BATCH;
	extern bool enable_batching;

	void reset_rendering_statistics();
	void add_sprite_to_render_queue(const Sprite& sprite);
	void render(); // renders all sprites in the render queue and clears the queue

	// RENDERING STATISTICS

	unsigned int get_sprites_drawn(); 
	unsigned int get_batches_drawn();
	unsigned int get_vertices_in_largest_batch();
	unsigned int get_sprites_in_largest_batch();
}