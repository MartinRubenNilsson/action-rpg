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
		update_physics(dt);
		update_players(dt);
		update_pickups(dt);
		update_bombs(dt);	
		update_ai_logic(dt);
		update_ai_graphics(dt);
		update_common(dt);
		update_tiles(dt);
		update_vfx(dt);
		update_cameras(dt);
	}
	 
	void draw(sf::RenderTarget& target)
	{
		const sf::View view = ecs::get_blended_camera_view();
		const sf::Vector2f camera_min = view.getCenter() - view.getSize() / 2.f; // assumes no rotation
		const sf::Vector2f camera_max = view.getCenter() + view.getSize() / 2.f; // assumes no rotation
		target.setView(view);
		sprites::set_time(_time);
		draw_tiles(camera_min, camera_max);
		draw_vfx(camera_min, camera_max);
	}

	void debug_draw()
	{
		if (debug_flags & DEBUG_TILES) {
			//TODO
		}
		if (debug_flags & DEBUG_PHYSICS)
			debug_draw_physics();
		if (debug_flags & DEBUG_AI)
			debug_draw_ai();
		if (debug_flags & DEBUG_PLAYER)
			debug_draw_players();
	}
}
