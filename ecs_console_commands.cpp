#include "stdafx.h"
#include "ecs_console_commands.h"
#include "ecs_common.h"
#include "ecs_camera.h"

namespace ecs
{
	extern entt::registry _registry;

	bool set_camera_priority(const std::string& entity_name, float priority)
	{
		for (auto [entity, name, camera] : _registry.view<Name, Camera>().each())
		{
			if (name.value == entity_name)
			{
				camera.priority = priority;
				return true;
			}
		}
		return false;
	}
}