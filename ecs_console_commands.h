#pragma once

namespace ecs
{
	// These functions are registered with the console in console_commands.cpp.

	bool set_camera_priority(const std::string& entity_name, float priority);
}
