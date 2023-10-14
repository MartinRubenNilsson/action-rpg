#include "stdafx.h"
#include "ecs_console_commands.h"
#include "ecs_common.h"
#include "ecs_camera.h"

namespace ecs
{
	extern entt::registry _registry;

	bool activate_camera(const std::string& entity_name)
	{
		for (auto [entity, name, camera] : _registry.view<Name, Camera>().each())
		{
			if (name.value == entity_name)
				return activate_camera(entity);
		}
		return false;
	}

	bool add_camera_trauma(const std::string& entity_name, float trauma)
	{
		for (auto [entity, name, camera] : _registry.view<Name, Camera>().each())
		{
			if (name.value == entity_name)
			{
				camera.trauma += trauma;
				return true;
			}
		}
		return false;
	}
}