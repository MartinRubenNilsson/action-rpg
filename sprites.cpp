#include "stdafx.h"
#include "sprites.h"
#include "graphics.h"
#include "graphics_globals.h"

namespace sprites
{
	bool operator<(const Sprite& left, const Sprite& right)
	{
		if (left.sorting_layer != right.sorting_layer)
			return left.sorting_layer < right.sorting_layer;
		if (left.sorting_pos.y != right.sorting_pos.y)
			return left.sorting_pos.y < right.sorting_pos.y;
		if (left.sorting_pos.x != right.sorting_pos.x)
			return left.sorting_pos.x < right.sorting_pos.x;
		if (left.shader != right.shader)
			return left.shader < right.shader;
		if (left.texture != right.texture)
			return left.texture < right.texture;
		return false;
	}

	constexpr unsigned int _calc_vertices_in_batch(unsigned int sprite_count)
	{
		return 6 * sprite_count - 2;
	}

	constexpr unsigned int _calc_sprites_in_batch(unsigned int vertex_count)
	{
		return (vertex_count + 2) / 6;
	}

	const unsigned int MAX_SPRITES = 1024;
	const unsigned int MAX_SPRITES_PER_BATCH = 512;
	const unsigned int MAX_VERTICES_PER_BATCH = _calc_vertices_in_batch(MAX_SPRITES_PER_BATCH);

	bool enable_batching = true;

	Sprite _sprite_buffer[MAX_SPRITES];
	unsigned int _sprites_by_draw_order[MAX_SPRITES]; // indices into _sprite_buffer
	unsigned int _sprites = 0;

	graphics::Vertex _batch_vertex_buffer[MAX_VERTICES_PER_BATCH];
	unsigned int _batch_vertices = 0;

	unsigned int _sprites_drawn = 0;
	unsigned int _batches_drawn = 0;
	unsigned int _vertices_in_largest_batch = 0;

	void _render_batch(Handle<graphics::Shader> shader, Handle<graphics::Texture> texture)
	{
		graphics::bind_shader(shader);
		graphics::bind_texture(0, texture);
		graphics::update_buffer(graphics::vertex_buffer, _batch_vertex_buffer, _batch_vertices * sizeof(graphics::Vertex));
		graphics::draw(graphics::Primitives::TriangleStrip, _batch_vertices);
		_batches_drawn++;
		_vertices_in_largest_batch = std::max(_vertices_in_largest_batch, (unsigned int)_batch_vertices);
		_batch_vertices = 0;
	}

	void reset_rendering_statistics()
	{
		_sprites = 0;
		_sprites_drawn = 0;
		_batches_drawn = 0;
		_vertices_in_largest_batch = 0;
	}

	void add_sprite_to_render_queue(const Sprite& sprite)
	{
		if (_sprites == MAX_SPRITES) return;
		static_assert(std::is_trivially_copyable<Sprite>::value, "Sprite must be trivially copyable.");
		memcpy(&_sprite_buffer[_sprites], &sprite, sizeof(Sprite));
		_sprites_by_draw_order[_sprites] = _sprites;
		_sprites++;
	}

	void render(std::string_view debug_group_name)
	{
		graphics::ScopedDebugGroup debug_group(debug_group_name);

		// Sort by draw order. As an optimization, we sort indices instead of the sprites themselves.
		std::sort(_sprites_by_draw_order, _sprites_by_draw_order + _sprites, [](unsigned int left, unsigned int right) {
			return _sprite_buffer[left] < _sprite_buffer[right];
		});

		// Sprites sharing the same state (texture and shader) are batched together to reduce draw calls.
		// This is done by creating a triangle strip for each batch and drawing it only when the state changes.
		// Each sprite is represented by 4 vertices in the triangle strip, but we also need to add duplicate
		// vertices to create degenerate triangles that separate the sprites in the strip: If ABCD and EFGH
		// are the triangle strips for two sprites, then the batched triangle strip will be ABCDDEEFGH.

		Handle<graphics::Shader> last_shader;
		Handle<graphics::Texture> last_texture;

		for (unsigned int i = 0; i < _sprites; ++i) {
			Sprite& sprite = _sprite_buffer[_sprites_by_draw_order[i]];

			Vector2f tl = sprite.min; // top-left corner
			Vector2f bl = { sprite.min.x, sprite.max.y }; // bottom-left corner
			Vector2f tr = { sprite.max.x, sprite.min.y }; // top-right corner
			Vector2f br = sprite.max; // bottom-right corner

			if (sprite.flags & SF_FLIP_X) {
				std::swap(tl, tr);
				std::swap(bl, br);
			}
			if (sprite.flags & SF_FLIP_Y) {
				std::swap(tl, bl);
				std::swap(tr, br);
			}
			if (sprite.flags & SF_FLIP_DIAGONAL) {
				std::swap(bl, tr);
			}

			// Are we in the middle of a batch?
			if (_batch_vertices > 0) {
				// Can we add the new sprite to the batch?
				if (enable_batching &&
					_batch_vertices != MAX_VERTICES_PER_BATCH &&
					sprite.shader == last_shader &&
					sprite.texture == last_texture &&
					!sprite.pre_render_callback)
				{
					// Add degenerate triangles to separate the sprites
					unsigned int previous_vertex_index = _batch_vertices - 1; // so we don't get undefined behavior in the next line
					_batch_vertex_buffer[_batch_vertices++] = _batch_vertex_buffer[previous_vertex_index]; // D
					_batch_vertex_buffer[_batch_vertices++] = { tl, sprite.color, sprite.tex_min }; // E
				} else {
					// Draw the current batch and start a new one
					_render_batch(last_shader, last_texture);
				}
			}

			// Add the vertices of the new sprite to the batch
			_batch_vertex_buffer[_batch_vertices++] = { tl, sprite.color, sprite.tex_min };
			_batch_vertex_buffer[_batch_vertices++] = { bl, sprite.color, { sprite.tex_min.x, sprite.tex_max.y } };
			_batch_vertex_buffer[_batch_vertices++] = { tr, sprite.color, { sprite.tex_max.x, sprite.tex_min.y } };
			_batch_vertex_buffer[_batch_vertices++] = { br, sprite.color, sprite.tex_max };

			// Execute the pre-render callback
			if (sprite.pre_render_callback) {
				sprite.pre_render_callback(sprite);
			}

			// Update the render states
			last_shader = sprite.shader;
			last_texture = sprite.texture;
		}

		// Draw the last batch if there is one
		if (_batch_vertices > 0) {
			_render_batch(last_shader, last_texture);
		}

		_sprites_drawn += _sprites;
		_sprites = 0;
	}

	unsigned int get_sprites_drawn()
	{
		return _sprites_drawn;
	}

	unsigned int get_batches_drawn()
	{
		return _batches_drawn;
	}

	unsigned int get_vertices_in_largest_batch()
	{
		return _vertices_in_largest_batch;
	}

	unsigned int get_sprites_in_largest_batch()
	{
		return _calc_sprites_in_batch(_vertices_in_largest_batch);
	}
}
