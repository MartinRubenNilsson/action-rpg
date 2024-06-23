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
#include "ecs_portal.h"
#include "console.h"
#include "sprites.h"
#include "debug_draw.h"
#include "graphics.h"

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
		update_portals(dt);
		update_pickups(dt);
		update_bombs(dt);	
		update_ai_logic(dt);
		update_ai_graphics(dt);
		update_common(dt);
		update_tiles(dt);
		update_vfx(dt);
		update_cameras(dt);
	}

	void get_camera_bounds(sf::Vector2f& min, sf::Vector2f& max)
	{
		const CameraView view = ecs::get_blended_camera_view();
		min = view.center - view.size / 2.f;
		max = view.center + view.size / 2.f;
	}
	 
	void add_sprites_to_render_queue(const sf::Vector2f& camera_min, const sf::Vector2f& camera_max)
	{
		add_tile_sprites_to_render_queue(camera_min, camera_max);
		add_vfx_sprites_to_render_queue(camera_min, camera_max);
		sprites::render_sprites_in_render_queue();
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
