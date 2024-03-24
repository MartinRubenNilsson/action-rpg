#include "stdafx.h"
#include "ecs.h"
#include "ecs_common.h"
#include "ecs_physics.h"
#include "ecs_player.h"
#include "ecs_ai.h"
#include "ecs_tile.h"
#include "ecs_camera.h"
#include "ecs_vfx.h"
#include "ecs_pickups.h"
#include "ecs_bomb.h"
#include "console.h"
#include "sprites.h"

namespace ecs
{
	int debug_flags = DEBUG_NONE;
	entt::registry _registry;
	float _time = 0.f;

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

		sprites::Sprite sprite{}; // so we don't recreate it every iteration
		for (auto [entity, tile] : _registry.view<Tile>().each()) {
			if (!tile.is_valid()) continue;
			if (!tile.get_flag(TF_VISIBLE)) continue;
			sprite.texture = tile.get_texture();
			if (!sprite.texture) continue;
			sprite.shader = tile.shader;
			sprite.min = tile.position - tile.pivot;
			if (sprite.min.x > view_max.x || sprite.min.y > view_max.y) continue;
			sf::IntRect texture_rect = tile.get_texture_rect();
			sprite.tex_min = { (float)texture_rect.left, (float)texture_rect.top };
			sprite.tex_max = { (float)texture_rect.left + texture_rect.width, (float)texture_rect.top + texture_rect.height };
			sprite.max = sprite.min + sprite.tex_max - sprite.tex_min;
			if (sprite.max.x < view_min.x || sprite.max.y < view_min.y) continue;
			sprite.color = tile.color;
			sprite.sorting_layer = (uint8_t)tile.sorting_layer;
			sprite.sorting_pos = sprite.min + tile.sorting_pivot;
			sprite.flags = 0;
			if (tile.get_flag(TF_FLIP_X))
				sprite.flags |= sprites::SF_FLIP_X;
			if (tile.get_flag(TF_FLIP_Y))
				sprite.flags |= sprites::SF_FLIP_Y;
			if (tile.get_flag(TF_FLIP_DIAGONAL))
				sprite.flags |= sprites::SF_FLIP_DIAGONAL;
			sprites::submit(sprite);
		}
		sprites::set_time(_time);
		sprites::render(target);

		// DRAW VFX

		for (auto [entity, vfx] : _registry.view<Vfx>().each())
			target.draw(vfx.sprite); // TODO: culling

		// DEBUG DRAWING

		if (debug_flags & DEBUG_PIVOTS) {
			//// Here the draw order is not important
			//for (const sprites::Sprite& sprite : _sprites) {
			//	if (sprite.sorting_layer != (uint8_t)SortingLayer::Objects) continue;
			//	sf::CircleShape circle(1.f);
			//	circle.setPosition(sprite.sorting_pos);
			//	circle.setFillColor(sf::Color::Red);
			//	target.draw(circle);
			//}
		}
		if (debug_flags & DEBUG_PHYSICS)
			debug_draw_physics();
		if (debug_flags & DEBUG_AI)
			debug_draw_ai();
		if (debug_flags & DEBUG_PLAYER)
			debug_draw_players();
	}
}
