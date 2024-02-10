#include "stdafx.h"
#include "ecs_player.h"
#include "ecs.h"
#include "ecs_tiled.h"
#include "ecs_physics.h"
#include "ecs_physics_filters.h"
#include "ecs_tile.h"
#include "ecs_camera.h"
#include "map.h"
#include "math_vectors.h"
#include "physics_helpers.h"
#include "console.h"
#include "audio.h"
#include "window.h"
#include "ui_hud.h"
#include "ui_textbox.h"
#include "ecs_arrow.h"
#include "ecs_pickups.h"
#include "random.h"
#include "tiled.h"

namespace ecs
{
	extern entt::registry _registry;

	const float PLAYER_WALK_SPEED = 72.f;
	const float PLAYER_RUN_SPEED = 136.f;
	const float PLAYER_STEALTH_SPEED = 36.f;
	const float ARROW_SPEED = 160.f;

	void process_event_player(const sf::Event& ev)
	{
		for (auto [entity, player] : _registry.view<Player>().each()) {
			if (ev.type == sf::Event::KeyPressed) {
				if (ev.key.code == sf::Keyboard::C)
					player.input.interact = true;
				if (ev.key.code == sf::Keyboard::Z)
					player.input.arrow_attack = true;
			}
		}
	}

	void fire_arrow(const sf::Vector2f& position, const sf::Vector2f& direction, int damage) {
		// Calculate the offset position
		float offsetDistance = 1.0f; // Adjust this value as needed
		sf::Vector2f offset = normalize(direction) * offsetDistance;
		sf::Vector2f fire_position = position + offset;

		entt::entity arrow_entity = _registry.create();
		set_class(arrow_entity, "arrow");

		// Setup the Arrow component
		Arrow arrow = { damage, 5.0f };
		_registry.emplace<Arrow>(arrow_entity, arrow);

		// Create a physics body for the arrow
		b2BodyDef bodyDef;
		bodyDef.type = b2_dynamicBody;
		bodyDef.position.Set(fire_position.x, fire_position.y); // Set the offset position here
		b2Body* body = emplace_body(arrow_entity, bodyDef);
		b2CircleShape shape;
		shape.m_p.x = 0;
		shape.m_p.y = 0;
		shape.m_radius = 6.f;
		b2FixtureDef fixture_def;
		fixture_def.shape = &shape;
		fixture_def.density = 1.f;
		fixture_def.filter = get_filter_for_class("arrow");
		body->CreateFixture(&fixture_def);

		// Set the velocity of the arrow
		sf::Vector2f arrow_velocity = normalize(direction) * ARROW_SPEED;
		body->SetLinearVelocity(b2Vec2(arrow_velocity.x, arrow_velocity.y));

		audio::play("event:/snd_fire_arrow");

		// Add additional components like Tile here if necessary
		if (Tile* tile = emplace_tile(arrow_entity, "items1", "arrow")) {
			tile->pivot = sf::Vector2f(6.f, 6.f);
		}
	}

	void update_player(float dt)
	{
		for (auto [player_entity, player, body, tile] :
			_registry.view<Player, b2Body*, Tile>().each()) {

			const sf::Vector2f player_position = get_world_center(body);
			const sf::Vector2f player_velocity = get_linear_velocity(body);

			// UPDATE AUDIO LISTENER POSITION

			audio::set_listener_position(player_position);

			// UPDATE TIMERS

			player.hurt_timer.update(dt);
			if (player.kill_timer.update(dt)) { // If player died this frame
				kill_player(player_entity);
				continue;
			}

			// UPDATE INPUT

			if (player.kill_timer.finished() && window::has_focus() && !console::is_visible()) {
				player.input.axis_x -= sf::Keyboard::isKeyPressed(sf::Keyboard::Left);
				player.input.axis_x += sf::Keyboard::isKeyPressed(sf::Keyboard::Right);
				player.input.axis_y -= sf::Keyboard::isKeyPressed(sf::Keyboard::Up);
				player.input.axis_y += sf::Keyboard::isKeyPressed(sf::Keyboard::Down);
				player.input.run = sf::Keyboard::isKeyPressed(sf::Keyboard::X);
				player.input.stealth = sf::Keyboard::isKeyPressed(sf::Keyboard::V);
			}

			// TODO Lock player movement while firing an arrow
			if (tile.get_class().starts_with("bow_shot") && tile.animation_timer.running()) {
				// Set the player's velocity to zero
				set_linear_velocity(body, sf::Vector2f(0.f, 0.f));
				continue; // Skip the rest of the update loop
			}


			// UPDATE PHYSICS

			sf::Vector2f movement_direction;
			float movement_speed = 0.f;

			if (player.kill_timer.finished()) { // Player is alive
				if (player.input.axis_x || player.input.axis_y) {

					// Determine movement direction
					movement_direction.x = (float)player.input.axis_x;
					movement_direction.y = (float)player.input.axis_y;
					movement_direction = normalize(movement_direction);

					// Determine movement speed
					if (player.input.stealth) {
						movement_speed = PLAYER_STEALTH_SPEED;
					} else if (player.input.run) {
						movement_speed = PLAYER_RUN_SPEED;
					} else {
						movement_speed = PLAYER_WALK_SPEED;
					}
				}
			} else { // Player is dying
				float spin_speed = 30.f * (1.f - player.kill_timer.get_progress());
				player.facing_direction = rotate(player.facing_direction, spin_speed * dt);
			}

			set_linear_velocity(body, movement_direction * movement_speed);

			if (!is_zero(movement_direction)) {
				player.facing_direction = movement_direction;
			}

			// HANDLE INTERACTION

			if (player.input.interact) {
				sf::Vector2f box_center = player_position + player.facing_direction * 16.f;
				sf::Vector2f box_min = box_center - sf::Vector2f(6.f, 6.f);
				sf::Vector2f box_max = box_center + sf::Vector2f(6.f, 6.f);
				std::unordered_set<std::string> audio_events_to_play; //So we don't play the same sound twice
				for (const BoxHit& hit : boxcast(box_min, box_max)) {
					if (hit.entity == player_entity) continue;
					std::string class_ = get_class(hit.entity);
					if (class_ == "slime") {
						destroy_at_end_of_frame(hit.entity);
					} else if (Tile* tile = _registry.try_get<Tile>(hit.entity)) {
						std::string tile_class = tile->get_class();
						if (tile_class == "grass") {
							audio_events_to_play.insert("event:/snd_cut_grass");
							if (random::coin_flip(0.2f))
								create_arrow_pickup(tile->position + sf::Vector2f(0.5f, 0.5f));
							destroy_at_end_of_frame(hit.entity);
						}
					} else {
						std::string string;
						if (get_string(hit.entity, "textbox", string)) {
							ui::push_textbox_presets(string);
							ui::pop_textbox();
						}
						if (get_string(hit.entity, "sound", string))
							audio_events_to_play.insert(string);
					}
				}
				for (const std::string& audio_event : audio_events_to_play)
					audio::play(audio_event);
			}

			// UPDATE ANIMATION

			{
				using namespace std::literals::string_literals;

				char dir = get_direction(player.facing_direction);

				// Right walk/run animation is a complete cycle.
				// Left walk/run animation is a flipped version of the right animation.
				// Up/down walk/run animation plays once, then plays again flipped, then loops.
				switch (dir) {
				case 'r':
					tile.flip_x = false;
					tile.animation_flip_x_on_loop = false;
					break;
				case 'l':
					dir = 'r';
					tile.flip_x = true;
					tile.animation_flip_x_on_loop = false;
					break;
				case 'u':
				case 'd':
					tile.animation_flip_x_on_loop = true;
					break;
				}

				// TODO HANDLE Arrow ATTACK. Wait 660 ms from animation start to fire the arrow

				// Handling arrow attack
				if (player.input.arrow_attack && player.arrow_ammo > 0) {
					tile.set_class("bow_shot_"s + dir); // Trigger bow shot animation
					player.bow_shot_timer.start(); // Start the timer with 660 ms
					tile.animation_loop = false; // Ensure that the animation will not loop
					player.input.arrow_attack = false;
				}
				else if (movement_speed >= PLAYER_RUN_SPEED) {
					tile.set_class("run_"s + dir);
					tile.animation_speed = 1.2f;
					tile.animation_loop = true;
				}
				else if (movement_speed >= PLAYER_WALK_SPEED) {
					tile.set_class("walk_"s + dir);
					tile.animation_speed = 1.2f;
					tile.animation_loop = true;
				}
				else {
					tile.set_class("idle_"s + dir);
					tile.animation_loop = true;
				}

				// 
				if (player.bow_shot_timer.update(dt)) {
					fire_arrow(player_position, player.facing_direction, 1);
					player.arrow_ammo--;
				}
			}



			// UPDATE COLOR

			sf::Color color = sf::Color::White;
			if (player.kill_timer.finished() && player.hurt_timer.running()) {
				float fraction = fmod(player.hurt_timer.get_time(), 0.15f) / 0.15f;
				color.a = (sf::Uint8)(255 * fraction);
			}
			tile.color = color;

			// UPDATE HUD

			ui::bindings::hud_player_health = player.health;

			// CLEAR INPUT

			player.input = {};
		}
	}

	void debug_player()
	{
		for (auto [entity, player] : _registry.view<Player>().each()) {
			ImGui::Begin("Player");

			// Hurt Player Button
			if (ImGui::Button("Hurt Player")) {
				// You can change the damage value as per your need
				hurt_player(entity, 1);
			}

			// Kill Player Button
			if (ImGui::Button("Kill Player")) {
				kill_player(entity);
			}

			// Increase Ammo Button
			if (ImGui::Button("Increase Ammo")) {
				// Increase ammo by 5, modify this number as needed
				player.arrow_ammo += 5;
			}

			ImGui::Text("Current Health: %d", player.health);
			ImGui::Text("Current Ammo: %d", player.arrow_ammo);

			ImGui::End();
		}
	}


	void emplace_player(entt::entity entity, const Player& player) {
		_registry.emplace_or_replace<Player>(entity, player);
	}

	void remove_player(entt::entity entity) {
		_registry.remove<Player>(entity);
	}

	bool kill_player(entt::entity entity)
	{
		if (!_registry.all_of<Player>(entity)) return false;
		detach_camera(entity);
		destroy_at_end_of_frame(entity);
		audio::stop_all_in_bus();
		audio::play("event:/snd_player_die");
		audio::play("event:/mus_coffin_dance");
		ui::push_textbox_presets("player_die");
		ui::pop_textbox();
		ui::bindings::hud_player_health = 0;
		return true;
	}

	bool hurt_player(entt::entity entity, int health_to_remove)
	{
		if (health_to_remove <= 0) return false;
		if (!_registry.all_of<Player>(entity)) return false;
		Player& player = _registry.get<Player>(entity);
		if (player.health <= 0) return false; // Player is already dead
		if (player.hurt_timer.running()) return false; // Player is invulnerable
		player.hurt_timer.start();
		player.health = std::max(0, player.health - health_to_remove);
		if (player.health > 0) { // Player survived
			audio::play("event:/snd_player_hurt");
		} else { // Player died
			player.kill_timer.start();
		}
		return true;
	}
}
