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
#include "tables.h"

Timer::Timer(float duration)
	: duration(duration)
{}

void Timer::start() {
	time_left = duration;
}

void Timer::stop() {
	time_left = 0.f;
}

bool Timer::update(float dt)
{
	dt = std::max(0.f, dt);
	if (time_left > 0.f) {
		time_left -= dt;
		if (time_left <= 0.f) {
			time_left = 0.f;
			return true;
		}
	}
	return false;
}

bool Timer::finished() const {
	return time_left <= 0.f;
}

float Timer::get_progress() const
{
	if (duration > 0.f)
		return 1.f - time_left / duration;
	return 1.f;
}

namespace ecs
{
	extern entt::registry _registry;

	const float PLAYER_WALK_SPEED = 4.5f;
	const float PLAYER_RUN_SPEED = 8.5f;
	const float PLAYER_HURT_COOLDOWN_DURATION = 1.0f;
	const float PLAYER_STEP_COOLDOWN_DURATION = 0.3f;

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
		for (auto [entity, player] : _registry.view<Player>().each()) {
			if (event.type == sf::Event::KeyPressed) {
				if (event.key.code == sf::Keyboard::C)
					player.input.interact = true;
			}
		}
	}

	void update_player(float dt)
	{
		// Update player input
		if (window::has_focus() && !console::is_showing()) {
			for (auto [entity, player] : _registry.view<Player>().each())
				_update_player_input(player.input);
		}

		for (auto [player_entity, player, body] : _registry.view<Player, b2Body*>().each()) {
			float input_speed = player.input.run ? PLAYER_RUN_SPEED : PLAYER_WALK_SPEED;
			sf::Vector2f input_velocity = player.input.direction * input_speed;
			set_linear_velocity(body, input_velocity);
			sf::Vector2f center = get_world_center(body);

			if (is_zero(player.input.direction)) {
				player.step_timer.start();
			} else { // if player is moving
				player.state.direction = player.input.direction;
				if (player.step_timer.update(dt)) {
					player.step_timer.start();
					map::play_footstep_sound_at(center);
				}
			}

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
						if (get_string(entity, "text", string)) {
							auto entries = tables::query_textbox_data_entries(string);
							for (auto& entry : entries) {
								ui::Textbox textbox;
								textbox.text = entry.text;
								textbox.sprite = entry.sprite;
								textbox.typing_speed = entry.speed;
								textbox.typing_sound = entry.sound;
								ui::push_textbox(textbox);
							}
							ui::pop_textbox();
						}
						if (get_string(entity, "sound", string))
							audio::play("event:/" + string);
					}
				}
			}
		}

		// Update player state
		for (auto [entity, player] : _registry.view<Player>().each()) {
			player.input = {};
			player.hurt_timer.update(dt);
		}

		// Update player sprite
		for (auto [entity, player, sprite] : _registry.view<const Player, Sprite>().each()) {
			sf::Color color = sf::Color::White;
			if (!player.hurt_timer.finished()) {
				float fraction = fmod(player.hurt_timer.get_time_elapsed(), 0.15f) / 0.15f;
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

	sf::Vector2f get_player_center()
	{
		for (auto [entity, player, body] : _registry.view<Player, b2Body*>().each())
			return get_world_center(body);
		return sf::Vector2f();
	}

	void hurt_player(int health_to_remove)
	{
		if (health_to_remove <= 0) return;
		for (auto [entity, player] : _registry.view<Player>().each()) {
			if (player.state.health <= 0) continue; // Player is already dead
			if (!player.hurt_timer.finished()) continue; // Player is invulnerable
			player.hurt_timer.start();
			player.state.health = std::max(0, player.state.health - health_to_remove);
			if (player.state.health > 0) {
				audio::play("event:/snd_player_hurt");
			} else {
				audio::stop_all();
				audio::play("event:/snd_player_die");
				audio::play("event:/mus_coffin_dance");
				detach_camera(entity);
				destroy_at_end_of_frame(entity);
			}
		}
	}
}
