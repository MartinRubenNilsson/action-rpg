#include "stdafx.h"
#include "map_entities.h"
#include "tiled.h"
#include "console.h"
#include "audio.h"
#include "ecs_common.h"
#include "ecs_physics.h"
#include "ecs_physics_filters.h"
#include "ecs_tile.h"
#include "ecs_player.h"
#include "ecs_camera.h"
#include "ecs_ai.h"
#include "ecs_portal.h"
#include "ecs_character.h"
#include "ecs_chest.h"

// Precautionary measure so we don't access entt::registry directly in this file.
#define DONT_ACCESS_REGISTRY_DIRECTLY_IN_MAP_ENTITIES_USE_HELPER_FUNCTIONS_INSTEAD
#define registry DONT_ACCESS_REGISTRY_DIRECTLY_IN_MAP_ENTITIES_USE_HELPER_FUNCTIONS_INSTEAD
#define _registry DONT_ACCESS_REGISTRY_DIRECTLY_IN_MAP_ENTITIES_USE_HELPER_FUNCTIONS_INSTEAD

namespace map
{
	size_t get_object_layer_index();

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
				if (!object.class_.empty()) {
					ecs::set_class(entity, object.class_);
				}
				if (!object.properties.empty()) {
					ecs::set_properties(entity, object.properties);
				}

				Vector2f position = object.position;

				// Objects are positioned by their top-left corner, except for tiles,
				// which are positioned by their bottom-left corner. This is confusing,
				// so let's adjust the position here to make it consistent.
				if (object.type == tiled::ObjectType::Tile) {
					position.y -= object.size.y;
				}

				if (object.type == tiled::ObjectType::Tile) {
					const tiled::Tile* tile = object.get_tile();
					assert(tile && "Tile not found.");

					Vector2f sorting_pivot = object.size / 2.f;

					// LOAD COLLIDERS

					if (!tile->objects.empty()) {

						b2BodyDef body_def{};
						body_def.type = b2_dynamicBody;
						body_def.fixedRotation = true;
						body_def.position.Set(position.x, position.y);
						b2Body* body = ecs::emplace_body(entity, body_def);

						for (const tiled::Object& collider : tile->objects) {

							if (collider.name == "pivot") {
								sorting_pivot = collider.position;
							}

							float collider_x = collider.position.x;
							float collider_y = collider.position.y;
							float collider_hw = collider.size.x / 2.0f;
							float collider_hh = collider.size.y / 2.0f;
							Vector2f collider_center(collider_x + collider_hw, collider_y + collider_hh);

							switch (collider.type) {
							case tiled::ObjectType::Rectangle: {
								b2PolygonShape shape{};
								shape.SetAsBox(collider_hw, collider_hh, collider_center, 0.f);
								b2FixtureDef fixture_def{};
								fixture_def.shape = &shape;
								fixture_def.density = 1.f;
								if (!object.class_.empty()) {
									fixture_def.filter = ecs::get_filter_for_class(object.class_);
								}
								body->CreateFixture(&fixture_def);
							} break;
							case tiled::ObjectType::Ellipse: {
								b2CircleShape shape{};
								shape.m_p = collider_center;
								shape.m_radius = collider_hw;
								b2FixtureDef fixture_def{};
								fixture_def.shape = &shape;
								fixture_def.density = 1.f;
								if (!object.class_.empty()) {
									fixture_def.filter = ecs::get_filter_for_class(object.class_);
								}
								body->CreateFixture(&fixture_def);
							} break;
							}
						}
					}

					// EMPLACE TILE

					ecs::Tile& ecs_tile = ecs::emplace_tile(entity);
					ecs_tile.set_tile(tile);
					ecs_tile.position = position;
					ecs_tile.sorting_pivot = sorting_pivot;
					// PITFALL: We don't set the sorting layer to the layer index here.
					// This is because we want all objects to be on the same layer, so they
					// are rendered in the correct order. This sorting layer may also be the
					// index of a tile layer so that certain static tiles are rendered as if
					// they were objects, e.g. trees and other props.
					ecs_tile.sorting_layer = (uint8_t)get_object_layer_index();
					ecs_tile.set_flag(ecs::TILE_VISIBLE, layer.visible);
					ecs_tile.set_flag(ecs::TILE_FLIP_X, object.tile.flipped_horizontally);
					ecs_tile.set_flag(ecs::TILE_FLIP_Y, object.tile.flipped_vertically);
					ecs_tile.set_flag(ecs::TILE_FLIP_DIAGONAL, object.tile.flipped_diagonally);

				} else { // If object is not a tile

					// LOAD COLLIDERS

					b2BodyDef body_def{};
					body_def.type = b2_staticBody;
					body_def.fixedRotation = true;
					body_def.position.Set(position.x, position.y);
					b2Body* body = ecs::emplace_body(entity, body_def);

					float hw = object.size.x / 2.f;
					float hh = object.size.y / 2.f;
					Vector2f center(hw, hh);

					switch (object.type) {
					case tiled::ObjectType::Rectangle: {
						b2PolygonShape shape{};
						shape.SetAsBox(hw, hh, center, 0.f);
						b2FixtureDef fixture_def{};
						fixture_def.shape = &shape;
						fixture_def.isSensor = true;
						fixture_def.filter = ecs::get_filter_for_class(object.class_);
						body->CreateFixture(&fixture_def);
					} break;
					case tiled::ObjectType::Ellipse: {
						b2CircleShape shape{};
						shape.m_p = center;
						shape.m_radius = hw;
						b2FixtureDef fixture_def{};
						fixture_def.shape = &shape;
						fixture_def.isSensor = true;
						fixture_def.filter = ecs::get_filter_for_class(object.class_);
						body->CreateFixture(&fixture_def);
					} break;
					}
				}

				// CLASS-SPECIFIC ENTITY SETUP

				if (object.class_ == "player") {

					ecs::Player player{};
					if (last_player) {
						player = *last_player;
						player.input_flags = 0;
					}

					const Vector2f pivot = { 32.f, 42.f };

					b2BodyDef body_def{};
					body_def.type = b2_dynamicBody;
					body_def.position = position + pivot;
					b2Body* body = ecs::emplace_body(entity, body_def);

					b2CircleShape shape{};
					shape.m_radius = 7.f;

					b2FixtureDef fixture_def{};
					fixture_def.shape = &shape;
					fixture_def.density = 1.f;
					fixture_def.filter = ecs::get_filter_for_class(object.class_);
					body->CreateFixture(&fixture_def);

					ecs::Tile* tile = ecs::get_tile(entity);
					if (tile) {
						tile->pivot = pivot;
						tile->sorting_pivot = pivot;
					}

					if (last_active_portal) {

						if (const tiled::Object* target_point = tiled::find_object_by_name(map, last_active_portal->target_point)) {
							if (body) {
								body->SetTransform(target_point->position, 0.f);
							}
							if (tile) {
								position = target_point->position;
							}
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
					ecs::emplace_tile(player.held_item);
					ecs::emplace_player(entity, player);

					ecs::Camera camera{};
					camera.view.center = position;
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

					if (tile) {
						tile->texture = character.texture;
					}

				} else if (object.class_ == "slime") {

					ecs::emplace_ai(entity, ecs::AiType::Slime);

				} else if (object.class_ == "portal") {

					ecs::Portal portal{};
					object.properties.get_string("target_map", portal.target_map);
					object.properties.get_string("target_point", portal.target_point);
					object.properties.get_string("exit_direction", portal.exit_direction);
					ecs::emplace_portal(entity, portal);

				} else if (object.class_ == "camera") {

					ecs::Camera camera{};
					camera.view.center = position;
					camera.confines_min = map_bounds_min;
					camera.confines_max = map_bounds_max;
					object.properties.get_entity("follow", camera.entity_to_follow);
					ecs::emplace_camera(entity, camera);

				} else if (object.class_ == "audio_source") {

					if (std::string event_name; object.properties.get_string("event", event_name)) {
						std::string path = "event:/" + event_name;
						audio::create_event({ .path = path.c_str(), .position = position });
					}

				} else if (object.class_ == "chest") {

					ecs::Chest chest{};
					if (std::string type; object.properties.get_string("type", type)) {
						if (type == "bomb") {
							chest.type = ecs::ChestType::Bomb;
						}
					}
					ecs::emplace_chest(entity, chest);

					const Vector2f pivot = { 16.f, 22.f };

					if (ecs::Tile* tile = ecs::get_tile(entity)) {
						tile->pivot = pivot;
						tile->sorting_pivot = pivot;
					}

					b2BodyDef body_def{};
					body_def.type = b2_staticBody;
					body_def.position = position + pivot;
					b2Body* body = ecs::emplace_body(entity, body_def);

					b2PolygonShape shape{};
					shape.SetAsBox(10.f, 6.f, b2Vec2_zero, 0.f);
					body->CreateFixture(&shape, 0.f);
				}
			}
		}

		// Create and setup tile entities.
		for (size_t layer_index = 0; layer_index < map.layers.size(); ++layer_index) {
			const tiled::Layer& layer = map.layers[layer_index];
			if (layer.type != tiled::LayerType::Tile) continue;
			for (unsigned int x = 0; x < layer.height; x++) {
				for (unsigned int y = 0; y < layer.width; y++) {

					const tiled::TileRef tile_ref = layer.tiles[x * layer.width + y];
					if (!tile_ref.gid) continue;
					const tiled::Tile* tile = map.get_tile(tile_ref.gid);
					if (!tile) {
						console::log_error("Tile not found for GID " + std::to_string(tile_ref.gid));
						continue;
					}

					const Vector2f position = { (float)y * map.tile_width, (float)x * map.tile_height };
					const Vector2f pivot = { 0.f, (float)tile->height - (float)map.tile_height };
					Vector2f sorting_pivot = { tile->width / 2.f, tile->height - map.tile_height / 2.f };

					entt::entity entity = ecs::create();
					if (!tile->class_.empty()) {
						ecs::set_class(entity, tile->class_);
					}
					if (!tile->properties.empty()) {
						ecs::set_properties(entity, tile->properties);
					}

					// EMPLACE BODY

					if (!tile->objects.empty()) {

						b2BodyDef body_def{};
						body_def.type = b2_staticBody;
						body_def.position = position;
						body_def.fixedRotation = true;
						b2Body* body = ecs::emplace_body(entity, body_def);

						for (const tiled::Object& collider : tile->objects) {
							if (collider.name == "pivot") {
								sorting_pivot = collider.position;
							}

							float collider_cx = collider.position.x - pivot.x;
							float collider_cy = collider.position.y - pivot.y;
							float collider_hw = collider.size.x / 2.0f;
							float collider_hh = collider.size.y / 2.0f;

							b2FixtureDef fixture_def{};
							collider.properties.get_bool("sensor", fixture_def.isSensor);

							switch (collider.type) {
							case tiled::ObjectType::Rectangle: {
								b2PolygonShape shape{};
								shape.SetAsBox(collider_hw, collider_hh,
									b2Vec2(collider_cx + collider_hw, collider_cy + collider_hh), 0.f);
								fixture_def.shape = &shape;
								body->CreateFixture(&fixture_def);
							} break;
							case tiled::ObjectType::Ellipse: {
								b2CircleShape shape{};
								shape.m_p.x = collider_cx;
								shape.m_p.y = collider_cy;
								shape.m_radius = collider_hw;
								fixture_def.shape = &shape;
								body->CreateFixture(&fixture_def);
							} break;
							case tiled::ObjectType::Polygon: {
								const size_t count = collider.points.size();
								if (count < 3) {
									console::log_error("Too few points in polygon collider! Got " +
										std::to_string(count) + ", need >= 3.");
								} else if (count <= b2_maxPolygonVertices && is_convex(collider.points)) {
									b2Vec2 points[b2_maxPolygonVertices];
									for (size_t i = 0; i < count; ++i) {
										points[i].x = collider_cx + collider.points[i].x;
										points[i].y = collider_cy + collider.points[i].y;
									}
									b2PolygonShape shape{};
									shape.Set(points, (int32)count);
									fixture_def.shape = &shape;
									body->CreateFixture(&fixture_def);
								} else {
									const std::vector<Vector2f> triangles = triangulate(collider.points);
									for (size_t i = 0; i < triangles.size(); i += 3) {
										b2Vec2 points[3];
										for (size_t j = 0; j < 3; ++j) {
											points[j].x = collider_cx + triangles[i + j].x;
											points[j].y = collider_cy + triangles[i + j].y;
										}
										b2PolygonShape shape{};
										shape.Set(points, 3);
										fixture_def.shape = &shape;
										body->CreateFixture(&fixture_def);
									}
								} break;
							}
							}
						}
					}

					// CRITICAL: This is an important optimization. Iterating through all entities
					// with both a Tile and b2Body* component can be expensive if there are many such
					// entities. "Pure" colliders don't need a tile component, so let's skip adding one.
					if (layer.name == "colliders" ||
						layer.name == "Colliders" ||
						layer.name == "collision" ||
						layer.name == "Collision") {
						continue;
					}

					// EMPLACE TILE

					ecs::Tile& ecs_tile = ecs::emplace_tile(entity);
					ecs_tile.set_tile(tile);
					ecs_tile.position = position;
					ecs_tile.pivot = pivot;
					ecs_tile.sorting_layer = (uint8_t)layer_index;
					ecs_tile.sorting_pivot = sorting_pivot;
					ecs_tile.set_flag(ecs::TILE_VISIBLE, layer.visible);
					ecs_tile.set_flag(ecs::TILE_FLIP_X, tile_ref.flipped_horizontally);
					ecs_tile.set_flag(ecs::TILE_FLIP_Y, tile_ref.flipped_vertically);
					ecs_tile.set_flag(ecs::TILE_FLIP_DIAGONAL, tile_ref.flipped_diagonally);
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