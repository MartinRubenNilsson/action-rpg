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

	const uint32_t MAX_SPRITES_PER_BATCH = 512;
	const uint32_t MAX_VERTICES_PER_BATCH = _calc_vertices_in_batch(MAX_SPRITES_PER_BATCH);
	bool enable_batching = true;
	std::vector<Sprite> _sprites_to_draw;
	std::vector<uint32_t> _sprites_by_draw_order; // indices into _sprites_to_draw
	sf::Vertex _batch_vertex_buffer[MAX_VERTICES_PER_BATCH];
	uint32_t _batch_vertex_count = 0;
	uint32_t _sprites_drawn = 0;
	uint32_t _batches_drawn = 0;
	uint32_t _largest_batch_vertex_count = 0;

	float _time = 0.f; //HACK

	void _render_batch(sf::RenderTarget& target, const sf::RenderStates& states)
	{
		_batches_drawn++;
		_largest_batch_vertex_count = std::max(_largest_batch_vertex_count, (uint32_t)_batch_vertex_count);
		target.draw(_batch_vertex_buffer, _batch_vertex_count, sf::TriangleStrip, states);
		_batch_vertex_count = 0;
	}

	void set_time(float time) {
		_time = time;
	}

	void draw(const Sprite& sprite)
	{
		_sprites_to_draw.push_back(sprite);
		_sprites_by_draw_order.push_back((uint32_t)_sprites_to_draw.size() - 1);
	}

	void render(sf::RenderTarget& target)
	{
		_sprites_drawn = (uint32_t)_sprites_to_draw.size();
		_batches_drawn = 0;
		_largest_batch_vertex_count = 0;

		// Sort by draw order. As an optimization, we sort indices instead of the sprites themselves.
		std::sort(_sprites_by_draw_order.begin(), _sprites_by_draw_order.end(), [](uint32_t left, uint32_t right) {
			return _sprites_to_draw[left] < _sprites_to_draw[right];
		});

		// Sprites sharing the same state (texture and shader) are batched together to reduce draw calls.
		// This is done by creating a triangle strip for each batch and drawing it only when the state changes.
		// Each sprite is represented by 4 vertices in the triangle strip, but we also need to add duplicate
		// vertices to create degenerate triangles that separate the sprites in the strip: If ABCD and EFGH
		// are the triangle strips for two sprites, then the batched triangle strip will be ABCDDEEFGH.

		sf::RenderStates states{};
		for (uint32_t sprite_index : _sprites_by_draw_order) {
			const Sprite& sprite = _sprites_to_draw[sprite_index];

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
			if (_batch_vertex_count > 0) {
				// Can we add the new sprite to the batch?
				// HACK: to render grass with different shader uniforms, break the batch for every custom shader
				if (enable_batching && _batch_vertex_count != MAX_VERTICES_PER_BATCH && !sprite.shader && sprite.texture == states.texture) {
					// Add degenerate triangles to separate the sprites
					uint32_t previous_vertex_index = _batch_vertex_count - 1; // so we don't get undefined behavior in the next line
					_batch_vertex_buffer[_batch_vertex_count++] = _batch_vertex_buffer[previous_vertex_index]; // D
					_batch_vertex_buffer[_batch_vertex_count++] = { tl, sprite.color, sprite.tex_min }; // E
				} else {
					// Draw the current batch and start a new one
					_render_batch(target, states);
				}
			}

			// Add the vertices of the new sprite to the batch
			_batch_vertex_buffer[_batch_vertex_count++] = { tl, sprite.color, sprite.tex_min };
			_batch_vertex_buffer[_batch_vertex_count++] = { bl, sprite.color, { sprite.tex_min.x, sprite.tex_max.y } };
			_batch_vertex_buffer[_batch_vertex_count++] = { tr, sprite.color, { sprite.tex_max.x, sprite.tex_min.y } };
			_batch_vertex_buffer[_batch_vertex_count++] = { br, sprite.color, sprite.tex_max };

			// Update shader uniforms
			if (sprite.shader) {
				sprite.shader->setUniform("time", _time);
				sprite.shader->setUniform("position", sprite.min);
			}

			// Update the render states
			states.texture = sprite.texture;
			states.shader = sprite.shader;
		}

		// Draw the last batch if there is one
		if (_batch_vertex_count > 0) {
			_render_batch(target, states);
		}

		_sprites_to_draw.clear();
		_sprites_by_draw_order.clear();
	}

	uint32_t get_sprites_drawn() {
		return _sprites_drawn;
	}

	uint32_t get_batches_drawn() {
		return _batches_drawn;
	}

	uint32_t get_largest_batch_vertex_count() {
		return _largest_batch_vertex_count;
	}

	uint32_t get_largest_batch_sprite_count() {
		return _calc_sprites_in_batch(_largest_batch_vertex_count);
	}
}
