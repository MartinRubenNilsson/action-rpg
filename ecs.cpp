#include "stdafx.h"
#include "ecs.h"
#include "console.h"
#include "ecs_common.h"
#include "ecs_physics.h"
#include "ecs_player.h"
#include "ecs_ai.h"
#include "ecs_tile.h"
#include "ecs_camera.h"
#include "ecs_vfx.h"
#include "ecs_pickups.h"
#include "ecs_bomb.h"

namespace ecs
{
	//This should be put in its own file
	struct Sprite
	{
		sf::Vector2f min; // top-left corner
		sf::Vector2f max; // bottom-right corner
		sf::Vector2f tex_min; // texture coordinates in pixels
		sf::Vector2f tex_max; // texture coordinates in pixels
		sf::Vector2f sorting_position;
		SortingLayer sorting_layer;
		std::shared_ptr<sf::Texture> texture;
		std::shared_ptr<sf::Shader> shader;
		sf::Color color = sf::Color::White;
	};

	bool operator<(const Sprite& left, const Sprite& right)
	{
		if (left.sorting_layer != right.sorting_layer)
			return left.sorting_layer < right.sorting_layer;
		if (left.sorting_position.y != right.sorting_position.y)
			return left.sorting_position.y < right.sorting_position.y;
		if (left.sorting_position.x != right.sorting_position.x)
			return left.sorting_position.x < right.sorting_position.x;
		if (left.texture != right.texture)
			return left.texture < right.texture;
		if (left.shader != right.shader)
			return left.shader < right.shader;
		return false;
	}

	int debug_flags = DEBUG_NONE;
	entt::registry _registry;
	float _time = 0.f;
	std::vector<Sprite> _sprites;
	std::vector<uint32_t> _sprites_draw_order; // indices into _sprites
	std::vector<sf::Vertex> _vertices;

	void initialize() {
		initialize_physics();
	}

	void shutdown()
	{
		clear();
		shutdown_physics();
	}

	void process_event(const sf::Event& event) {
		process_event_players(event);
	}

	void update(float dt)
	{
		_time += dt;
		update_players(dt);
		update_pickups(dt);
		update_bombs(dt);	
		update_ai_logic(dt);
		update_ai_graphics(dt);
		update_physics(dt);
		update_common(dt);
		update_tiles(dt);
		update_vfx(dt);
		update_cameras(dt);
	}
	 
	void render(sf::RenderTarget& target)
	{
		const sf::View view = ecs::get_blended_camera_view();
		const sf::Vector2f view_min = view.getCenter() - view.getSize() / 2.f; // assumes no rotation
		const sf::Vector2f view_max = view.getCenter() + view.getSize() / 2.f; // assumes no rotation
		target.setView(view);

		for (auto [entity, tile] : _registry.view<Tile>().each()) {
			if (!tile.is_valid()) continue;
			if (!tile.get_flag(TF_VISIBLE)) continue;
			sf::Vector2f min = tile.position - tile.pivot;
			if (min.x > view_max.x || min.y > view_max.y) continue;
			sf::IntRect texture_rect = tile.get_texture_rect();
			sf::Vector2f tex_min = { (float)texture_rect.left, (float)texture_rect.top };
			sf::Vector2f tex_max = { (float)texture_rect.left + texture_rect.width, (float)texture_rect.top + texture_rect.height };
			sf::Vector2f max = min + tex_max - tex_min;
			if (max.x < view_min.x || max.y < view_min.y) continue;
			if (tile.get_flag(TF_FLIP_X)) std::swap(tex_min.x, tex_max.x);
			if (tile.get_flag(TF_FLIP_Y)) std::swap(tex_min.y, tex_max.y);
			_sprites_draw_order.push_back((uint32_t)_sprites.size());
			_sprites.emplace_back(
				min, max, tex_min, tex_max, min + tile.sorting_pivot, tile.sorting_layer,
				tile.get_texture(), tile.shader, tile.color);
		}
		std::sort(_sprites_draw_order.begin(), _sprites_draw_order.end(), [](uint32_t left, uint32_t right) {
			return _sprites[left] < _sprites[right];
		});
		sf::RenderStates states{};
		for (uint32_t sprite_index : _sprites_draw_order) {

			// Sprites sharing the same state (texture and shader) are batched together to reduce draw calls.
			// This is done by creating a triangle strip for each batch and drawing it only when the state changes.
			// Each sprite is represented by 4 vertices in the triangle strip, but we also need to add duplicate
			// vertices to create the degenerate triangles that separate the sprites in the strip: If ABCD and EFGH
			// are the triangle strips of two sprites, the batched triangle strip will be ABCDDEEFGH.

			const Sprite& sprite = _sprites[sprite_index];
			// Are we in the middle of a batch?
			if (!_vertices.empty()) { 
				// Can we add the new sprite to the batch?
				// HACK: to render grass with different shader uniforms, break the batch for every custom shader
				if (!sprite.shader && sprite.texture.get() == states.texture) {
					// Add degenerate triangles to separate the sprites
					_vertices.push_back(_vertices.back()); // D
					_vertices.emplace_back(sprite.min, sprite.color, sprite.tex_min); // E
				} else {
					// Draw the current batch and start a new one
					target.draw(_vertices.data(), _vertices.size(), sf::TriangleStrip, states);
					_vertices.clear();
				}
			}
			// Add the vertices of the new sprite to the batch
			_vertices.emplace_back(sprite.min, sprite.color, sprite.tex_min);
			_vertices.emplace_back(sf::Vector2f(sprite.min.x, sprite.max.y), sprite.color, sf::Vector2f(sprite.tex_min.x, sprite.tex_max.y));
			_vertices.emplace_back(sf::Vector2f(sprite.max.x, sprite.min.y), sprite.color, sf::Vector2f(sprite.tex_max.x, sprite.tex_min.y));
			_vertices.emplace_back(sprite.max, sprite.color, sprite.tex_max);
			// Update shader uniforms
			if (sprite.shader.get()) {
				sprite.shader->setUniform("time", _time);
				sprite.shader->setUniform("position", sprite.min);
			}
			// Update the render states
			states.texture = sprite.texture.get();
			states.shader = sprite.shader.get();
		}
		if (!_vertices.empty()) { // Draw the last batch if there is one
			target.draw(_vertices.data(), _vertices.size(), sf::TriangleStrip, states);
			_vertices.clear();
		}

		// DRAW VFX

		for (auto [entity, vfx] : _registry.view<Vfx>().each())
			target.draw(vfx.sprite); // TODO: culling

		// DEBUG DRAWING

		if (debug_flags & DEBUG_PIVOTS) {
			// Here the draw order is not important
			for (const Sprite& sprite : _sprites) {
				if (sprite.sorting_layer != SortingLayer::Objects) continue;
				sf::CircleShape circle(1.f);
				circle.setPosition(sprite.sorting_position);
				circle.setFillColor(sf::Color::Red);
				target.draw(circle);
			}
		}
		if (debug_flags & DEBUG_PHYSICS)
			debug_draw_physics();
		if (debug_flags & DEBUG_AI)
			debug_draw_ai();
		if (debug_flags & DEBUG_PLAYER)
			debug_draw_players();

		_sprites.clear();
		_sprites_draw_order.clear();
	}
}
