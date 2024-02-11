#include "stdafx.h"
#include "ecs_camera.h"
#include <tweeny/easing.h>
#include "physics_helpers.h"
#include "random_noise.h"
#include "console.h"
#include "window.h"

namespace ecs
{
	const sf::View Camera::DEFAULT_VIEW = window::get_default_view();
	const float _CAMERA_BLEND_DURATION = 1.f;

	extern entt::registry _registry;

	entt::entity _active_camera_entity = entt::null;
	sf::View _last_camera_view = Camera::DEFAULT_VIEW;
	sf::View _active_camera_view = Camera::DEFAULT_VIEW;
	sf::View _blended_camera_view = Camera::DEFAULT_VIEW;
	float _camera_shake_time = 0.f;
	float _camera_blend_time = _CAMERA_BLEND_DURATION;

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

		if (confining_rect.width > 0.f) {
			if (result.width < confining_rect.width) {
				if (result.left < confining_rect.left)
					result.left = confining_rect.left;
				if (result.left + result.width > confining_rect.left + confining_rect.width)
					result.left = confining_rect.left + confining_rect.width - result.width;
			} else {
				result.left = confining_rect.left + confining_rect.width / 2.f - result.width / 2.f;
			}
		}

		if (confining_rect.height > 0.f) {
			if (result.height < confining_rect.height) {
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
		// Update timers.
		_camera_shake_time += dt;
		_camera_blend_time = std::clamp(_camera_blend_time + dt, 0.f, _CAMERA_BLEND_DURATION);

		for (auto [entity, camera] : _registry.view<Camera>().each()) {
			// If the camera has a follow target, center the view on the target.
			if (_registry.valid(camera.follow) &&
				_registry.all_of<b2Body*>(camera.follow)) {
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
					random::perlin_noise_f(0, camera.shake_frequency * _camera_shake_time);
				shake_offset.y = total_shake_amplitude *
					random::perlin_noise_f(1, camera.shake_frequency * _camera_shake_time);
			}
			camera._shaky_view = camera.view;
			camera._shaky_view.move(shake_offset);

			// Confine the shaky view.
			_confine(camera._shaky_view, camera.confining_rect);
		}

		// Update the active camera view.
		if (Camera* camera = _registry.try_get<Camera>(_active_camera_entity)) {
			_active_camera_view = camera->_shaky_view;
		} else {
			_active_camera_view = Camera::DEFAULT_VIEW;
		}

		// Update the blended camera view.
		float blend_factor = _camera_blend_time / _CAMERA_BLEND_DURATION;
		_blended_camera_view.setCenter(
			tweeny::easing::exponentialOut.run(blend_factor,
				_last_camera_view.getCenter(),
				_active_camera_view.getCenter()));
	}

	const sf::View& get_active_camera_view() {
		return _active_camera_view;
	}

	const sf::View& get_blended_camera_view() {
		return _blended_camera_view;
	}

	bool activate_camera(entt::entity entity, bool hard_cut)
	{
		if (!_registry.all_of<Camera>(entity)) return false;
		_active_camera_entity = entity;
		_last_camera_view = _active_camera_view;
		_camera_blend_time = hard_cut ? _CAMERA_BLEND_DURATION : 0.f;
		return true;
	}

	void emplace_camera(entt::entity entity, const Camera& camera) {
		_registry.emplace_or_replace<Camera>(entity, camera);
	}

	entt::entity detach_camera(entt::entity entity)
	{
		Camera* camera = _registry.try_get<Camera>(entity);
		if (!camera) return entt::null;
		camera->follow = entt::null;
		entt::entity new_entity = _registry.create();
		_registry.emplace<Camera>(new_entity, *camera);
		_registry.remove<Camera>(entity);
		if (_active_camera_entity == entity)
			_active_camera_entity = new_entity;
		return new_entity;
	}

	bool add_camera_trauma(entt::entity entity, float trauma)
	{
		if (Camera* camera = _registry.try_get<Camera>(entity)) {
			camera->trauma += trauma;
			return true;
		}
		return false;
	}

	bool add_camera_trauma(float trauma)
	{
		if (_active_camera_entity == entt::null) return false;
		return add_camera_trauma(_active_camera_entity, trauma);
	}
}