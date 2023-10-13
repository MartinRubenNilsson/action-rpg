#include "stdafx.h"
#include "defines.h"
#include "ecs_camera.h"
#include "physics_helpers.h"

namespace ecs
{
	extern entt::registry _registry;

	sf::View _current_camera_view;

	void update_cameras(float dt)
	{
		for (auto [entity, camera] : _registry.view<Camera>().each())
		{
			if (_registry.valid(camera.follow_target) &&
				_registry.all_of<b2Body*>(camera.follow_target))
			{
				b2Body* body = _registry.get<b2Body*>(camera.follow_target);
				camera.view.setCenter(get_world_center(body));
			}

			// Keep the camera within the confining box
			// unless the box is smaller than the camera,
			// in which case the camera is centered on the box.

			sf::FloatRect camera_box(
				camera.view.getCenter() - camera.view.getSize() / 2.f,
				camera.view.getSize());

			sf::FloatRect confining_box = camera.confining_box;

			if (confining_box.width > 0.f)
			{
				if (camera_box.width < confining_box.width)
				{
					if (camera_box.left < confining_box.left)
						camera_box.left = confining_box.left;
					if (camera_box.left + camera_box.width > confining_box.left + confining_box.width)
						camera_box.left = confining_box.left + confining_box.width - camera_box.width;
				} else {
					camera_box.left = confining_box.left + confining_box.width / 2.f - camera_box.width / 2.f;
				}
			}

			if (confining_box.height > 0.f)
			{
				if (camera_box.height < confining_box.height)
				{
					if (camera_box.top < confining_box.top)
						camera_box.top = confining_box.top;
					if (camera_box.top + camera_box.height > confining_box.top + confining_box.height)
						camera_box.top = confining_box.top + confining_box.height - camera_box.height;
				} else {
					camera_box.top = confining_box.top + confining_box.height / 2.f - camera_box.height / 2.f;
				}
			}

			camera.view.reset(camera_box);
		}

		// TODO: interpolation
		for (auto [entity, camera] : _registry.view<Camera>().each()) {
			_current_camera_view = camera.view;
		}
	}

	const sf::View& get_current_camera_view() {
		return _current_camera_view;
	}
}