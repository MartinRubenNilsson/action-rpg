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

	std::vector<Sprite> _sprites;
	std::vector<uint32_t> _sprites_by_draw_order; // indices into _sprites_to_draw
	std::vector<sf::Vertex> _vertices;

	void draw(const Sprite& sprite)
	{
		_sprites.push_back(sprite);
		_sprites_by_draw_order.push_back((uint32_t)_sprites.size() - 1);
	}

	void render(sf::RenderTarget& target)
	{
		// Sort by draw order. As an optimization, we sort indices instead of the sprites themselves.
		std::sort(_sprites_by_draw_order.begin(), _sprites_by_draw_order.end(), [](uint32_t left, uint32_t right) {
			return _sprites[left] < _sprites[right];
		});

		// Sprites sharing the same state (texture and shader) are batched together to reduce draw calls.
		// This is done by creating a triangle strip for each batch and drawing it only when the state changes.
		// Each sprite is represented by 4 vertices in the triangle strip, but we also need to add duplicate
		// vertices to create degenerate triangles that separate the sprites in the strip: If ABCD and EFGH
		// are the triangle strips for two sprites, then the batched triangle strip will be ABCDDEEFGH.

		sf::RenderStates states{};
		for (uint32_t sprite_index : _sprites_by_draw_order) {
			const Sprite& sprite = _sprites[sprite_index];

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
			if (!_vertices.empty()) {
				// Can we add the new sprite to the batch?
				// HACK: to render grass with different shader uniforms, break the batch for every custom shader
				if (!sprite.shader && sprite.texture.get() == states.texture) {
					// Add degenerate triangles to separate the sprites
					_vertices.push_back(_vertices.back()); // D
					_vertices.emplace_back(tl, sprite.color, sprite.tex_min); // E
				} else {
					// Draw the current batch and start a new one
					target.draw(_vertices.data(), _vertices.size(), sf::TriangleStrip, states);
					_vertices.clear();
				}
			}

			// Add the vertices of the new sprite to the batch
			_vertices.emplace_back(tl, sprite.color, sprite.tex_min);
			_vertices.emplace_back(bl, sprite.color, sf::Vector2f(sprite.tex_min.x, sprite.tex_max.y));
			_vertices.emplace_back(tr, sprite.color, sf::Vector2f(sprite.tex_max.x, sprite.tex_min.y));
			_vertices.emplace_back(br, sprite.color, sprite.tex_max);

			// Update shader uniforms
			if (sprite.shader.get()) {
				//sprite.shader->setUniform("time", _time);
				sprite.shader->setUniform("position", sprite.min);
			}

			// Update the render states
			states.texture = sprite.texture.get();
			states.shader = sprite.shader.get();
		}

		// Draw the last batch if there is one
		if (!_vertices.empty()) { 
			target.draw(_vertices.data(), _vertices.size(), sf::TriangleStrip, states);
			_vertices.clear();
		}

		_sprites.clear();
		_sprites_by_draw_order.clear();
	}
}
