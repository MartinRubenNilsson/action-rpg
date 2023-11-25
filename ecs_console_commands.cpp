#include "stdafx.h"
#include "ecs_console_commands.h"
#include "tiled.h"
#include "ecs_camera.h"

namespace ecs
{
	extern entt::registry _registry;

	bool activate_camera(const std::string& entity_name)
	{
		for (auto [entity, object, camera] : _registry.view<tiled::Object, Camera>().each())
		{
			if (object.name == entity_name)
				return activate_camera(entity);
		}
		return false;
	}

	bool add_camera_trauma(const std::string& entity_name, float trauma)
	{
		for (auto [entity, object, camera] : _registry.view<tiled::Object, Camera>().each())
		{
			if (object.name == entity_name)
			{
				camera.trauma += trauma;
				return true;
			}
		}
		return false;
	}
}