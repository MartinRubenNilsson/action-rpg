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
		return false;
	}

	int debug_flags = DEBUG_NONE;

	entt::registry _registry;
	std::vector<Sprite> _sprites;
	std::vector<uint32_t> _sprites_draw_order; // indices into _sprites
	uint32_t _sprite_count = 0;

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
			Sprite sprite{};
			sprite.min = tile.position - tile.pivot;
			if (sprite.min.x > view_max.x || sprite.min.y > view_max.y) continue;
			sf::IntRect texture_rect = tile.get_texture_rect();
			sprite.tex_min = { (float)texture_rect.left, (float)texture_rect.top };
			sprite.tex_max = { (float)texture_rect.left + texture_rect.width, (float)texture_rect.top + texture_rect.height };
			sprite.max = sprite.min + sprite.tex_max - sprite.tex_min;
			if (sprite.max.x < view_min.x || sprite.max.y < view_min.y) continue;
			if (tile.get_flag(TF_FLIP_X)) std::swap(sprite.tex_min.x, sprite.tex_max.x);
			if (tile.get_flag(TF_FLIP_Y)) std::swap(sprite.tex_min.y, sprite.tex_max.y);
			sprite.sorting_layer = tile.sorting_layer;
			sprite.sorting_position = sprite.min + tile.sorting_pivot;
			sprite.texture = tile.get_texture();
			sprite.shader = tile.shader;
			sprite.color = tile.color;
			_sprites.push_back(sprite);
			_sprites_draw_order.push_back(_sprite_count++);
		}
		std::sort(_sprites_draw_order.begin(), _sprites_draw_order.end(), [](uint32_t left, uint32_t right) {
			return _sprites[left] < _sprites[right];
		});
		sf::Vertex vertices[4];
		sf::RenderStates states{};
		for (uint32_t sprite_index : _sprites_draw_order) {
			const Sprite& sprite = _sprites[sprite_index];
			vertices[0].position = sprite.min;
			vertices[1].position = { sprite.min.x, sprite.max.y };
			vertices[2].position = { sprite.max.x, sprite.min.y };
			vertices[3].position = sprite.max;
			vertices[0].texCoords = sprite.tex_min;
			vertices[1].texCoords = { sprite.tex_min.x, sprite.tex_max.y };
			vertices[2].texCoords = { sprite.tex_max.x, sprite.tex_min.y };
			vertices[3].texCoords = sprite.tex_max;
			for (size_t i = 0; i < 4; ++i)
				vertices[i].color = sprite.color;
			states.texture = sprite.texture.get();
			states.shader = sprite.shader.get();
			target.draw(vertices, 4, sf::TriangleStrip, states);
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
		_sprite_count = 0;
	}
}
