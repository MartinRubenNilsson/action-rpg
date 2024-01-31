#include "stdafx.h"
#include "ecs_player.h"
#include "ecs.h"
#include "ecs_tiled.h"
#include "ecs_physics.h"
#include "ecs_physics_filters.h"
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
#include "ecs_projectile.h"
#include "ecs_pickups.h"
#include "random.h"
#include "tiled.h"
#ifdef _DEBUG
#include <imgui.h>
#endif

namespace ecs
{
	extern entt::registry _registry;

	const float PLAYER_WALK_SPEED = 72.f;
	const float PLAYER_RUN_SPEED = 136.f;
	const float PROJECTILE_SPEED = 160.f;
	const float PLAYER_STEALTH_SPEED = 36.f;

	void process_event_player(const sf::Event& ev)
	{
		for (auto [entity, player] : _registry.view<Player>().each()) {
			if (ev.type == sf::Event::KeyPressed) {
				if (ev.key.code == sf::Keyboard::C)
					player.input.interact = true;
				if (ev.key.code == sf::Keyboard::Z)
					player.input.projectile_attack = true;
			}
		}
	}

	void fire_projectile(const sf::Vector2f& position, const sf::Vector2f& direction, int damage) {
		// Calculate the offset position
		float offsetDistance = 1.0f; // Adjust this value as needed
		sf::Vector2f offset = normalize(direction) * offsetDistance;
		sf::Vector2f fire_position = position + offset;

		entt::entity projectile_entity = _registry.create();
		set_class(projectile_entity, "arrow");

		// Setup the Projectile component
		Projectile projectile = { damage, 5.0f };
		_registry.emplace<Projectile>(projectile_entity, projectile);

		// Create a physics body for the projectile
		b2BodyDef bodyDef;
		bodyDef.type = b2_dynamicBody;
		bodyDef.position.Set(fire_position.x, fire_position.y); // Set the offset position here
		b2Body* body = emplace_body(projectile_entity, bodyDef);
		b2CircleShape shape;
		shape.m_p.x = 0;
		shape.m_p.y = 0;
		shape.m_radius = 6.f;
		b2FixtureDef fixture_def;
		fixture_def.shape = &shape;
		fixture_def.density = 1.f;
		fixture_def.filter = get_filter_for_class("arrow");
		body->CreateFixture(&fixture_def);

		// Set the velocity of the projectile
		sf::Vector2f projectile_velocity = normalize(direction) * PROJECTILE_SPEED;
		body->SetLinearVelocity(b2Vec2(projectile_velocity.x, projectile_velocity.y));

		//TODO Play arrow firing sound here
		audio::play("event:/snd_fire_arrow");

		// Add additional components like Tile here if necessary
		if (Tile* tile = emplace_tile(projectile_entity, "items1", "arrow")) {
			tile->pivot = sf::Vector2f(6.f, 6.f);
		}
	}

	void update_player(float dt)
	{
		for (auto [player_entity, player, body, tile] : _registry.view<Player, b2Body*, Tile>().each()) {

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

			// UPDATE PHYSICS

			sf::Vector2f player_position = get_world_center(body);
			sf::Vector2f player_velocity = get_linear_velocity(body);

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
						if (get_string(hit.entity, "textbox", string))
							ui::open_textbox_preset(string);
						if (get_string(hit.entity, "sound", string))
							audio_events_to_play.insert(string);
					}
				}
				for (const std::string& audio_event : audio_events_to_play)
					audio::play(audio_event);
			}

			// HANDLE PROJECTILE ATTACK

			if (player.input.projectile_attack && player.arrow_ammo > 0) {
				fire_projectile(player_position, player.facing_direction, 1);
				player.arrow_ammo--;
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
					break;
				case 'l':
					tile.flip_x = true;
					dir = 'r';
					break;
				case 'u':
				case 'd':
					tile.flip_x = (tile.get_animation_loop_count() % 2);
					break;
				}

				if (movement_speed >= PLAYER_RUN_SPEED) {
					tile.set_class("run_"s + dir);
					tile.animation_speed = 1.2f;
				} else if (movement_speed >= PLAYER_WALK_SPEED) {
					tile.set_class("walk_"s + dir);
					tile.animation_speed = 1.2f;
				} else {
					tile.set_class("idle_"s + dir);
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

			ui::hud_player_health = player.health;

			// CLEAR INPUT

			player.input = {};
		}
	}

	void debug_player()
	{
#ifdef _DEBUG
		for (auto [entity, player] : _registry.view<Player>().each()) {
			ImGui::Begin("Player");
			// TODO
			ImGui::End();
		}
#endif
	}

	void emplace_player(entt::entity entity, const Player& player) {
		_registry.emplace<Player>(entity, player);
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
		ui::open_textbox_preset("player_die_01");
		ui::hud_player_health = 0;
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
