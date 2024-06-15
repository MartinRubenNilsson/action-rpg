#include "stdafx.h"
#include "sprites.h"

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
		if (left.texture != right.texture)
			return left.texture < right.texture;
		if (left.shader != right.shader)
			return left.shader < right.shader;
		return false;
	}

	constexpr uint32_t _calc_vertices_in_batch(uint32_t sprite_count) {
		return 6 * sprite_count - 2;
	}

	constexpr uint32_t _calc_sprites_in_batch(uint32_t vertex_count) {
		return (vertex_count + 2) / 6;
	}

	const uint32_t MAX_SPRITES = 1024;
	const uint32_t MAX_SPRITES_PER_BATCH = 512;
	const uint32_t MAX_VERTICES_PER_BATCH = _calc_vertices_in_batch(MAX_SPRITES_PER_BATCH);

	bool enable_batching = true;

	Sprite _sprite_buffer[MAX_SPRITES];
	uint32_t _sprites_by_draw_order[MAX_SPRITES]; // indices into _sprite_buffer
	uint32_t _sprites = 0;

	sf::Vertex _batch_vertex_buffer[MAX_VERTICES_PER_BATCH];
	uint32_t _batch_vertices = 0;

	uint32_t _sprites_drawn = 0;
	uint32_t _batches_drawn = 0;
	uint32_t _vertices_in_largest_batch = 0;

	void _render_batch(sf::RenderTarget& target, const sf::RenderStates& states)
	{
		//sf::Texture::bind(states.texture);
		//sf::Shader::bind(states.shader);
		target.draw(_batch_vertex_buffer, _batch_vertices, sf::TriangleStrip, states);
		//sf::Shader::bind(nullptr);
		//sf::Texture::bind(nullptr);
		_batches_drawn++;
		_vertices_in_largest_batch = std::max(_vertices_in_largest_batch, (uint32_t)_batch_vertices);
		_batch_vertices = 0;
	}

	void draw(const Sprite& sprite)
	{
		if (_sprites == MAX_SPRITES) return;
		memcpy(&_sprite_buffer[_sprites], &sprite, sizeof(Sprite));
		_sprites_by_draw_order[_sprites] = _sprites;
		_sprites++;
	}

	void render(sf::RenderTarget& target)
	{
		_sprites_drawn = _sprites;
		_batches_drawn = 0;
		_vertices_in_largest_batch = 0;

		// Sort by draw order. As an optimization, we sort indices instead of the sprites themselves.
		std::sort(_sprites_by_draw_order, _sprites_by_draw_order + _sprites, [](uint32_t left, uint32_t right) {
			return _sprite_buffer[left] < _sprite_buffer[right];
		});

		// Sprites sharing the same state (texture and shader) are batched together to reduce draw calls.
		// This is done by creating a triangle strip for each batch and drawing it only when the state changes.
		// Each sprite is represented by 4 vertices in the triangle strip, but we also need to add duplicate
		// vertices to create degenerate triangles that separate the sprites in the strip: If ABCD and EFGH
		// are the triangle strips for two sprites, then the batched triangle strip will be ABCDDEEFGH.

		sf::RenderStates states{};
		for (uint32_t i = 0; i < _sprites; ++i) {
			Sprite& sprite = _sprite_buffer[_sprites_by_draw_order[i]];

			sf::Vector2f tl = sprite.min; // top-left corner
			sf::Vector2f bl = { sprite.min.x, sprite.max.y }; // bottom-left corner
			sf::Vector2f tr = { sprite.max.x, sprite.min.y }; // top-right corner
			sf::Vector2f br = sprite.max; // bottom-right corner

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
					sprite.texture == states.texture &&
					sprite.shader == states.shader &&
					!sprite.pre_render_callback)
				{
					// Add degenerate triangles to separate the sprites
					uint32_t previous_vertex_index = _batch_vertices - 1; // so we don't get undefined behavior in the next line
					_batch_vertex_buffer[_batch_vertices++] = _batch_vertex_buffer[previous_vertex_index]; // D
					_batch_vertex_buffer[_batch_vertices++] = { tl, sprite.color, sprite.tex_min }; // E
				} else {
					// Draw the current batch and start a new one
					_render_batch(target, states);
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
			states.texture = sprite.texture;
			states.shader = sprite.shader;
		}

		// Draw the last batch if there is one
		if (_batch_vertices > 0) {
			_render_batch(target, states);
		}

		_sprites = 0;
	}

	uint32_t get_sprites_drawn() {
		return _sprites_drawn;
	}

	uint32_t get_batches_drawn() {
		return _batches_drawn;
	}

	uint32_t get_vertices_in_largest_batch() {
		return _vertices_in_largest_batch;
	}

	uint32_t get_sprites_in_largest_batch() {
		return _calc_sprites_in_batch(_vertices_in_largest_batch);
	}
}
