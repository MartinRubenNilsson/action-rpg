#include "stdafx.h"
#include "ecs.h"
#include "ecs_common.h"
#include "ecs_physics.h"
#include "ecs_sprite.h"
#include "ecs_player.h"
#include "ecs_ai.h"
#include "ecs_tile.h"
#include "ecs_camera.h"
#include "ecs_vfx.h"
#include "ecs_pickups.h"
#include "ecs_bomb.h"
#include "ecs_portal.h"
#include "ecs_blade_trap.h"
#include "console.h"
#include "sprites.h"
#include "shapes.h"
#include "graphics.h"

namespace ecs
{
	int debug_flags = DEBUG_NONE;
	entt::registry _registry;

	void initialize()
	{
		initialize_physics();
	}

	void shutdown()
	{
		clear();
		shutdown_physics();
	}

	void process_window_event(const window::Event& event) 
	{
		process_window_event_for_players(event);
	}

	void update(float dt)
	{
		update_physics(dt);
		update_players(dt);
		update_portals(dt);
		update_pickups(dt);
		update_bombs(dt);
		update_blade_traps(dt);
		update_ai_logic(dt);
		update_ai_graphics(dt);
		destroy_entities_to_be_destroyed_at_end_of_frame();
		update_tile_positions(dt);
		update_tile_animations(dt);
		update_vfx(dt);
		update_cameras(dt);
	}

	void get_camera_bounds(Vector2f& min, Vector2f& max)
	{
		Vector2f center, size;
		ecs::get_blended_camera_view(center, size);
		min = center - size / 2.f;
		max = center + size / 2.f;
	}
	 
	void draw_sprites(const Vector2f& camera_min, const Vector2f& camera_max)
	{
		for (auto [entity, sprite] : _registry.view<const sprites::Sprite>().each()) {
			if (!(sprite.flags & sprites::SPRITE_VISIBLE)) continue;
			if (sprite.min.x > camera_max.x) continue;
			if (sprite.min.y > camera_max.y) continue;
			if (sprite.max.x < camera_min.x) continue;
			if (sprite.max.y < camera_min.y) continue;
			sprites::add(sprite);
		}
		add_tile_sprites_for_drawing(camera_min, camera_max);
		add_vfx_sprites_for_drawing(camera_min, camera_max);
		sprites::sort();
		sprites::draw("ECS");
	}

	void add_debug_shapes_to_render_queue()
	{
		if (debug_flags & DEBUG_PHYSICS) {
			debug_draw_physics();
		}
		if (debug_flags & DEBUG_AI) {
			debug_draw_ai();
		}
		if (debug_flags & DEBUG_PLAYER) {
			show_player_debug_window();
		}
	}
}
