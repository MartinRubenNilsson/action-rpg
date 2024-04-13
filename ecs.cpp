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
#include "debug_draw.h"

namespace ecs
{
	int debug_flags = DEBUG_NONE;
	entt::registry _registry;

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
		const CameraView view = ecs::get_blended_camera_view();
		const sf::Vector2f camera_min = view.center - view.size / 2.f; // assumes no rotation
		const sf::Vector2f camera_max = view.center + view.size / 2.f; // assumes no rotation
		target.setView(sf::View(view.center, view.size));
		draw_tiles(camera_min, camera_max);
		draw_vfx(camera_min, camera_max);
	}

	void _debug_draw_entities()
	{
		for (auto [entity, body] : _registry.view<b2Body*>().each()) {
			if (!_registry.any_of<Player, AiType>(entity)) continue;
			std::string entity_string = std::to_string((entt::id_type)entity);
			sf::Vector2f position = vector_cast<sf::Vector2f>(body->GetWorldCenter());
			position.y -= 16.f;
			debug::draw_text(entity_string, position);
		}
	}

	void debug_draw()
	{
		if (debug_flags & DEBUG_ENTITIES)
			_debug_draw_entities();
		if (debug_flags & DEBUG_PHYSICS)
			debug_draw_physics();
		if (debug_flags & DEBUG_AI)
			debug_draw_ai();
		if (debug_flags & DEBUG_PLAYER)
			debug_draw_players();
	}
}
