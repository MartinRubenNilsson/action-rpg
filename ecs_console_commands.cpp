#include "stdafx.h"
#include "ecs_tiled.h"
#include "ecs_console_commands.h"
#include "tiled.h"
#include "ecs_player.h"
#include "ecs_camera.h"

namespace ecs
{
	extern entt::registry _registry;

	bool kill_player() {
		//return kill_player(get_player_entity());
		return false;
	}

	bool hurt_player(int health_to_remove) {
		//return hurt_player(get_player_entity(), health_to_remove);
		return false;
	}

	bool activate_camera(const std::string& entity_name) {
		return activate_camera(find_entity_by_name(entity_name));
	}

	bool add_camera_trauma(const std::string& entity_name, float trauma) {
		return add_camera_trauma(find_entity_by_name(entity_name), trauma);
	}
}