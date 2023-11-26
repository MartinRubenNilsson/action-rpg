#include "stdafx.h"
#include "ecs_player.h"
#include "ecs.h"
#include "ecs_tiled.h"
#include "ecs_physics.h"
#include "math_vectors.h"
#include "physics_helpers.h"
#include "console.h"
#include "audio.h"
#include "window.h"
#include "ui_hud.h"
#include "ui_textbox.h"

namespace ecs
{
	extern entt::registry _registry;

	const float PLAYER_WALK_SPEED = 4.5f;
	const float PLAYER_RUN_SPEED = 8.5f;
	const float PLAYER_HURT_COOLDOWN_TIME = 1.0f;

	void _update_player_input(PlayerInput& input)
	{
		input.direction.x -= sf::Keyboard::isKeyPressed(sf::Keyboard::Left);
		input.direction.x += sf::Keyboard::isKeyPressed(sf::Keyboard::Right);
		input.direction.y -= sf::Keyboard::isKeyPressed(sf::Keyboard::Up);
		input.direction.y += sf::Keyboard::isKeyPressed(sf::Keyboard::Down);
		input.direction = normalize(input.direction);
		input.run = sf::Keyboard::isKeyPressed(sf::Keyboard::X);
	}

	void process_event_player(const sf::Event& event)
	{
		for (auto [entity, player] : _registry.view<Player>().each())
		{
			if (event.type == sf::Event::KeyPressed)
			{
				if (event.key.code == sf::Keyboard::C)
					player.input.interact = true;
			}
		}
	}

	void update_player(float dt)
	{
		// Update player input
		if (window::has_focus() && !console::is_showing())
		{
			for (auto [entity, player] : _registry.view<Player>().each())
				_update_player_input(player.input);
		}

		// Update player physics
		for (auto [player_entity, player, body] : _registry.view<Player, b2Body*>().each())
		{
			float input_speed = player.input.run ? PLAYER_RUN_SPEED : PLAYER_WALK_SPEED;
			sf::Vector2f input_velocity = player.input.direction * input_speed;
			set_linear_velocity(body, input_velocity);

			if (!is_zero(player.input.direction))
				player.state.direction = player.input.direction;

			sf::Vector2f center = get_world_center(body);
			if (player.input.interact)
			{
				sf::Vector2f aabb_center = center + player.state.direction;
				sf::Vector2f aabb_min = aabb_center - sf::Vector2f(0.5f, 0.5f);
				sf::Vector2f aabb_max = aabb_center + sf::Vector2f(0.5f, 0.5f);
				for (entt::entity entity : query_aabb(aabb_min, aabb_max))
				{
					if (entity == player_entity)
						continue;

					std::string type = get_class(entity);

					if (type.starts_with("enemy"))
					{
						destroy_at_end_of_frame(entity);
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
		{
			player.input = {};
			ui::hud_player_health = player.state.health;
			if (player.state.hurt_cooldown_timer > 0.f)
				player.state.hurt_cooldown_timer -= dt;
		}
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

	void hurt_player(int health_to_remove)
	{
		if (health_to_remove <= 0) return;
		for (auto [entity, player] : _registry.view<Player>().each())
		{
			if (player.state.health <= 0) continue; // Player is already dead
			if (player.state.hurt_cooldown_timer > 0.f) continue; // Player is still invulnerable
			audio::play("event:/snd_player_hurt");
			player.state.health = std::max(0, player.state.health - health_to_remove);
			player.state.hurt_cooldown_timer = PLAYER_HURT_COOLDOWN_TIME;
			if (player.state.health > 0) continue; // Player is still alive
			audio::play("event:/snd_player_die");
			destroy_at_end_of_frame(entity);
		}
	}
}

