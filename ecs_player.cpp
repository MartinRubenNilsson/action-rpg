#include "stdafx.h"
#include "ecs_player.h"
#include "ecs.h"
#include "ecs_tiled.h"
#include "ecs_physics.h"
#include "ecs_graphics.h"
#include "ecs_camera.h"
#include "map.h"
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

	void _update_player_input(PlayerInput& input)
	{
		input.direction = sf::Vector2f();
		input.direction.x -= sf::Keyboard::isKeyPressed(sf::Keyboard::Left);
		input.direction.x += sf::Keyboard::isKeyPressed(sf::Keyboard::Right);
		input.direction.y -= sf::Keyboard::isKeyPressed(sf::Keyboard::Up);
		input.direction.y += sf::Keyboard::isKeyPressed(sf::Keyboard::Down);
		input.direction = normalize(input.direction);
		input.run = sf::Keyboard::isKeyPressed(sf::Keyboard::X);
	}

	void process_event_player(const sf::Event& event)
	{
		for (auto [entity, player] : _registry.view<Player>().each()) {
			if (event.type == sf::Event::KeyPressed) {
				if (event.key.code == sf::Keyboard::C)
					player.input.interact = true;
			}
		}
	}

	void update_player(float dt)
	{
		for (auto [entity, player] : _registry.view<Player>().each()) {
			player.hurt_timer.update(dt);
			if (player.kill_timer.update(dt)) {
				kill_player(entity);
				return;
			}
			player.input = {};
			if (player.kill_timer.stopped() && window::has_focus() && !console::is_showing()) {
				_update_player_input(player.input);
			}
		}

		for (auto [player_entity, player, body] : _registry.view<Player, b2Body*>().each()) {
			sf::Vector2f center = get_world_center(body);

			float speed = 0.f;
			if (player.kill_timer.stopped()) {
				if (is_zero(player.input.direction)) {
					player.step_timer.start();
				} else { // if player is moving
					speed = player.input.run ? PLAYER_RUN_SPEED : PLAYER_WALK_SPEED;
					player.state.direction = player.input.direction;
					if (player.step_timer.update(dt)) {
						player.step_timer.start();
						map::play_footstep_sound_at(center);
					}
				}
			} else {
				// Player is dying, let's spin!!
				speed = 0.001f;
				float spin_speed = 30.f * (1.f - player.kill_timer.get_progress());
				player.state.direction = rotate(player.state.direction, spin_speed * dt);
			}

			set_linear_velocity(body, player.state.direction * speed);

			if (player.input.interact) {
				sf::Vector2f aabb_center = center + player.state.direction;
				sf::Vector2f aabb_min = aabb_center - sf::Vector2f(0.5f, 0.5f);
				sf::Vector2f aabb_max = aabb_center + sf::Vector2f(0.5f, 0.5f);
				for (entt::entity entity : query_aabb(aabb_min, aabb_max)) {
					if (entity == player_entity) continue;
					std::string type = get_class(entity);
					if (type.starts_with("enemy")) {
						destroy_at_end_of_frame(entity);
					} else {
						std::string string;
						if (get_string(entity, "textbox", string))
							ui::open_textbox_preset(string);
						if (get_string(entity, "sound", string))
							audio::play("event:/" + string);
					}
				}
			}
		}

		// Update animation
		for (auto [entity, player, anim, body] : _registry.view<Player, Animation, b2Body*>().each()) {
			sf::Vector2f velocity = get_linear_velocity(body);
			float speed = length(velocity);
			std::string tile_class;
			if (speed >= PLAYER_RUN_SPEED) {
				tile_class = "run";
			} else if (speed >= PLAYER_WALK_SPEED) {
				tile_class = "walk";
			} else {
				tile_class = "idle";
			}
			tile_class += "_";
			tile_class += get_direction(player.state.direction);
			if (!anim.is_playing(tile_class)) {
				anim.play(tile_class);
				console::log(tile_class);
			}
		}

		// Update sprite
		for (auto [entity, player, sprite] : _registry.view<const Player, Sprite>().each()) {
			sf::Color color = sf::Color::White;
			if (player.kill_timer.stopped() && player.hurt_timer.started()) {
				float fraction = fmod(player.hurt_timer.get_time(), 0.15f) / 0.15f;
				color.a = (sf::Uint8)(255 * fraction);
			}
			sprite.sprite.setColor(color);
		}

		// Update HUD
		ui::hud_player_health = 0;
		for (auto [entity, player] : _registry.view<const Player>().each()) {
			ui::hud_player_health = player.state.health;
		}
	}

	void emplace_player(entt::entity entity, const Player& player) {
		_registry.emplace<Player>(entity, player);
	}

	bool player_exists() {
		return !_registry.view<Player>().empty();
	}

	entt::entity get_player_entity() {
		return _registry.view<Player>().front();
	}

	sf::Vector2f get_player_world_center()
	{
		for (auto [entity, player, body] : _registry.view<Player, b2Body*>().each()) {
			return get_world_center(body);
		}
		return sf::Vector2f();
	}

	bool kill_player(entt::entity entity)
	{
		if (!_registry.all_of<Player>(entity)) return false;
		detach_camera(entity);
		destroy_at_end_of_frame(entity);
		audio::stop_all();
		audio::play("event:/snd_player_die");
		audio::play("event:/mus_coffin_dance");
		ui::open_textbox_preset("player_die_01");
		ui::hud_player_health = 0;
		return true;
	}

	bool hurt_player(entt::entity entity, int health_to_remove)
	{
		if (health_to_remove <= 0) return false;
		if (!_registry.all_of<Player>(entity)) return false;
		Player& player = _registry.get<Player>(entity);
		if (player.state.health <= 0) return false; // Player is already dead
		if (player.hurt_timer.started()) return false; // Player is invulnerable
		player.hurt_timer.start();
		player.state.health = std::max(0, player.state.health - health_to_remove);
		if (player.state.health > 0) { // Player survived
			audio::play("event:/snd_player_hurt");
		} else { // Player died
			player.kill_timer.start();
		}
		return true;
	}
}
