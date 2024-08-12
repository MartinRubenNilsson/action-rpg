#include "stdafx.h"
#include "ecs_player.h"
#include "ecs_common.h"
#include "ecs_physics.h"
#include "ecs_physics_filters.h"
#include "ecs_arrow.h"
#include "ecs_sprite.h"
#include "ecs_tile.h"
#include "ecs_camera.h"
#include "ecs_bomb.h"
#include "ecs_damage.h"
#include "ecs_interact.h"
#include "ecs_character.h"
#include "console.h"
#include "audio.h"
#include "window.h"
#include "window_events.h"
#include "ui_hud.h"
#include "ui_textbox.h"
#include "ui_bindings.h"
#include "random.h"
#include "map_tilegrid.h"
#include "postprocessing.h"
#include "shapes.h"

namespace ecs
{
	enum PLAYER_TILE : unsigned int
	{
		PLAYER_TILE_IDLE_DOWN = 0,
		PLAYER_TILE_IDLE_UP = 16,
		PLAYER_TILE_IDLE_RIGHT = 32,

		PLAYER_TILE_PUSH_DOWN = 8,
		PLAYER_TILE_PUSH_UP = 24,
		PLAYER_TILE_PUSH_RIGHT = 40,

		PLAYER_TILE_WALK_DOWN = 48,
		PLAYER_TILE_WALK_UP = 52,
		PLAYER_TILE_WALK_RIGHT = 64,

		PLAYER_TILE_RUN_DOWN = 51,
		PLAYER_TILE_RUN_UP = 55,
		PLAYER_TILE_RUN_RIGHT = 70,

		PLAYER_TILE_FOREHAND_STRIKE_DOWN = 132,
		PLAYER_TILE_FOREHAND_STRIKE_UP = 148,
		PLAYER_TILE_FOREHAND_STRIKE_RIGHT = 164,

		PLAYER_TILE_BOW_SHOT_DOWN = 133,
		PLAYER_TILE_BOW_SHOT_UP = 149,
		PLAYER_TILE_BOW_SHOT_RIGHT = 165,

		PLAYER_TILE_DYING_RIGHT_DOWN = 178,
		PLAYER_TILE_DYING_RIGHT_UP = 181,

		PLAYER_TILE_DEAD_RIGHT_DOWN = 180,
		PLAYER_TILE_DEAD_RIGHT_UP = 183,
	};

	const float _PLAYER_WALK_SPEED = 60.f;
	const float _PLAYER_RUN_SPEED = 136.f;
	const float _PLAYER_STEALTH_SPEED = 36.f;
	const float _PLAYER_ARROW_SPEED = 160.f;

	extern entt::registry _registry;
	unsigned int _input_flags_to_enable = 0;
	unsigned int _input_flags_to_disable = 0;

	void process_window_event_for_players(const window::Event& ev)
	{
		if (ev.type == window::EventType::KeyPress) {
			switch (ev.key.code) {
			case window::Key::Left:
				_input_flags_to_enable |= INPUT_LEFT;
				break;
			case window::Key::Right:
				_input_flags_to_enable |= INPUT_RIGHT;
				break;
			case window::Key::Up:
				_input_flags_to_enable |= INPUT_UP;
				break;
			case window::Key::Down:
				_input_flags_to_enable |= INPUT_DOWN;
				break;
			case window::Key::LShift:
				_input_flags_to_enable |= INPUT_RUN;
				break;
			case window::Key::LControl:
				_input_flags_to_enable |= INPUT_STEALTH;
				break;
			case window::Key::C:
				_input_flags_to_enable |= INPUT_INTERACT;
				break;
			case window::Key::X:
				_input_flags_to_enable |= INPUT_SHOOT_BOW;
				break;
			case window::Key::Z:
				_input_flags_to_enable |= INPUT_DROP_BOMB;
				break;
			case window::Key::Space:
				_input_flags_to_enable |= INPUT_SWING_SWORD;
				break;
			}
		} else if (ev.type == window::EventType::KeyRelease) {
			switch (ev.key.code) {
			case window::Key::Left:
				_input_flags_to_disable |= INPUT_LEFT;
				break;
			case window::Key::Right:
				_input_flags_to_disable |= INPUT_RIGHT;
				break;
			case window::Key::Up:
				_input_flags_to_disable |= INPUT_UP;
				break;
			case window::Key::Down:
				_input_flags_to_disable |= INPUT_DOWN;
				break;
			case window::Key::LShift:
				_input_flags_to_disable |= INPUT_RUN;
				break;
			case window::Key::LControl:
				_input_flags_to_disable |= INPUT_STEALTH;
				break;
			}
		}
	}

	void _player_attack(entt::entity entity, const Vector2f& position)
	{
		Vector2f box_min = position - Vector2f(6.f, 6.f);
		Vector2f box_max = position + Vector2f(6.f, 6.f);
		apply_damage_in_box({ DamageType::Melee, 1, entity }, box_min, box_max, ~CC_Player);
	}

	void update_players(float dt)
	{
		const bool player_accepts_input = (dt > 0.f && window::has_focus() && !console::has_focus());

		for (auto [player_entity, player, body, sprite, tile, animation] :
			_registry.view<Player, b2Body*, sprites::Sprite, Tile, TileAnimation>().each()) {

			if (player_accepts_input) {
				player.input_flags |= _input_flags_to_enable;
				player.input_flags &= ~_input_flags_to_disable;
			} else {
				player.input_flags = 0;
			}

			// UPDATE TIMERS

			player.hurt_timer.update(dt);

			// GET PHYSICS STATE

			const Vector2f position = body->GetPosition();
			const Vector2f velocity = body->GetLinearVelocity();
			Vector2f new_velocity; // will be modified differently depending on the state

			enum class HeldItemType
			{
				None,
				Sword,
				Bow,
			};

			HeldItemType held_item_type = HeldItemType::None;

			// UPDATE AUDIO

			audio::set_listener_position(position);
			audio::set_parameter_label("terrain", map::to_string(map::get_terrain_type_at(position)));
			if (animation.frame_changed && animation.frame % 3 == 0) {
				// Take a step every 3 frames
				audio::create_event({ .path = "event:/snd_footstep" });
			}

			// UPDATE POST-PROCESSING
			postprocessing::set_darkness_center(position);

			char dir = get_direction(player.look_dir);
			char tile_dir = dir;

			switch (dir) {
			case 'r':
				sprite.flags &= ~sprites::SPRITE_FLIP_HORIZONTALLY;
				tile.set_flag(TILE_FLIP_X_ON_LOOP, false);
				break;
			case 'l':
				tile_dir = 'r';
				sprite.flags |= sprites::SPRITE_FLIP_HORIZONTALLY;
				tile.set_flag(TILE_FLIP_X_ON_LOOP, false);
				break;
			case 'u':
			case 'd':
				tile.set_flag(TILE_FLIP_X_ON_LOOP, true);
				break;
			}

			switch (player.state) {
			case PlayerState::Normal: {

				Vector2f new_move_dir;
				float new_move_speed = 0.f;

				if (player.input_flags & INPUT_LEFT)  new_move_dir.x--;
				if (player.input_flags & INPUT_RIGHT) new_move_dir.x++;
				if (player.input_flags & INPUT_UP)    new_move_dir.y--;
				if (player.input_flags & INPUT_DOWN)  new_move_dir.y++;

				if (!is_zero(new_move_dir)) {
					player.look_dir = new_move_dir;
					new_move_dir = normalize(new_move_dir);
					if (player.input_flags & INPUT_STEALTH) {
						new_move_speed = _PLAYER_STEALTH_SPEED;
					} else if (player.contacting_pushable_block) {
						new_move_speed = _PLAYER_WALK_SPEED;
					} else if (player.input_flags & INPUT_RUN) {
						new_move_speed = _PLAYER_RUN_SPEED;
					} else {
						new_move_speed = _PLAYER_WALK_SPEED;
					}
				}

				new_velocity = new_move_dir * new_move_speed;

				if (player.input_flags & INPUT_SWING_SWORD) {

					switch (dir) {
					case 'l':
					case 'r': tile.set_tile(PLAYER_TILE_FOREHAND_STRIKE_RIGHT); break;
					case 'u': tile.set_tile(PLAYER_TILE_FOREHAND_STRIKE_UP); break;
					case 'd': tile.set_tile(PLAYER_TILE_FOREHAND_STRIKE_DOWN); break;
					}

					audio::create_event({ .path = "event:/snd_sword_attack" });
					animation.progress = 0.f;
					animation.loop = false;

					player.state = PlayerState::SwingingSword;

				} else if (player.input_flags & INPUT_SHOOT_BOW && player.arrows > 0) {

					switch (dir) {
					case 'l':
					case 'r': tile.set_tile(PLAYER_TILE_BOW_SHOT_RIGHT); break;
					case 'u': tile.set_tile(PLAYER_TILE_BOW_SHOT_UP); break;
					case 'd': tile.set_tile(PLAYER_TILE_BOW_SHOT_DOWN); break;
					}

					animation.progress = 0.f;
					animation.loop = false;

					player.state = PlayerState::ShootingBow;

				} else if (player.input_flags & INPUT_DROP_BOMB && player.bombs > 0) {

					create_bomb(position + player.look_dir * 16.f);
					player.bombs--;

				} else if (player.contacting_pushable_block && !is_zero(new_velocity)) {

					switch (dir) {
					case 'l':
					case 'r': tile.set_tile(PLAYER_TILE_PUSH_RIGHT); break;
					case 'u': tile.set_tile(PLAYER_TILE_PUSH_UP); break;
					case 'd': tile.set_tile(PLAYER_TILE_PUSH_DOWN); break;
					}
					tile.set_flag(TILE_FLIP_X_ON_LOOP, false);

				} else if (new_move_speed >= _PLAYER_RUN_SPEED) {

					switch (dir) {
					case 'l':
					case 'r': tile.set_tile(PLAYER_TILE_RUN_RIGHT); break;
					case 'u': tile.set_tile(PLAYER_TILE_RUN_UP); break;
					case 'd': tile.set_tile(PLAYER_TILE_RUN_DOWN); break;
					}
					animation.loop = true;

				} else if (new_move_speed >= _PLAYER_WALK_SPEED) {

					switch (dir) {
					case 'l': 
					case 'r': tile.set_tile(PLAYER_TILE_WALK_RIGHT); break;
					case 'u': tile.set_tile(PLAYER_TILE_WALK_UP); break;
					case 'd': tile.set_tile(PLAYER_TILE_WALK_DOWN); break;
					}
					animation.loop = true;

				} else {

					switch (dir) {
					case 'l':
					case 'r': tile.set_tile(PLAYER_TILE_IDLE_RIGHT); break;
					case 'u': tile.set_tile(PLAYER_TILE_IDLE_UP); break;
					case 'd': tile.set_tile(PLAYER_TILE_IDLE_DOWN); break;
					}
					animation.loop = true;

				}

				if (player.input_flags & INPUT_INTERACT) {
					const Vector2f box_center = position + player.look_dir * 16.f;
					const Vector2f box_min = box_center - Vector2f(6.f, 6.f);
					const Vector2f box_max = box_center + Vector2f(6.f, 6.f);
					interact_in_box(box_min, box_max);
				}

			} break;
			case PlayerState::SwingingSword: {
				held_item_type = HeldItemType::Sword;
				if (tile_dir != 'r') {
					sprite.flags &= ~sprites::SPRITE_FLIP_HORIZONTALLY;
				}
				if (animation.frame_changed && animation.frame == 1) {
					_player_attack(player_entity, position + player.look_dir * 16.f);
				}
				if (animation.progress == 1.f) {
					player.state = PlayerState::Normal;
				}
			} break;
			case PlayerState::ShootingBow: {
				held_item_type = HeldItemType::Bow;
				if (tile_dir != 'r') {
					sprite.flags &= ~sprites::SPRITE_FLIP_HORIZONTALLY;
				}
				if (player.arrows > 0 && animation.frame_changed && animation.frame == 2) {
					player.arrows--;
					create_arrow(position + player.look_dir * 16.f, player.look_dir * _PLAYER_ARROW_SPEED);
				}
				if (animation.progress == 1.f) {
					player.state = PlayerState::Normal;
				}
			} break;
			case PlayerState::Dying: {

				switch (dir) {
				case 'l':
				case 'r': tile.set_tile(PLAYER_TILE_DYING_RIGHT_DOWN); break;
				case 'u': tile.set_tile(PLAYER_TILE_DYING_RIGHT_UP); break;
				case 'd': tile.set_tile(PLAYER_TILE_DYING_RIGHT_DOWN); break;
				}

				animation.progress = 0.f;
				animation.loop = false;

				if (animation.progress == 1.f) {

					switch (dir) {
					case 'l':
					case 'r': tile.set_tile(PLAYER_TILE_DEAD_RIGHT_DOWN); break;
					case 'u': tile.set_tile(PLAYER_TILE_DEAD_RIGHT_UP); break;
					case 'd': tile.set_tile(PLAYER_TILE_DEAD_RIGHT_DOWN); break;
					}

					kill_player(player_entity);
					player.state = PlayerState::Dead;
				}

			} break;
			case PlayerState::Dead: {
				// Do nothing, u r ded
			} break;
			}

			body->SetLinearVelocity(new_velocity);

			// UPDATE HELD ITEM GRAPHICS

#if 0
			if (Tile* held_item_tile = get_tile(player.held_item)) {
				held_item_tile->set_flag(TILE_VISIBLE, held_item_type != HeldItemType::None);
				Vector2f player_tile_sorting_pos = tile.position - tile.pivot + tile.sorting_pivot;
				switch (held_item_type) {
				case HeldItemType::Sword: {
					held_item_tile->set_tileset("sword");
					held_item_tile->set_tile(3);
					held_item_tile->position = position;
					held_item_tile->position.y -= 10.f;
					held_item_tile->pivot = { 16.f, 16.f };
					held_item_tile->sorting_pivot =
						player_tile_sorting_pos - held_item_tile->position + held_item_tile->pivot;
					switch (dir) {
					case 'u':
						held_item_tile->set_rotation((int)animation.frame - 1);
						switch (animation.frame) {
						case 0: held_item_tile->position += { -18.f, -3.f }; break;
						case 1: held_item_tile->position += { 0.f, -21.f }; break;
						case 2: held_item_tile->position += { 20.f, -1.f }; break;
						}
						break;
					case 'r':
						held_item_tile->set_rotation(2 - (int)animation.frame);
						switch (animation.frame) {
						case 0: held_item_tile->position += { -0.f, 18.f }; break;
						case 1: held_item_tile->position += { 28.f, 0.f }; break;
						case 2: held_item_tile->position += { 1.f, -18.f }; break;
						}
						break;
					case 'd':
						held_item_tile->set_rotation((int)animation.frame + 1);
						switch (animation.frame) {
						case 0: held_item_tile->position += { 18.f, 2.f }; break;
						case 1: held_item_tile->position += { 0.f, 20.f }; break;
						case 2: held_item_tile->position += { -18.f, -2.f }; break;
						}
						break;
					case 'l':
						held_item_tile->set_rotation((int)animation.frame + 2);
						switch (animation.frame) {
						case 0: held_item_tile->position += { 2.f, 18.f }; break;
						case 1: held_item_tile->position += { -28.f, 0.f }; break;
						case 2: held_item_tile->position += { -2.f, -18.f }; break;
						}
						break;
					}
				} break;
				case HeldItemType::Bow: {
					held_item_tile->set_tileset("bow_01");
					held_item_tile->set_tile(animation.frame);
					held_item_tile->position = position;
					held_item_tile->position.y -= 13.f;
					held_item_tile->pivot = { 16.f, 16.f };
					held_item_tile->sorting_pivot =
						player_tile_sorting_pos - held_item_tile->position + held_item_tile->pivot;
					switch (dir) {
					case 'r':
						held_item_tile->set_rotation(0);
						held_item_tile->position.x += 16.f;
						held_item_tile->position.y += 2.f;
						held_item_tile->sorting_pivot.y -= 2.f;
						held_item_tile->sorting_pivot.y += 1.f;
						break;
					case 'd':
						held_item_tile->set_rotation(1);
						held_item_tile->position.y += 11.f;
						held_item_tile->sorting_pivot.y -= 11.f;
						held_item_tile->sorting_pivot.y += 1.f;
						break;
					case 'l':
						held_item_tile->set_rotation(2);
						held_item_tile->position.x -= 16.f;
						held_item_tile->position.y += 3.f;
						held_item_tile->sorting_pivot.y -= 3.f;
						held_item_tile->sorting_pivot.y += 1.f;
						break;
					case 'u':
						held_item_tile->set_rotation(3);
						held_item_tile->position.y -= 11.f;
						held_item_tile->sorting_pivot.y += 11.f;
						held_item_tile->sorting_pivot.y -= 1.f;
						break;
					}
				} break;
				} 
			}
#endif

			// UPDATE SPRITE COLOR

			if (player.hurt_timer.running()) {
				constexpr float BLINK_PERIOD = 0.15f;
				float fraction = fmod(player.hurt_timer.get_time(), BLINK_PERIOD) / BLINK_PERIOD;
				sprite.color.a = (unsigned char)(255 * fraction);
			} else {
				sprite.color.a = 255;
			}

			// UPDATE HUD

			ui::bindings::hud_player_health = player.health;
			ui::bindings::hud_arrow_ammo = player.arrows;
			ui::bindings::hud_bomb_ammo = player.bombs;
			ui::bindings::hud_rupee_amount = player.rupees;

			// CLEAR ONE-SHOT INPUT FLAGS

			player.input_flags &= ~INPUT_INTERACT;
			player.input_flags &= ~INPUT_SWING_SWORD;
			player.input_flags &= ~INPUT_SHOOT_BOW;
			player.input_flags &= ~INPUT_DROP_BOMB;
		}

		_input_flags_to_enable = 0;
		_input_flags_to_disable = 0;
	}

	void show_player_debug_window()
	{
#ifdef DEBUG_IMGUI
		const size_t player_count = _registry.view<Player>().size();

		ImGui::Begin("Players"); 
		for (auto [entity, player, body, tile, character] : _registry.view<Player, b2Body*, Tile, Character>().each()) {
			const std::string tree_node_label = "Player " + std::to_string((uint32_t)entity);

			// For convenience, if there's just one player, open debug menu immediately. 
			ImGui::SetNextItemOpen(player_count == 1, ImGuiCond_Appearing);
			if (!ImGui::TreeNode(tree_node_label.c_str())) continue;

			Vector2f position = body->GetPosition();
			ImGui::Text("Position: %.1f, %.1f", position.x, position.y);
			ImGui::Text("Terrain: %s", map::to_string(map::get_terrain_type_at(position)).c_str());
			ImGui::Text("State: %s", magic_enum::enum_name(player.state).data());
			ImGui::Text("Health: %d", player.health);
			ImGui::Text("Arrows: %d", player.arrows);
			ImGui::Text("Rupees: %d", player.rupees);
			
			if (ImGui::Button("Apply 1 Damage"))
				apply_damage_to_player(entity, { DamageType::Default, 1 });
			if (ImGui::Button("Kill"))
				apply_damage_to_player(entity, { DamageType::Default, 999 });

			if (ImGui::Button("Give 5 Arrows"))
				player.arrows += 5;
			if (ImGui::Button("Give 5 Bombs"))
				player.bombs += 5;
			if (ImGui::Button("Give 5 Rupees"))
				player.rupees += 5;

#if 0
			if (ImGui::Button("Randomize Appearance")) {
				randomize_character(character);
				regenerate_character_texture(character);
				tile.texture = character.texture;
			}
#endif

			ImGui::Spacing(); //did this even do anything??
			ImGui::TreePop();
		}

		ImGui::End();
#endif
	}

	entt::entity find_player_entity()
	{
		return _registry.view<Player>().front();
	}

	Player& emplace_player(entt::entity entity, const Player& player)
	{
		return _registry.emplace_or_replace<Player>(entity, player);
	}

	Player* get_player(entt::entity entity)
	{
		return _registry.try_get<Player>(entity);
	}

	bool remove_player(entt::entity entity)
	{
		return _registry.remove<Player>(entity);
	}

	bool has_player(entt::entity entity)
	{
		return _registry.all_of<Player>(entity);
	}

	bool kill_player(entt::entity entity)
	{
		if (!_registry.all_of<Player>(entity)) return false;
		detach_camera(entity);
		audio::stop_all_in_bus();
		audio::create_event({ .path = "event:/snd_player_die" });
		audio::create_event({ .path = "event:/mus_coffin_dance" });
		ui::open_or_enqueue_textbox_presets("player/die");
		ui::bindings::hud_player_health = 0;
		return true;
	}

	void on_player_begin_contact_pushable_block(entt::entity player_entity)
	{
		Player* player = get_player(player_entity);
		if (!player) return;
		player->contacting_pushable_block = true;
		audio::stop_event(player->stone_sliding_sound); // Stop any previously playing sound
		player->stone_sliding_sound = audio::create_event({ .path = "event:/props/stone_slide" });
	}

	void on_player_end_contact_pushable_block(entt::entity player_entity)
	{
		Player* player = get_player(player_entity);
		if (!player) return;
		player->contacting_pushable_block = false;
		audio::stop_event(player->stone_sliding_sound);
	}

	bool apply_damage_to_player(entt::entity entity, const Damage& damage)
	{
		if (damage.amount <= 0) return false;
		Player* player = get_player(entity);
		if (!player) return false;
		if (player->health <= 0) return false; // Player is already dead
		if (player->hurt_timer.running()) return false; // Player is invulnerable
		player->health = std::max(0, player->health - damage.amount);
		add_trauma_to_active_camera(0.8f);
		if (player->health > 0) { // Player survived
			audio::create_event({ .path = "event:/snd_player_hurt" });
			player->hurt_timer.start();
		} else { // Player died
			player->state = PlayerState::Dying;
		}
		return true;
	}
}
