#include "stdafx.h"
#include "ecs_player.h"
#include "ecs.h"
#include "ecs_tiled.h"
#include "ecs_physics.h"
#include "math_vectors.h"
#include "physics_helpers.h"
#include "console.h"
#include "window.h"
#include "ui_textbox.h"
#include "audio.h"

namespace ecs
{
	extern entt::registry _registry;

	const float PLAYER_WALK_SPEED = 4.5f;
	const float PLAYER_RUN_SPEED = 8.5f;

	void _update_inputs(Player& player)
	{
		player.input_direction.x -= sf::Keyboard::isKeyPressed(sf::Keyboard::Left);
		player.input_direction.x += sf::Keyboard::isKeyPressed(sf::Keyboard::Right);
		player.input_direction.y -= sf::Keyboard::isKeyPressed(sf::Keyboard::Up);
		player.input_direction.y += sf::Keyboard::isKeyPressed(sf::Keyboard::Down);
		player.input_direction = normalize(player.input_direction);
		player.input_run = sf::Keyboard::isKeyPressed(sf::Keyboard::X);
	}

	void _clear_inputs(Player& player)
	{
		player.input_direction = sf::Vector2f();
		player.input_run = false;
		player.input_interact = false;
	}

	void process_event_player(const sf::Event& event)
	{
		for (auto [entity, player] : _registry.view<Player>().each())
		{
			if (event.type == sf::Event::KeyPressed)
			{
				if (event.key.code == sf::Keyboard::C)
					player.input_interact = true;
			}
		}
	}

	void update_player(float dt)
	{
		if (window::has_focus() && !console::is_showing())
		{
			for (auto [player_entity, player] : _registry.view<Player>().each())
				_update_inputs(player);
		}

		for (auto [player_entity, player, body] : _registry.view<Player, b2Body*>().each())
		{
			float input_speed = player.input_run ? PLAYER_RUN_SPEED : PLAYER_WALK_SPEED;
			sf::Vector2f input_velocity = player.input_direction * input_speed;
			set_linear_velocity(body, input_velocity);

			if (!is_zero(player.input_direction))
				player.direction = player.input_direction;

			sf::Vector2f center = get_world_center(body);
			if (player.input_interact)
			{
				sf::Vector2f aabb_center = center + player.direction;
				sf::Vector2f aabb_min = aabb_center - sf::Vector2f(0.5f, 0.5f);
				sf::Vector2f aabb_max = aabb_center + sf::Vector2f(0.5f, 0.5f);
				for (entt::entity entity : query_aabb(aabb_min, aabb_max))
				{
					if (entity == player_entity)
						continue;

					std::string type = get_class(entity);

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
		}

		for (auto [entity, player] : _registry.view<Player>().each())
			_clear_inputs(player);
	}

	bool player_exists() {
		return !_registry.view<Player>().empty();
	}

	sf::Vector2f get_player_center()
	{
		for (auto [entity, player, body] : _registry.view<Player, b2Body*>().each())
			return get_world_center(body);
		return sf::Vector2f();
	}
}

