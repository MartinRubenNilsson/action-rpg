#include "ecs_player.h"
#include "ecs_common.h"
#include "ecs_physics.h"
#include "math_vectors.h"
#include "physics_helpers.h"
#include "console.h"
#include "window.h"
#include "ui_textbox.h"
#include "audio.h"

#define PLAYER_WALK_SPEED 4.5f
#define PLAYER_RUN_SPEED 8.5f
bool _player_in_stealth_mode = false;
float _player_noise_level = 0.0f;


namespace ecs
{
	extern entt::registry _registry;

	entt::entity _player_entity = entt::null;

	sf::Vector2f _player_input_direction;
	bool _player_input_run = false;
	bool _player_input_interact = false;

	sf::Vector2f _player_direction;

	bool player_exists() {
		return _registry.valid(_player_entity);
	}

	void set_player_entity(entt::entity entity) {
		_player_entity = entity;
	}

	entt::entity get_player_entity() {
		return _player_entity;
	}

	void set_player_center(const sf::Vector2f& center)
	{
		if (auto body = _registry.try_get<b2Body*>(_player_entity))
			set_world_center(*body, center);
	}

	sf::Vector2f get_player_center() {
		if (auto body = _registry.try_get<b2Body*>(_player_entity))
			return get_world_center(*body);
		return sf::Vector2f();
	}

	void _update_player_inputs()
	{
		_player_input_direction.x -= sf::Keyboard::isKeyPressed(sf::Keyboard::Left);
		_player_input_direction.x += sf::Keyboard::isKeyPressed(sf::Keyboard::Right);
		_player_input_direction.y -= sf::Keyboard::isKeyPressed(sf::Keyboard::Up);
		_player_input_direction.y += sf::Keyboard::isKeyPressed(sf::Keyboard::Down);
		_player_input_direction = normalize(_player_input_direction);

		_player_input_run = sf::Keyboard::isKeyPressed(sf::Keyboard::X);

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z)) {
			_player_in_stealth_mode = !_player_in_stealth_mode;
		}
	}

	void _clear_player_inputs()
	{
		_player_input_direction = sf::Vector2f();
		_player_input_run = false;
		_player_input_interact = false;
	}

	void process_event_player(const sf::Event& event)
	{
		if (event.type == sf::Event::KeyPressed)
		{
			if (event.key.code == sf::Keyboard::C)
				_player_input_interact = true;
		}
	}

	// NEW: Function to update noise level
	void update_noise_level() {
		_player_noise_level = _player_in_stealth_mode ? 0.1f : 1.0f;
	}

	void update_player()
	{
		_player_entity = ecs::find_entity_by_name("player");

		if (!_registry.valid(_player_entity))
			return;
		if (!_registry.all_of<b2Body*>(_player_entity))
			return;

		auto body = _registry.get<b2Body*>(_player_entity);
		const sf::Vector2f center = get_world_center(body);

		// UPDATE INPUTS

		if (window::has_focus() && !console::is_visible())
			_update_player_inputs(); 

		// NEW: Update velocity
		const float input_speed = (_player_input_run ? PLAYER_RUN_SPEED : PLAYER_WALK_SPEED) * (_player_in_stealth_mode ? 0.5f : 1.0f);
		const sf::Vector2f input_velocity = _player_input_direction * input_speed;
		set_linear_velocity(body, input_velocity);

		// NEW: Update noise level
		update_noise_level();


		// UPDATE DIRECTION

		if (!is_zero(_player_input_direction))
			_player_direction = _player_input_direction;

		// INTERACT

		if (_player_input_interact)
		{
			sf::Vector2f aabb_center = center + _player_direction;
			sf::Vector2f aabb_min = aabb_center - sf::Vector2f(0.5f, 0.5f);
			sf::Vector2f aabb_max = aabb_center + sf::Vector2f(0.5f, 0.5f);
			for (entt::entity entity : query_aabb(aabb_min, aabb_max))
			{
				if (entity == _player_entity)
					continue;

				std::string type = get_type(entity);

				if (type.starts_with("enemy"))
				{
					mark_for_destruction(entity);
				}
				else
				{
					std::string string;
					if (get_string(entity, "text", string))
						ui::set_textbox_entries(string);
					if (get_string(entity, "sound", string))
						audio::play("event:/" + string);
				}
			}
		}

		// CLEANUP

		_clear_player_inputs();
	}
}

