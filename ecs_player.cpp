#include "stdafx.h"
#include "ecs_player.h"
#include "ecs_common.h"
#include "ecs_physics.h"
#include "ecs_physics_filters.h"
#include "ecs_arrow.h"
#include "ecs_tile.h"
#include "ecs_camera.h"
#include "ecs_pickups.h"
#include "ecs_bomb.h"
#include "ecs_vfx.h"
#include "physics_helpers.h"
#include "console.h"
#include "audio.h"
#include "window.h"
#include "ui_hud.h"
#include "ui_textbox.h"
#include "random.h"
#include "character.h"
#include "debug_draw.h"
#include "map_tilegrid.h"

using namespace std::literals::string_literals;

namespace ecs
{
	extern entt::registry _registry;

	const float _PLAYER_WALK_SPEED = 72.f;
	const float _PLAYER_RUN_SPEED = 136.f;
	const float _PLAYER_STEALTH_SPEED = 36.f;
	const float _PLAYER_ARROW_SPEED = 160.f;

	void process_event_players(const sf::Event& ev)
	{
		for (auto [entity, player] : _registry.view<Player>().each()) {
			if (ev.type == sf::Event::KeyPressed) {
				switch (ev.key.code) {
				case sf::Keyboard::Left:
					player.input.axis_x--;
					break;
				case sf::Keyboard::Right:
					player.input.axis_x++;
					break;
				case sf::Keyboard::Up:
					player.input.axis_y--;
					break;
				case sf::Keyboard::Down:
					player.input.axis_y++;
					break;
				case sf::Keyboard::LShift:
					player.input.run = true;
					break;
				case sf::Keyboard::LControl:
					player.input.stealth = true;
					break;
				case sf::Keyboard::C:
					player.input.interact = true;
					break;
				case sf::Keyboard::X:
					player.input.fire_arrow = true;
					break;
				case sf::Keyboard::Z:
					player.input.drop_bomb = true;
					break;
				case sf::Keyboard::Space:
					player.input.sword_attack = true;
					break;
				}
			} else if (ev.type == sf::Event::KeyReleased) {
				switch (ev.key.code) {
				case sf::Keyboard::Left:
					player.input.axis_x++;
					break;
				case sf::Keyboard::Right:
					player.input.axis_x--;
					break;
				case sf::Keyboard::Up:
					player.input.axis_y++;
					break;
				case sf::Keyboard::Down:
					player.input.axis_y--;
					break;
				case sf::Keyboard::LShift:
					player.input.run = false;
					break;
				case sf::Keyboard::LControl:
					player.input.stealth = false;
					break;
				}
			}
		}
	}

	// TODO: Put in a separate file with _player_attack below
	void _player_interact(const sf::Vector2f& position)
	{
		sf::Vector2f box_center = position;
		sf::Vector2f box_min = box_center - sf::Vector2f(6.f, 6.f);
		sf::Vector2f box_max = box_center + sf::Vector2f(6.f, 6.f);
		debug::draw_box(box_min, box_max, sf::Color::Red, 0.2f);
		std::unordered_set<std::string> audio_events_to_play; //So we don't play the same sound twice
		for (const BoxHit& hit : boxcast(box_min, box_max, ~CC_Player)) {
			std::string class_ = get_class(hit.entity);
			if (class_ == "slime") {
				destroy_at_end_of_frame(hit.entity);
			}
			else {
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

	void _player_attack(const sf::Vector2f& position)
	{
		sf::Vector2f box_center = position;
		sf::Vector2f box_min = box_center - sf::Vector2f(6.f, 6.f);
		sf::Vector2f box_max = box_center + sf::Vector2f(6.f, 6.f);
		debug::draw_box(box_min, box_max, sf::Color::Red, 0.2f);
		std::unordered_set<std::string> audio_events_to_play; //So we don't play the same sound twice
		for (const BoxHit& hit : boxcast(box_min, box_max, ~CC_Player)) {
			std::string class_ = get_class(hit.entity);
			sf::Vector2f hit_position = get_world_center(hit.body);
			if (class_ == "slime") {
				destroy_at_end_of_frame(hit.entity);
			} else if (class_ == "grass") {
				audio_events_to_play.insert("event:/snd_cut_grass");
				if (random::chance(0.2f))
					create_arrow_pickup(hit_position + sf::Vector2f(2.f, 2.f));
				else if (random::chance(0.2f))
					create_rupee_pickup(hit_position + sf::Vector2f(2.f, 2.f));
				destroy_at_end_of_frame(hit.entity);
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

	void update_players(float dt)
	{
		const bool player_accepts_input = (window::has_focus() && !console::has_focus());

		for (auto [player_entity, player, body, tile] : _registry.view<Player, b2Body*, Tile>().each()) {

			if (!player_accepts_input) {
				player.input = {};
			}

			// UPDATE TIMERS

			player.hurt_timer.update(dt);

			const sf::Vector2f position = get_world_center(body);
			const sf::Vector2f velocity = get_linear_velocity(body);
			sf::Vector2f new_velocity = velocity; // will be modified differently depending on the state

			audio::set_listener_position(position);

			// HANDLE TERRAIN

			const map::TerrainType terrain = map::get_terrain_type_at(position);
			audio::set_parameter_label("terrain", map::to_string(terrain));

			char dir = get_direction(player.look_dir);

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

			switch (player.state) {
			case PlayerState::Normal: {

				sf::Vector2f new_move_dir;
				float new_move_speed = 0.f;

				if (player.input.axis_x || player.input.axis_y) {
					new_move_dir = normalize({
						(float)player.input.axis_x,
						(float)player.input.axis_y });
					player.look_dir = new_move_dir;
					if (player.input.stealth) {
						new_move_speed = _PLAYER_STEALTH_SPEED;
					} else if (player.input.run) {
						new_move_speed = _PLAYER_RUN_SPEED;
					} else {
						new_move_speed = _PLAYER_WALK_SPEED;
					}
				}

				new_velocity = new_move_dir * new_move_speed;

				if (player.input.fire_arrow && player.arrows > 0) {
					tile.set_sprite("bow_shot_"s + dir);
					player.bow_shot_timer.start();
					tile.animation_loop = false; // Ensure that the animation will not loop
					player.state = PlayerState::Attacking;
				} else if (player.input.sword_attack) {
					tile.set_sprite("sword_attack_"s + dir);
					player.sword_attack_timer.start();
					tile.animation_loop = false;
					player.state = PlayerState::Attacking;
				} else if (player.input.drop_bomb && player.bombs > 0) {
					create_bomb(position + player.look_dir * 16.f);
					player.bombs--;
				} else if (new_move_speed >= _PLAYER_RUN_SPEED) {
					tile.set_sprite("run_"s + dir);
					tile.animation_speed = 1.2f;
					tile.animation_loop = true;
					if (tile.animation_looped_last_update())
						audio::play("event:/snd_footstep");
				} else if (new_move_speed >= _PLAYER_WALK_SPEED) {
					tile.set_sprite("walk_"s + dir);
					tile.animation_speed = 1.2f;
					tile.animation_loop = true;
					if (tile.animation_looped_last_update())
						audio::play("event:/snd_footstep");
				} else if (player.input.interact) {
					_player_interact(position + player.look_dir * 16.f);
				} else {
					tile.set_sprite("idle_"s + dir);
					tile.animation_loop = true;
				}
			} break;
			case PlayerState::Attacking: {
				new_velocity = sf::Vector2f(0.f, 0.f); // lock movement

				// Fire an arrow a while after the animation starts.
				if (player.bow_shot_timer.update(dt) && player.arrows > 0) {
					player.arrows--;
					create_arrow(
						position + player.look_dir * 16.f,
						player.look_dir * _PLAYER_ARROW_SPEED);
				}
				else if (player.sword_attack_timer.update(dt)) {
					_player_attack(position + player.look_dir * 16.f);
				}

				// When animation is done, we are done attacking.
				if (tile.animation_timer.finished()) {
					player.state = PlayerState::Normal;
				}
			} break;
			case PlayerState::Dying: {
				new_velocity = sf::Vector2f(0.f, 0.f); // lock movement

				tile.animation_loop = false;

				if (tile.animation_timer.finished()) {
					tile.set_sprite("dead_"s + dir);
					kill_player(player_entity);
					player.state = PlayerState::Dead;
					break;
				}
				if (dir == 'd') dir = 'u';
				tile.set_sprite("dying_"s + dir);
			} break;
			case PlayerState::Dead: {
				new_velocity = sf::Vector2f(0.f, 0.f); // lock movement
			} break;
			}

			set_linear_velocity(body, new_velocity);

			// UPDATE TILE COLOR
			{
				sf::Color color = sf::Color::White;
				if (player.hurt_timer.running()) {
					float fraction = fmod(player.hurt_timer.get_time(), 0.15f) / 0.15f;
					color.a = (sf::Uint8)(255 * fraction);
				}
				tile.color = color;
			}

			// UPDATE HUD

			ui::bindings::hud_player_health = player.health;
			ui::bindings::hud_arrow_ammo = player.arrows;
			ui::bindings::hud_bomb_ammo = player.bombs;
			ui::bindings::hud_rupee_amount = player.rupees;

			// CLEAR INPUT

			player.input.interact = false;
			player.input.fire_arrow = false;
			player.input.drop_bomb = false;
			player.input.sword_attack = false;
		}
	}

	void debug_draw_players()
	{
		ImGui::Begin("Player");

		for (auto [entity, player, body] : _registry.view<Player, b2Body*>().each()) {
			sf::Vector2f position = get_world_center(body);
			ImGui::Text("Position: %.1f, %.1f", position.x, position.y);
			ImGui::Text("Terrain: %s", map::to_string(map::get_terrain_type_at(position)).c_str());
		}

		for (auto [entity, player] : _registry.view<Player>().each()) {

			ImGui::Text("State: %s", magic_enum::enum_name(player.state).data());
			ImGui::Text("Health: %d", player.health);
			ImGui::Text("Arrows: %d", player.arrows);
			ImGui::Text("Rupees: %d", player.rupees);
			
			if (ImGui::Button("Hurt"))
				hurt_player(entity, 1);
			ImGui::SameLine();
			if (ImGui::Button("Kill"))
				kill_player(entity);

			if (ImGui::Button("Give 5 Arrows"))
				player.arrows += 5;
			ImGui::SameLine();
			if (ImGui::Button("Give 5 Rupees"))
				player.rupees += 5;
		}

		for (auto [entity, player, tile] : _registry.view<Player, Tile>().each()) {
			if (ImGui::Button("Randomize")) {
				Character character{};
				character.randomize();
				tile.texture = character.bake_texture();
			}
		}

		ImGui::End();
	}

	void emplace_player(entt::entity entity, const Player& player) {
		_registry.emplace_or_replace<Player>(entity, player);
	}

	bool remove_player(entt::entity entity) {
		return _registry.remove<Player>(entity);
	}

	bool kill_player(entt::entity entity)
	{
		if (!_registry.all_of<Player>(entity)) return false;
		detach_camera(entity);
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
		player.health = std::max(0, player.health - health_to_remove);
		add_trauma_to_active_camera(0.8f);
		if (player.health > 0) { // Player survived
			audio::play("event:/snd_player_hurt");
			player.hurt_timer.start();
		} else { // Player died
			player.state = PlayerState::Dying;
		}
		return true;
	}
}
