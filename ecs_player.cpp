#include "stdafx.h"
#include "ecs_player.h"
#include "ecs_common.h"
#include "ecs_physics.h"
#include "ecs_physics_filters.h"
#include "ecs_arrow.h"
#include "ecs_tile.h"
#include "ecs_camera.h"
#include "ecs_bomb.h"
#include "ecs_damage.h"
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

namespace ecs
{
	const float _PLAYER_WALK_SPEED = 60.f;
	const float _PLAYER_RUN_SPEED = 136.f;
	const float _PLAYER_STEALTH_SPEED = 36.f;
	const float _PLAYER_ARROW_SPEED = 160.f;

	extern entt::registry _registry;

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
					player.input.shoot_bow = true;
					break;
				case sf::Keyboard::Z:
					player.input.drop_bomb = true;
					break;
				case sf::Keyboard::Space:
					player.input.use_sword = true;
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

	// TODO: Put in a separate unit
	void _player_interact(const sf::Vector2f& position)
	{
		sf::Vector2f box_center = position;
		sf::Vector2f box_min = box_center - sf::Vector2f(6.f, 6.f);
		sf::Vector2f box_max = box_center + sf::Vector2f(6.f, 6.f);
		debug::draw_box(box_min, box_max, sf::Color::Cyan, 0.2f);
		for (const OverlapHit& hit : overlap_box(box_min, box_max, ~CC_Player)) {
			std::string class_ = get_class(hit.entity);
			std::string string;
			if (get_string(hit.entity, "textbox", string)) {
				ui::push_textbox_presets(string);
				ui::pop_textbox();
			}
			if (get_string(hit.entity, "sound", string))
				audio::play(string);
		}
	}

	void _player_attack(entt::entity entity, const sf::Vector2f& position)
	{
		sf::Vector2f box_min = position - sf::Vector2f(6.f, 6.f);
		sf::Vector2f box_max = position + sf::Vector2f(6.f, 6.f);
		apply_damage_in_box({ DamageType::Melee, 1, entity }, box_min, box_max, ~CC_Player);
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

			// GET PHYSICS STATE

			const sf::Vector2f position = get_world_center(body);
			const sf::Vector2f velocity = get_linear_velocity(body);
			sf::Vector2f new_velocity; // will be modified differently depending on the state

			std::string new_held_item_tileset_name;
			std::string new_held_item_tile_class = "n";
			sf::Vector2f new_held_item_position;
			bool new_held_item_visible = false;

			// UPDATE AUDIO

			audio::set_listener_position(position);
			audio::set_parameter_label("terrain", map::to_string(map::get_terrain_type_at(position)));
			if (tile.get_flag(TF_FRAME_CHANGED) && tile.get_properties().has("step")) {
				audio::play("event:/snd_footstep");
			}

			char dir = get_direction(player.look_dir);

			switch (dir) {
			case 'r':
				tile.set_flag(TF_FLIP_X, false);
				tile.set_flag(TF_FLIP_X_ON_LOOP, false);
				break;
			case 'l':
				dir = 'r';
				tile.set_flag(TF_FLIP_X, true);
				tile.set_flag(TF_FLIP_X_ON_LOOP, false);
				break;
			case 'u':
			case 'd':
				tile.set_flag(TF_FLIP_X_ON_LOOP, true);
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

				if (player.input.use_sword) {
					tile.set_tile("sword_attack_"s + dir);
					tile.set_flag(TF_LOOP, false);
					player.state = PlayerState::UsingSword;
				} else if (player.input.shoot_bow && player.arrows > 0) {
					tile.set_tile("bow_shot_"s + dir);
					tile.set_flag(TF_LOOP, false);
					player.state = PlayerState::UsingBow;
				} else if (player.input.drop_bomb && player.bombs > 0) {
					create_bomb(position + player.look_dir * 16.f);
					player.bombs--;
				} else if (new_move_speed >= _PLAYER_RUN_SPEED) {
					tile.set_tile("run_"s + dir);
					tile.set_flag(TF_LOOP, true);
				} else if (new_move_speed >= _PLAYER_WALK_SPEED) {
					tile.set_tile("walk_"s + dir);
					tile.set_flag(TF_LOOP, true);
				} else if (player.input.interact) {
					_player_interact(position + player.look_dir * 16.f);
				} else {
					tile.set_tile("idle_"s + dir);
					tile.set_flag(TF_LOOP, true);
				}
			} break;
			case PlayerState::UsingSword: {
				new_held_item_tileset_name = "sword";
				new_held_item_tile_class = "n";
				new_held_item_position = position + player.look_dir * 16.f;
				new_held_item_visible = true;
				if (tile.get_flag(TF_FRAME_CHANGED) && tile.get_properties().has("strike")) {
					_player_attack(player_entity, position + player.look_dir * 16.f);
				}
				if (tile.animation_timer.finished()) {
					player.state = PlayerState::Normal;
				}
			} break;
			case PlayerState::UsingBow: {
				new_held_item_tileset_name = "bow_01";
				new_held_item_position = position + player.look_dir * 16.f;
				new_held_item_visible = true;
				if (player.arrows > 0 && tile.get_flag(TF_FRAME_CHANGED) && tile.get_properties().has("shoot")) {
					player.arrows--;
					create_arrow(position + player.look_dir * 16.f, player.look_dir * _PLAYER_ARROW_SPEED);
				}
				if (tile.animation_timer.finished()) {
					player.state = PlayerState::Normal;
				}
			} break;
			case PlayerState::Dying: {
				if (dir == 'd') dir = 'u';
				tile.set_tile("dying_"s + dir);
				tile.set_flag(TF_LOOP, false);
				if (tile.animation_timer.finished()) {
					tile.set_tile("dead_"s + dir);
					kill_player(player_entity);
					player.state = PlayerState::Dead;
				}
			} break;
			case PlayerState::Dead: {
				// Do nothing, u r ded
			} break;
			}

			set_linear_velocity(body, new_velocity);

			// UPDATE HELD ITEM

			if (Tile* held_item_tile = try_get_tile(player.held_item)) {
				held_item_tile->set_flag(TF_VISIBLE, new_held_item_visible);
				held_item_tile->set_tile(new_held_item_tile_class, new_held_item_tileset_name);
				held_item_tile->position = new_held_item_position;
				held_item_tile->pivot = { 16.f, 28.f };
			}

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
			player.input.use_sword = false;
			player.input.shoot_bow = false;
			player.input.drop_bomb = false;
		}
	}

	void debug_draw_players()
	{
		const size_t player_count = _registry.view<Player>().size();

		ImGui::Begin("Players"); 
		for (auto [entity, player, body, tile] : _registry.view<Player, b2Body*, Tile>().each()) {
			const std::string tree_node_label = "Player " + std::to_string((uint32_t)entity);

			// For convenience, if there's just one player, open debug menu immediately. 
			ImGui::SetNextItemOpen(player_count == 1, ImGuiCond_Appearing);
			if (!ImGui::TreeNode(tree_node_label.c_str())) continue;

			sf::Vector2f position = get_world_center(body);
			ImGui::Text("Position: %.1f, %.1f", position.x, position.y);
			ImGui::Text("Terrain: %s", map::to_string(map::get_terrain_type_at(position)).c_str());
			ImGui::Text("State: %s", magic_enum::enum_name(player.state).data());
			ImGui::Text("Health: %d", player.health);
			ImGui::Text("Arrows: %d", player.arrows);
			ImGui::Text("Rupees: %d", player.rupees);
			
			if (ImGui::Button("Apply 1 Damage"))
				apply_damage_to_player(entity, { DamageType::Default, 1 });
			if (ImGui::Button("Kill"))
				kill_player(entity);

			if (ImGui::Button("Give 5 Arrows"))
				player.arrows += 5;
			if (ImGui::Button("Give 5 Bombs"))
				player.bombs += 5;
			if (ImGui::Button("Give 5 Rupees"))
				player.rupees += 5;

			if (ImGui::Button("Randomize Appearance")) {
				Character character{};
				character.randomize();
				tile.texture = character.bake_texture();
			}

			ImGui::Spacing(); //did this even do anything??
			ImGui::TreePop();
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

	bool apply_damage_to_player(entt::entity entity, const Damage& damage)
	{
		if (damage.amount <= 0) return false;
		if (!_registry.all_of<Player>(entity)) return false;
		Player& player = _registry.get<Player>(entity);
		if (player.health <= 0) return false; // Player is already dead
		if (player.hurt_timer.running()) return false; // Player is invulnerable
		player.health = std::max(0, player.health - damage.amount);
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
