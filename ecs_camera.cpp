#include "stdafx.h"
#include "defines.h"
#include "ecs_camera.h"
#include "physics_helpers.h"
#include "random_noise.h"
#include "console.h"

namespace ecs
{
	extern entt::registry _registry;

	float _shake_time = 0.f;
	sf::View _current_camera_view;
	
	sf::View get_default_camera_view() {
		return sf::View(sf::FloatRect(0.f, 0.f, VIEW_WIDTH, VIEW_HEIGHT));
	}

	sf::View get_current_camera_view() {
		return _current_camera_view;
	}

	// Assumes that the view has a rotation angle of 0.
	sf::FloatRect _get_rect(const sf::View& view)
	{
		return sf::FloatRect(
			view.getCenter() - view.getSize() / 2.f,
			view.getSize());
	}

	sf::FloatRect _confine(const sf::FloatRect& rect, const sf::FloatRect& confining_rect)
	{
		sf::FloatRect result = rect;

		if (confining_rect.width > 0.f)
		{
			if (result.width < confining_rect.width)
			{
				if (result.left < confining_rect.left)
					result.left = confining_rect.left;
				if (result.left + result.width > confining_rect.left + confining_rect.width)
					result.left = confining_rect.left + confining_rect.width - result.width;
			} else {
				result.left = confining_rect.left + confining_rect.width / 2.f - result.width / 2.f;
			}
		}

		if (confining_rect.height > 0.f)
		{
			if (result.height < confining_rect.height)
			{
				if (result.top < confining_rect.top)
					result.top = confining_rect.top;
				if (result.top + result.height > confining_rect.top + confining_rect.height)
					result.top = confining_rect.top + confining_rect.height - result.height;
			} else {
				result.top = confining_rect.top + confining_rect.height / 2.f - result.height / 2.f;
			}
		}

		return result;
	}

	// Assumes that the view has a rotation angle of 0.
	void _confine(sf::View& view, const sf::FloatRect& confining_rect) {
		view.reset(_confine(_get_rect(view), confining_rect));
	}

	void update_cameras(float dt)
	{
		_shake_time += dt;

		for (auto [entity, camera] : _registry.view<Camera>().each())
		{
			// If the camera has a follow target, center the view on the target.
			if (_registry.valid(camera.follow) &&
				_registry.all_of<b2Body*>(camera.follow))
			{
				camera.view.setCenter(get_world_center(
					_registry.get<b2Body*>(camera.follow)));
			}

			// Confine the view.
			_confine(camera.view, camera.confining_rect);

			// Update the trauma.
			camera.trauma -= camera.trauma_decay * dt;
			camera.trauma = std::clamp(camera.trauma, 0.f, 1.f);

			// Compute the shaky view.
			sf::Vector2f shake_offset;
			if (camera.shake_amplitude && camera.shake_frequency && camera.trauma) {
				float total_shake_amplitude = camera.shake_amplitude *
					camera.trauma * camera.trauma;
				shake_offset.x = total_shake_amplitude *
					random::perlin_noise_f(0, camera.shake_frequency * _shake_time);
				shake_offset.y = total_shake_amplitude *
					random::perlin_noise_f(1, camera.shake_frequency * _shake_time);
			}
			camera._shaky_view = camera.view;
			camera._shaky_view.move(shake_offset);

			// Confine the shaky view.
			_confine(camera._shaky_view, camera.confining_rect);
		}

		// Find the camera with the highest priority and use its view.
		// If there are multiple cameras with the same priority, the first one found is used.

		Camera* highest_priority_camera = nullptr;
		for (auto [entity, camera] : _registry.view<Camera>().each())
		{
			if (!highest_priority_camera || camera.priority > highest_priority_camera->priority)
				highest_priority_camera = &camera;
		}

		_current_camera_view = get_default_camera_view();
		if (highest_priority_camera)
			_current_camera_view = highest_priority_camera->_shaky_view;
	}
}