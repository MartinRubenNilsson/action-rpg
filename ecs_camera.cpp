#include "stdafx.h"
#include "ecs_camera.h"
#include "physics_helpers.h"
#include "random.h"
#include "easings.h"

namespace ecs
{
	const float _CAMERA_BLEND_DURATION = 1.f;

	extern entt::registry _registry;

	entt::entity _active_camera_entity = entt::null;
	CameraView _last_camera_view;
	CameraView _active_camera_view;
	CameraView _blended_camera_view;
	float _camera_shake_time = 0.f;
	float _camera_blend_time = _CAMERA_BLEND_DURATION;

	CameraView _confine_camera_view(const CameraView& view, const sf::Vector2f& confines_min, const sf::Vector2f& confines_max)
	{
		const sf::Vector2f camera_center_min = confines_min + view.size / 2.f;
		const sf::Vector2f camera_center_max = confines_max - view.size / 2.f;
		CameraView result = view;
		if (camera_center_min.x < camera_center_max.x) {
			result.center.x = std::clamp(result.center.x, camera_center_min.x, camera_center_max.x);
		} else {
			result.center.x = (confines_min.x + confines_max.x) / 2.f;
		}
		if (camera_center_min.y < camera_center_max.y) {
			result.center.y = std::clamp(result.center.y, camera_center_min.y, camera_center_max.y);
		} else {
			result.center.y = (confines_min.y + confines_max.y) / 2.f;
		}
		return result;
	}

	void update_cameras(float dt)
	{
		// Update timers.
		_camera_shake_time += dt;
		_camera_blend_time = std::clamp(_camera_blend_time + dt, 0.f, _CAMERA_BLEND_DURATION);

		for (auto [entity, camera] : _registry.view<Camera>().each()) {
			// If the camera has a follow target, center the view on the target.
			if (_registry.valid(camera.follow) && _registry.all_of<b2Body*>(camera.follow)) {
				camera.view.center = get_world_center(_registry.get<b2Body*>(camera.follow));
			}

			// Confine the view.
			camera.view = _confine_camera_view(camera.view, camera.confines_min, camera.confines_max);

			// Update the trauma.
			camera.trauma = std::clamp(camera.trauma - camera.trauma_decay * dt, 0.f, 1.f);

			// Compute the shaky view.
			sf::Vector2f shake_offset;
			if (camera.shake_amplitude && camera.shake_frequency && camera.trauma) {
				float total_shake_amplitude = camera.shake_amplitude * camera.trauma * camera.trauma;
				shake_offset.x = total_shake_amplitude *
					random::fractal_perlin_noise(0, camera.shake_frequency * _camera_shake_time);
				shake_offset.y = total_shake_amplitude *
					random::fractal_perlin_noise(1, camera.shake_frequency * _camera_shake_time);
			}
			camera._shaken_view = camera.view;
			camera._shaken_view.center += shake_offset;

			// Confine the shaky view.
			camera._shaken_view = _confine_camera_view(camera._shaken_view, camera.confines_min, camera.confines_max);
		}

		// Update the active camera view.
		if (Camera* camera = _registry.try_get<Camera>(_active_camera_entity)) {
			_active_camera_view = camera->_shaken_view;
		} else {
			_active_camera_view = {};
		}

		// Update the blended camera view.
		float blend_factor = ease_out_expo(_camera_blend_time / _CAMERA_BLEND_DURATION);
		_blended_camera_view.size = lerp(_last_camera_view.size, _active_camera_view.size, blend_factor);
		_blended_camera_view.center = lerp(_last_camera_view.center, _active_camera_view.center, blend_factor);
	}

	const CameraView& get_active_camera_view() {
		return _active_camera_view;
	}

	const CameraView& get_blended_camera_view() {
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

	bool add_trauma_to_camera(entt::entity entity, float trauma)
	{
		if (Camera* camera = _registry.try_get<Camera>(entity)) {
			camera->trauma += trauma;
			return true;
		}
		return false;
	}

	bool add_trauma_to_active_camera(float trauma)
	{
		if (_active_camera_entity == entt::null) return false;
		return add_trauma_to_camera(_active_camera_entity, trauma);
	}
}