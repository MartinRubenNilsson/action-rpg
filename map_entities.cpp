#include "stdafx.h"
#include "map_entities.h"
#include "tiled.h"
#include "console.h"
#include "audio.h"
#include "graphics.h"
#include "ecs_common.h"
#include "ecs_physics.h"
#include "ecs_physics_filters.h"
#include "ecs_sprite.h"
#include "ecs_animation.h"
#include "ecs_player.h"
#include "ecs_camera.h"
#include "ecs_ai.h"
#include "ecs_portal.h"
#include "ecs_character.h"
#include "ecs_chest.h"
#include "ecs_blade_trap.h"

// Precautionary measure so we don't access entt::registry directly in this file.
#define DONT_ACCESS_REGISTRY_DIRECTLY_IN_MAP_ENTITIES_USE_HELPER_FUNCTIONS_INSTEAD
#define registry DONT_ACCESS_REGISTRY_DIRECTLY_IN_MAP_ENTITIES_USE_HELPER_FUNCTIONS_INSTEAD
#define _registry DONT_ACCESS_REGISTRY_DIRECTLY_IN_MAP_ENTITIES_USE_HELPER_FUNCTIONS_INSTEAD

namespace map
{
	unsigned int get_object_layer_index();

	void create_entities(const tiled::Map& map)
	{
		//
		// IMPORTANT:
		// 
		// In one of my school projects, my group had a similar function to this one,
		// with the difference being that we used functions like entt::registry::emplace()
		// directly. It turns out that this caused a lot of template bloat, which slowed down
		// compilation times, and eventually made the .cpp file too large to compile.
		// 
		// The solution was to wrap calls to entt::registry::emplace() in our own helper functions
		// and put them in separate .cpp files, so each template is instantiated in its own unit.
		// This is why we have functions like ecs::emplace_tile() and ecs::emplace_body().
		//
		// TLDR: You may NOT access entt::registry directly in this file! Make a helper function
		// in ecs_[your component here].cpp and call that instead!
		//

		const Vector2f map_bounds_min = { 0.f, 0.f };
		const Vector2f map_bounds_max = {
			(float)map.width * map.tile_width,
			(float)map.height * map.tile_height };

		// Save some components to be carried over between maps,
		// or that is needed in general when populating the new map.
		std::optional<ecs::Player> last_player;
		std::optional<ecs::Character> last_player_character;
		std::optional<ecs::Portal> last_active_portal;
		{
			entt::entity player_entity = ecs::find_player_entity();
			if (ecs::Player* player = ecs::get_player(player_entity)) {
				last_player = *player;
			}
			if (ecs::Character* character = ecs::get_character(player_entity)) {
				last_player_character = *character;
			}
			entt::entity portal_entity = ecs::find_active_portal_entity();
			if (ecs::Portal* portal = ecs::get_portal(portal_entity)) {
				last_active_portal = *portal;
			}
		}

		// Destroy all entities before creating new ones.

		destroy_entities();

		// Pre-create object entities. This is because we want to be sure that the
		// object UIDs we get from Tiled are free to use as entity identifiers.

		for (const tiled::Layer& layer : map.layers) {
			if (layer.type != tiled::LayerType::Object) continue;
			for (const tiled::Object& object : layer.objects) {
				ecs::create(object.id);
			}
		}

		// Setup object entities.

		for (const tiled::Layer& layer : map.layers) {
			if (layer.type != tiled::LayerType::Object) continue;
			for (const tiled::Object& object : layer.objects) {

				// At this point, a corresponding entity should have been created
				// that reuses the object UID from Tiled. If not, something went wrong.
				entt::entity entity = object.id;
				assert(ecs::valid(entity));

				if (!object.name.empty()) {
					ecs::set_name(entity, object.name);
				}

				ecs::Class class_ = ecs::Class::None;
				if (!object.class_.empty() && ecs::string_to_class(object.class_, class_)) {
					ecs::set_class(entity, class_);
				}

				if (!object.properties.empty()) {
					ecs::set_properties(entity, object.properties);
				}

				// In Tiled, objects are positioned by their top-left corner...
				Vector2f position_top_left = object.position;

				switch (object.type) {
				case tiled::ObjectType::Tile: {

					// ...unless it's a tile, in which case it's positioned by its bottom-left corner.
					// This is confusing, so let's adjust the position here to make it consistent.
					position_top_left.y -= object.size.y;

					const tiled::Tileset* tileset = tiled::get_tileset(object.tileset_ref.tileset);
					if (!tileset) {
						console::log_error("Tileset not found for object " + object.name);
						continue;
					}

					const unsigned int tile_id = object.tile_ref.gid - object.tileset_ref.first_gid;
					if (tile_id >= tileset->tiles.size()) {
						console::log_error("Tile not found for object " + object.name);
						continue;
					}

					const tiled::Tile& tile = tileset->tiles[tile_id];

					// EMPLACE SPRITE

					tiled::TextureRect tex_rect = tileset->get_texture_rect(tile_id);

					sprites::Sprite& sprite = ecs::emplace_sprite(entity);
					sprite.shader = graphics::sprite_shader;
					sprite.texture = graphics::load_texture(tileset->image_path);
					sprite.position = position_top_left;
					sprite.size = object.size;
					sprite.tex_position = { (float)tex_rect.x, (float)tex_rect.y };
					sprite.tex_size = { (float)tex_rect.w, (float)tex_rect.h };
					Vector2u texture_size;
					graphics::get_texture_size(sprite.texture, texture_size.x, texture_size.y);
					sprite.tex_position /= Vector2f(texture_size);
					sprite.tex_size /= Vector2f(texture_size);
					// PITFALL: We don't set the sorting layer to the layer index here.
					// This is because we want all objects to be on the same layer, so they
					// are rendered in the correct order. This sorting layer may also be the
					// index of a tile layer so that certain static tiles are rendered as if
					// they were objects, e.g. trees and other props.
					sprite.sorting_layer = (uint8_t)get_object_layer_index();
					sprite.sorting_point = object.size / 2.f;
					if (!layer.visible) {
						sprite.flags &= ~sprites::SPRITE_VISIBLE;
					}
					if (object.tile_ref.flipped_horizontally) {
						sprite.flags |= sprites::SPRITE_FLIP_HORIZONTALLY;
					}
					if (object.tile_ref.flipped_vertically) {
						sprite.flags |= sprites::SPRITE_FLIP_VERTICALLY;
					}
					if (object.tile_ref.flipped_diagonally) {
						sprite.flags |= sprites::SPRITE_FLIP_DIAGONALLY;
					}

					// EMPLACE ANIMATION

					ecs::Animation& animation = ecs::emplace_animation(entity);
					animation.tileset = object.tileset_ref.tileset;
					animation.tile_id = tile_id;

					if (!tile.objects.empty()) {

						// EMPLACE SPRITE-BODY ATTACHMENT

						ecs::emplace_sprite_body_attachment(entity);

						// EMPLACE BODY

						b2BodyDef body_def = b2DefaultBodyDef();
						body_def.type = b2_dynamicBody;
						body_def.fixedRotation = true;
						body_def.position = position_top_left;
						b2BodyId body = ecs::emplace_body(entity, body_def);

						for (const tiled::Object& collider : tile.objects) {

							const float coll_x = collider.position.x;
							const float coll_y = collider.position.y;
							const float coll_hw = collider.size.x / 2.f;
							const float coll_hh = collider.size.y / 2.f;
							const Vector2f coll_center = { coll_x + coll_hw, coll_y + coll_hh };

							switch (collider.type) {
							case tiled::ObjectType::Rectangle: {

								b2ShapeDef shape_def = b2DefaultShapeDef();
								shape_def.filter = ecs::get_filter_for_class(object.class_);
								b2Polygon box = b2MakeOffsetBox(coll_hw, coll_hh, coll_center, 0.f);
								b2CreatePolygonShape(body, &shape_def, &box);

							} break;
							case tiled::ObjectType::Ellipse: {

								b2ShapeDef shape_def = b2DefaultShapeDef();
								shape_def.filter = ecs::get_filter_for_class(object.class_);
								b2Circle circle{};
								circle.center = coll_center;
								circle.radius = coll_hw;
								b2CreateCircleShape(body, &shape_def, &circle);

							} break;
							}
						}
					}

				} break;
				default: { // Rectangle, Ellipse, Point, Polygon, Polyline

					// CREATE SENSORS

					b2BodyDef body_def = b2DefaultBodyDef();
					body_def.type = b2_staticBody;
					body_def.fixedRotation = true;
					body_def.position = position_top_left;
					b2BodyId body = ecs::emplace_body(entity, body_def);

					const float hw = object.size.x / 2.f;
					const float hh = object.size.y / 2.f;
					const Vector2f center = { hw, hh };

					switch (object.type) {
					case tiled::ObjectType::Rectangle: {

						b2ShapeDef shape_def = b2DefaultShapeDef();
						shape_def.isSensor = true;
						shape_def.filter = ecs::get_filter_for_class(object.class_);
						b2Polygon box = b2MakeOffsetBox(hw, hh, center, 0.f);
						b2CreatePolygonShape(body, &shape_def, &box);

					} break;
					case tiled::ObjectType::Ellipse: {

						b2ShapeDef shape_def = b2DefaultShapeDef();
						shape_def.isSensor = true;
						shape_def.filter = ecs::get_filter_for_class(object.class_);
						b2Circle circle{};
						circle.center = center;
						circle.radius = hw;
						b2CreateCircleShape(body, &shape_def, &circle);	

					} break;
					}

				} break;
				}

				// CLASS-SPECIFIC ENTITY SETUP

				switch (class_) {
				case ecs::Class::Player: {

					ecs::Player player{};
					if (last_player) {
						player = *last_player;
						player.input_flags = 0;
					}

					const Vector2f pivot = { 32.f, 42.f };

					if (sprites::Sprite* sprite = ecs::get_sprite(entity)) {
						sprite->sorting_point = pivot;
					}
					{
						b2BodyDef body_def = b2DefaultBodyDef();
						body_def.type = b2_dynamicBody;
						body_def.position = position_top_left + pivot;
						body_def.fixedRotation = true;
						b2BodyId body = ecs::emplace_body(entity, body_def);
						b2ShapeDef shape_def = b2DefaultShapeDef();
						shape_def.filter = ecs::get_filter_for_class(object.class_);
						b2Circle circle{};
						circle.radius = 7.f;
						b2CreateCircleShape(body, &shape_def, &circle);
					}

					ecs::emplace_sprite_body_attachment(entity, -pivot);

					if (last_active_portal) {

						if (const tiled::Object* target_point = map.get_object(last_active_portal->target_point)) {
#if 0
							if (body) {
								body->SetTransform(target_point->position_top_left, 0.f);
							}
#endif
						}

						if (last_active_portal->exit_direction == "up") {
							player.look_dir = { 0.f, -1.f };
						} else if (last_active_portal->exit_direction == "down") {
							player.look_dir = { 0.f, 1.f };
						} else if (last_active_portal->exit_direction == "left") {
							player.look_dir = { -1.f, 0.f };
						} else if (last_active_portal->exit_direction == "right") {
							player.look_dir = { 1.f, 0.f };
						}
					}

					player.held_item = ecs::create();
					ecs::emplace_animation(player.held_item);
					ecs::emplace_player(entity, player);

					ecs::Camera camera{};
					camera.center = position_top_left;
					camera.confines_min = map_bounds_min;
					camera.confines_max = map_bounds_max;
					camera.entity_to_follow = entity;
					ecs::emplace_camera(entity, camera);
					ecs::activate_camera(entity, true);

					ecs::Character character{};
					if (last_player_character) {
						character = *last_player_character;
					} else {
						ecs::randomize_character(character);
					}
					ecs::emplace_character(entity, character);
					ecs::regenerate_character_texture(character);

#if 0
					if (tile) {
						tile->texture = character.texture;
					}
#endif

				} break;
				case ecs::Class::Slime: {

					ecs::emplace_ai(entity, ecs::AiType::Slime);

				} break;
				case ecs::Class::Portal: {

					ecs::Portal& portal = ecs::emplace_portal(entity);
					object.properties.get_string("target_map", portal.target_map);
					object.properties.get_string("target_point", portal.target_point);
					object.properties.get_string("exit_direction", portal.exit_direction);

				} break;
				case ecs::Class::Camera: {

					ecs::Camera& camera = ecs::emplace_camera(entity);
					camera.center = position_top_left;
					camera.confines_min = map_bounds_min;
					camera.confines_max = map_bounds_max;
					object.properties.get_entity("follow", camera.entity_to_follow);

				} break;
				case ecs::Class::Chest: {

					ecs::Chest chest{};
					if (std::string type; object.properties.get_string("type", type)) {
						if (type == "bomb") {
							chest.type = ecs::ChestType::Bomb;
						}
					}
					ecs::emplace_chest(entity, chest);

					const Vector2f pivot = { 16.f, 22.f };
					{
						b2BodyDef body_def = b2DefaultBodyDef();
						body_def.type = b2_staticBody;
						body_def.position = position_top_left + pivot;
						body_def.fixedRotation = true;
						b2BodyId body = ecs::emplace_body(entity, body_def);
						b2ShapeDef shape_def = b2DefaultShapeDef();
						b2Polygon box = b2MakeBox(10.f, 6.f);
						b2CreatePolygonShape(body, &shape_def, &box);
					}

				} break;
				case ecs::Class::BladeTrap: {

					const Vector2f pivot = { 8.f, 8.f };

					ecs::BladeTrap& blade_trap = ecs::emplace_blade_trap(entity);
					blade_trap.start_position = position_top_left + pivot;

					if (sprites::Sprite* sprite = ecs::get_sprite(entity)) {
						sprite->sorting_point = pivot;
					}
					{
						b2BodyDef body_def = b2DefaultBodyDef();
						body_def.type = b2_dynamicBody;
						body_def.position = position_top_left + pivot;
						body_def.fixedRotation = true;
						b2BodyId body = ecs::emplace_body(entity, body_def);
						b2ShapeDef shape_def = b2DefaultShapeDef();
						b2Circle circle{};
						circle.radius = 6.f;
						b2CreateCircleShape(body, &shape_def, &circle);
					}

					ecs::emplace_sprite_body_attachment(entity, -pivot);

				} break;
				}
			}
		}

		// Create and setup tile entities.

		for (size_t layer_index = 0; layer_index < map.layers.size(); ++layer_index) {
			const tiled::Layer& layer = map.layers[layer_index];
			if (layer.type != tiled::LayerType::Tile) continue;

			// OPTIMIZATION: When iterating through the view of all ecs::Tile components, EnTT
			// returns them in reverse order of creation. Let's therefore CREATE them in reverse
			// draw order (bottom-to-top and right-to-left) so that when we iterate we access them
			// in draw order (left-to-right and top-to-bottom). This makes it so we spend less time
			// sorting them before rendering.

			for (unsigned int y = layer.height; y--;) {
				for (unsigned int x = layer.width; x--;) {

					const tiled::TileRef tile_ref = layer.tiles[x + y * layer.width];
					if (!tile_ref.gid) continue; // Skip empty tiles

					const tiled::TilesetRef tileset_ref = map.get_tileset_ref(tile_ref.gid);
					if (!tileset_ref.first_gid) continue; // Valid GIDs start at 1

					const tiled::Tileset* tileset = tiled::get_tileset(tileset_ref.tileset);
					if (!tileset) {
						console::log_error("Tileset not found for GID " + std::to_string(tile_ref.gid));
						continue;
					}

					const unsigned int tile_id = tile_ref.gid - tileset_ref.first_gid;
					if (tile_id >= tileset->tiles.size()) {
						console::log_error("Tile not found for GID " + std::to_string(tile_ref.gid));
						continue;
					}

					const tiled::Tile& tile = tileset->tiles[tile_id];

					const Vector2f position = {
						(float)x * map.tile_width,
						(float)y * map.tile_height - tileset->tile_height + map.tile_height
					};
					const Vector2f size = { (float)tileset->tile_width, (float)tileset->tile_height };
					const Vector2f sorting_point = { size.x / 2.f, size.y - map.tile_height / 2.f };

					entt::entity entity = ecs::create();
					ecs::Class class_ = ecs::Class::None;
					if (!tile.class_.empty() && ecs::string_to_class(tile.class_, class_)) {
						ecs::set_class(entity, class_);
					}
					if (!tile.properties.empty()) {
						ecs::set_properties(entity, tile.properties);
					}

					// EMPLACE SPRITE

					tiled::TextureRect tex_rect = tileset->get_texture_rect(tile_id);

					sprites::Sprite& sprite = ecs::emplace_sprite(entity);
					sprite.shader = graphics::sprite_shader;
					sprite.texture = graphics::load_texture(tileset->image_path);
					sprite.position = position;
					sprite.size = size;
					sprite.tex_position = { (float)tex_rect.x, (float)tex_rect.y };
					sprite.tex_size = { (float)tex_rect.w, (float)tex_rect.h };
					Vector2u texture_size;
					graphics::get_texture_size(sprite.texture, texture_size.x, texture_size.y);
					sprite.tex_position /= Vector2f(texture_size);
					sprite.tex_size /= Vector2f(texture_size);
					sprite.sorting_layer = (uint8_t)layer_index;
					sprite.sorting_point = sorting_point;
					if (!layer.visible) {
						sprite.flags &= ~sprites::SPRITE_VISIBLE;
					}
					if (tile_ref.flipped_horizontally) {
						sprite.flags |= sprites::SPRITE_FLIP_HORIZONTALLY;
					}
					if (tile_ref.flipped_vertically) {
						sprite.flags |= sprites::SPRITE_FLIP_VERTICALLY;
					}
					if (tile_ref.flipped_diagonally) {
						sprite.flags |= sprites::SPRITE_FLIP_DIAGONALLY;
					}

					// EMPLACE ANIMATION

					// The majority of tiles are not animated and don't change during gameplay,
					// so let's only add an animation component if the tile is actually animated.
					if (!tile.animation.empty()) {
						ecs::Animation& animation = ecs::emplace_animation(entity);
						animation.tileset = tileset_ref.tileset;
						animation.tile_id = tile_id;
					}

					// EMPLACE BODY

					// PITFALL: We only create bodies for tiles that have colliders!
					b2BodyId body = b2_nullBodyId;

					if (!tile.objects.empty()) {

						b2BodyDef body_def = b2DefaultBodyDef();
						body_def.type = b2_staticBody;
						body_def.position = position;
						body_def.fixedRotation = true;
						body = ecs::emplace_body(entity, body_def);

						for (const tiled::Object& collider : tile.objects) {

							const Vector2f collider_center = collider.position;
							const Vector2f collider_half_size = collider.size / 2.f;

							b2ShapeDef shape_def = b2DefaultShapeDef();
							collider.properties.get_bool("sensor", shape_def.isSensor);

							switch (collider.type) {
							case tiled::ObjectType::Rectangle: {

								b2Polygon box = b2MakeOffsetBox(
									collider_half_size.x,
									collider_half_size.y,
									collider_center + collider_half_size, 0.f);
								b2CreatePolygonShape(body, &shape_def, &box);

							} break;
							case tiled::ObjectType::Ellipse: {

								b2Circle circle{};
								circle.center = collider_center;
								circle.radius = collider_half_size.x;
								b2CreateCircleShape(body, &shape_def, &circle);

							} break;
							case tiled::ObjectType::Polygon: {

								const int32_t count = (int32_t)collider.points.size();
								if (count < 3) {
									console::log_error("Too few points in polygon collider! Got " +
										std::to_string(count) + ", need >= 3.");
									break;
								}

								if (count <= b2_maxPolygonVertices && is_convex(collider.points)) {

									b2Vec2 points[b2_maxPolygonVertices];
									for (int32_t i = 0; i < count; ++i) {
										points[i] = collider_center + collider.points[i];
									}
									b2Hull hull = b2ComputeHull(points, count);
									if (!b2ValidateHull(&hull)) {
										console::log_error("Invalid hull in polygon collider!");
										break;
									}
									b2Polygon polygon = b2MakePolygon(&hull, 0.f);
									b2CreatePolygonShape(body, &shape_def, &polygon);
									break;
								}

								const std::vector<Vector2f> triangles = triangulate(collider.points);
								for (size_t i = 0; i < triangles.size(); i += 3) {
									b2Vec2 points[3];
									for (size_t j = 0; j < 3; ++j) {
										points[j] = collider_center + triangles[i + j];
									}
									b2Hull hull = b2ComputeHull(points, 3);
									if (!b2ValidateHull(&hull)) {
										console::log_error("Invalid hull in polygon collider!");
										continue;
									}
									b2Polygon polygon = b2MakePolygon(&hull, 0.f);
									b2CreatePolygonShape(body, &shape_def, &polygon);
								}

							} break;
							}
						}
					}

					// CLASS-SPECIFIC ENTITY SETUP


					switch (class_) {
					case ecs::Class::Grass: {

						//TODO: set shader and uniform buffer
						sprite.sorting_point = { 8.f, 28.f };

					} break;
					}
				}
			}
		}
	}

	void patch_entities(const MapPatch& patch)
	{
		// Patching is a way to modify the state of the map after it has been created.

		for (entt::entity entity : patch.destroyed_entities) {
			ecs::destroy_immediately(entity);
		}
		for (entt::entity entity : patch.opened_chests) {
			ecs::open_chest(entity, true);
		}
	}

	void destroy_entities()
	{
		ecs::clear();
	}
}