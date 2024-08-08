#include "stdafx.h"
#include "sprites.h"
#include "graphics.h"
#include "graphics_globals.h"

namespace sprites
{
	struct Batch
	{
		Handle<graphics::Shader> shader;
		Handle<graphics::Texture> texture;
		unsigned int vertex_count = 0;
		unsigned int vertex_offset = 0; // offset into the vertex buffer
	};

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

	std::vector<Sprite> _sprites;
	std::vector<unsigned int> _sprite_indices; // will be sorted by draw order
	std::vector<graphics::Vertex> _vertices;
	std::vector<Batch> _batches;

	unsigned int _sprites_drawn = 0;
	unsigned int _batches_drawn = 0;
	unsigned int _vertices_in_largest_batch = 0;

	void reset_rendering_statistics()
	{
		_sprites_drawn = 0;
		_batches_drawn = 0;
		_vertices_in_largest_batch = 0;
	}

	void add_sprite_to_render_queue(const Sprite& sprite)
	{
		_sprite_indices.push_back((unsigned int)_sprites.size());
		_sprites.push_back(sprite);
	}

	void render(std::string_view debug_group_name)
	{
		graphics::ScopedDebugGroup debug_group(debug_group_name);

		if (_sprites.empty()) return;

		// Sort by draw order. As an optimization, we sort indices instead of the sprites themselves.
		std::sort(_sprite_indices.begin(), _sprite_indices.end(), [](unsigned int left, unsigned int right) {
			return _sprites[left] < _sprites[right];
		});

		// Sprites sharing the same state (texture and shader) are batched together to reduce draw calls.
		// This is done by creating a triangle strip for each batch and drawing it only when the state changes.
		// Each sprite is represented by 4 vertices in the triangle strip, but we also need to add duplicate
		// vertices to create degenerate triangles that separate the sprites in the strip: If ABCD and EFGH
		// are the triangle strips for two sprites, then the batched triangle strip will be ABCDDEEFGH.

		for (unsigned int sprite_index : _sprite_indices) {
			const Sprite& sprite = _sprites[sprite_index];

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

			if (_batches.empty()) {
				Batch& first_batch = _batches.emplace_back();
				first_batch.shader = sprite.shader;
				first_batch.texture = sprite.texture;
			} else {
				Batch& current_batch = _batches.back();
				if (sprite.shader == current_batch.shader && sprite.texture == current_batch.texture) {
					// Add degenerate triangles to separate the sprites
					_vertices.emplace_back(_vertices.back()); // D
					_vertices.emplace_back(tl, sprite.color, sprite.tex_min); // E
					current_batch.vertex_count += 2;
				} else {
					Batch& new_batch = _batches.emplace_back();
					new_batch.shader = sprite.shader;
					new_batch.texture = sprite.texture;
					new_batch.vertex_offset = (unsigned int)_vertices.size();
				}
			}

			// Add the vertices of the new sprite to the batch
			_vertices.emplace_back(tl, sprite.color, sprite.tex_min);
			_vertices.emplace_back(bl, sprite.color, Vector2f(sprite.tex_min.x, sprite.tex_max.y));
			_vertices.emplace_back(tr, sprite.color, Vector2f(sprite.tex_max.x, sprite.tex_min.y));
			_vertices.emplace_back(br, sprite.color, sprite.tex_max);
			_batches.back().vertex_count += 4;
		}

		const unsigned int vertices_byte_size = (unsigned int)_vertices.size() * sizeof(graphics::Vertex);
		if (vertices_byte_size <= graphics::get_buffer_byte_size(graphics::vertex_buffer)) {
			graphics::update_buffer(graphics::vertex_buffer, _vertices.data(), vertices_byte_size);
		} else {
			//TODO: make a resize buffer function
			graphics::destroy_buffer(graphics::vertex_buffer);
			graphics::vertex_buffer = graphics::create_buffer({
				.debug_name = "vertex buffer",
				.type = graphics::BufferType::Vertex,
				.usage = graphics::Usage::DynamicDraw,
				.byte_size = vertices_byte_size,
				.initial_data = _vertices.data()
			});
		}

		for (const Batch& batch : _batches) {
			graphics::bind_shader(batch.shader);
			graphics::bind_texture(0, batch.texture);
			//TODO: bind uniform buffer
			graphics::draw(graphics::Primitives::TriangleStrip, batch.vertex_count, batch.vertex_offset);
			_vertices_in_largest_batch = std::max(_vertices_in_largest_batch, batch.vertex_count);
		}

		_sprites_drawn += (unsigned int)_sprites.size();
		_batches_drawn += (unsigned int)_batches.size();

		_sprites.clear();
		_sprite_indices.clear();
		_vertices.clear();
		_batches.clear();
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
