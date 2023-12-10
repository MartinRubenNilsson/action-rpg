#pragma once

namespace ecs
{
	// These functions are registered with the console in console_commands.cpp.

	bool kill_player();
	bool hurt_player(int health_to_remove);
	bool activate_camera(const std::string& entity_name);
	bool add_camera_trauma(const std::string& entity_name, float trauma);
}
