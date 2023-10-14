#pragma once

namespace ecs
{
	// These functions are registered with the console in console_commands.cpp.

	bool activate_camera(const std::string& entity_name);
	bool add_camera_trauma(const std::string& entity_name, float trauma);
}
