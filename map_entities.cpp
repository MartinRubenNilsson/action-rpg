#include "stdafx.h"
#include "map_entities.h"
#include "tiled.h"
#include "console.h"
#include "ecs_common.h"
#include "ecs_physics.h"
#include "ecs_physics_filters.h"
#include "ecs_tile.h"
#include "ecs_player.h"
#include "ecs_camera.h"
#include "ecs_ai.h"
#include "character.h"
#include "audio.h"

namespace map
{
	void create_entities(const tiled::Map& map)
	{
		const sf::FloatRect map_bounds{ 0.f, 0.f,
			(float)map.width * map.tile_width,
			(float)map.height * map.tile_height };

		// Create object entities first. This is because we want to be sure that the
		// object UIDs we get from Tiled are free to use as entity identifiers.
		for (const tiled::Layer& layer : map.layers) {
			if (layer.objects.empty()) continue;
			ecs::SortingLayer sorting_layer = ecs::layer_name_to_sorting_layer(layer.name);
			for (const tiled::Object& object : layer.objects) {

				// Attempt to use the object's UID as the entity identifier.
				// If the identifier is already in use, a new one will be generated.
				entt::entity entity = ecs::create(object.entity);

				if (!object.name.empty())
					ecs::set_name(entity, object.name);
				if (!object.class_.empty())
					ecs::set_class(entity, object.class_);
				if (!object.properties.empty())
					ecs::set_properties(entity, object.properties);

				float x = object.position.x;
				float y = object.position.y;
				float w = object.size.x;
				float h = object.size.y;

				if (object.type == tiled::ObjectType::Tile) {
					assert(object.tile && "Tile not found.");

					// Objects are positioned by their top-left corner, except for tiles,
					// which are positioned by their bottom-left corner. This is confusing,
					// so let's adjust the position here to make it consistent.
					y -= h;

					ecs::Tile& ecs_tile = ecs::emplace_tile(entity, object.tile);
					ecs_tile.position = sf::Vector2f(x, y);
					ecs_tile.sorting_layer = ecs::SortingLayer::Objects;
					ecs_tile.sorting_pivot = sf::Vector2f(w / 2.f, h / 2.f);
					ecs_tile.set_flag(ecs::TF_VISIBLE, layer.visible);
					ecs_tile.set_flag(ecs::TF_FLIP_X, object.flip_flags & tiled::FLIP_HORIZONTAL);
					ecs_tile.set_flag(ecs::TF_FLIP_Y, object.flip_flags & tiled::FLIP_VERTICAL);

					// LOAD COLLIDERS

					if (!object.tile->objects.empty()) {
						b2BodyDef body_def;
						body_def.type = b2_dynamicBody;
						body_def.fixedRotation = true;
						body_def.position.x = x;
						body_def.position.y = y;
						b2Body* body = ecs::emplace_body(entity, body_def);

						for (const tiled::Object& collider : object.tile->objects) {
							if (collider.name == "pivot")
								ecs_tile.sorting_pivot = collider.position;

							float collider_x = collider.position.x;
							float collider_y = collider.position.y;
							float collider_hw = collider.size.x / 2.0f;
							float collider_hh = collider.size.y / 2.0f;
							b2Vec2 collider_center(collider_x + collider_hw, collider_y + collider_hh);

							switch (collider.type) {
							case tiled::ObjectType::Rectangle: {
								b2PolygonShape shape{};
								shape.SetAsBox(collider_hw, collider_hh, collider_center, 0.f);
								b2FixtureDef fixture_def{};
								fixture_def.shape = &shape;
								fixture_def.density = 1.f;
								if (!object.class_.empty())
									fixture_def.filter = ecs::get_filter_for_class(object.class_);
								body->CreateFixture(&fixture_def);
							} break;
							case tiled::ObjectType::Ellipse: {
								b2CircleShape shape{};
								shape.m_p = collider_center;
								shape.m_radius = collider_hw;
								b2FixtureDef fixture_def{};
								fixture_def.shape = &shape;
								fixture_def.density = 1.f;
								if (!object.class_.empty())
									fixture_def.filter = ecs::get_filter_for_class(object.class_);
								body->CreateFixture(&fixture_def);
							} break;
							}
						}
					}
				} else { // If object is not a tile

					// LOAD COLLIDERS

					b2BodyDef body_def;
					body_def.type = b2_staticBody;
					body_def.fixedRotation = true;
					body_def.position.x = x;
					body_def.position.y = y;
					b2Body* body = ecs::emplace_body(entity, body_def);

					float hw = w / 2.0f;
					float hh = h / 2.0f;
					b2Vec2 center(hw, hh);

					switch (object.type) {
					case tiled::ObjectType::Rectangle: {
						b2PolygonShape shape;
						shape.SetAsBox(hw, hh, center, 0.f);
						b2FixtureDef fixture_def;
						fixture_def.shape = &shape;
						fixture_def.isSensor = true;
						fixture_def.filter = ecs::get_filter_for_class(object.class_);
						body->CreateFixture(&fixture_def);
					} break;
					case tiled::ObjectType::Ellipse: {
						b2CircleShape shape;
						shape.m_p = center;
						shape.m_radius = hw;
						b2FixtureDef fixture_def;
						fixture_def.shape = &shape;
						fixture_def.isSensor = true;
						fixture_def.filter = ecs::get_filter_for_class(object.class_);
						body->CreateFixture(&fixture_def);
					} break;
					}
				}

				// CLASS-SPECIFIC COMPONENTS

				if (object.class_ == "player") {
					ecs::emplace_player(entity);

					ecs::Camera camera;
					camera.follow = entity;
					camera.confining_rect = map_bounds;
					ecs::emplace_camera(entity, camera);
					ecs::activate_camera(entity, true);

					Character character{};
					character.randomize();
					if (ecs::Tile* tile = ecs::try_get_tile(entity))
						tile->texture = character.bake_texture();

				} else if (object.class_ == "slime") {
					ecs::emplace_ai(entity, ecs::AiType::Slime);
				} else if (object.class_ == "camera") {
					ecs::Camera camera;
					camera.view.setCenter(x, y);
					ecs::get_entity(entity, "follow", camera.follow);
					camera.confining_rect = map_bounds;
					ecs::emplace_camera(entity, camera);
				} else if (object.class_ == "audio_source") {
					std::string event_name;
					if (object.properties.get_string("event", event_name)) {
						audio::EventOptions options{};
						options.position = sf::Vector2f(x, y);
						audio::play("event:/" + event_name, options);
					}
				}
			}
		}

		// Create tile entities second.
		for (const tiled::Layer& layer : map.layers) {
			if (layer.tiles.empty()) continue;
			ecs::SortingLayer sorting_layer = ecs::layer_name_to_sorting_layer(layer.name);
			for (uint32_t tile_y = 0; tile_y < layer.height; tile_y++) {
				for (uint32_t tile_x = 0; tile_x < layer.width; tile_x++) {

					const auto [tile, flip_flags] = layer.tiles[tile_y * layer.width + tile_x];
					if (!tile) continue;

					float position_x = (float)tile_x * map.tile_width;
					float position_y = (float)tile_y * map.tile_height;
					float pivot_x = 0.f;
					float pivot_y = (float)(tile->tileset->tile_height - map.tile_height);
					float sorting_pivot_x = tile->tileset->tile_width / 2.f;
					float sorting_pivot_y = tile->tileset->tile_height - map.tile_height / 2.f;

					entt::entity entity = ecs::create();
					if (!tile->class_.empty())
						ecs::set_class(entity, tile->class_);
					if (!tile->properties.empty())
						ecs::set_properties(entity, tile->properties);

					ecs::Tile& ecs_tile = ecs::emplace_tile(entity, tile);
					ecs_tile.position = sf::Vector2f(position_x, position_y);
					ecs_tile.pivot = sf::Vector2f(pivot_x, pivot_y);
					ecs_tile.sorting_layer = sorting_layer;
					ecs_tile.sorting_pivot = sf::Vector2f(sorting_pivot_x, sorting_pivot_y);
					ecs_tile.set_flag(ecs::TF_VISIBLE, layer.visible);
					ecs_tile.set_flag(ecs::TF_FLIP_X, flip_flags & tiled::FLIP_HORIZONTAL);
					ecs_tile.set_flag(ecs::TF_FLIP_Y, flip_flags & tiled::FLIP_VERTICAL);

					if (tile->objects.empty())
						continue;

					// LOAD COLLIDERS

					b2BodyDef body_def;
					body_def.type = b2_staticBody;
					body_def.position.x = position_x;
					body_def.position.y = position_y;
					body_def.fixedRotation = true;
					b2Body* body = ecs::emplace_body(entity, body_def);

					for (const tiled::Object& collider : tile->objects) {
						if (collider.name == "pivot")
							ecs_tile.sorting_pivot = collider.position;

						float collider_cx = (collider.position.x - pivot_x);
						float collider_cy = (collider.position.y - pivot_y);
						float collider_hw = collider.size.x / 2.0f;
						float collider_hh = collider.size.y / 2.0f;

						b2FixtureDef fixture_def;
						collider.properties.get_bool("sensor", fixture_def.isSensor);

						switch (collider.type) {
						case tiled::ObjectType::Rectangle: {
							b2PolygonShape shape;
							shape.SetAsBox(collider_hw, collider_hh,
								b2Vec2(collider_cx + collider_hw, collider_cy + collider_hh), 0.f);
							fixture_def.shape = &shape;
							body->CreateFixture(&fixture_def);
						} break;
						case tiled::ObjectType::Ellipse: {
							b2CircleShape shape;
							shape.m_p.x = collider_cx;
							shape.m_p.y = collider_cy;
							shape.m_radius = collider_hw;
							fixture_def.shape = &shape;
							body->CreateFixture(&fixture_def);
						} break;
						case tiled::ObjectType::Polygon: {
							size_t count = collider.points.size();
							if (count < 3) {
								console::log_error(
									"Too few points in polygon collider! Got " +
									std::to_string(count) + ", need >= 3.");
							} else if (count <= b2_maxPolygonVertices && is_convex(collider.points)) {
								b2Vec2 points[b2_maxPolygonVertices];
								for (size_t i = 0; i < count; ++i) {
									points[i].x = collider_cx + collider.points[i].x;
									points[i].y = collider_cy + collider.points[i].y;
								}
								b2PolygonShape shape;
								shape.Set(points, (int32)count);
								fixture_def.shape = &shape;
								body->CreateFixture(&fixture_def);
							} else {
								for (const std::array<sf::Vector2f, 3>&triangle : triangulate(collider.points)) {
									b2Vec2 points[3];
									for (size_t i = 0; i < 3; ++i) {
										points[i].x = collider_cx + triangle[i].x;
										points[i].y = collider_cy + triangle[i].y;
									}
									b2PolygonShape shape;
									shape.Set(points, 3);
									fixture_def.shape = &shape;
									body->CreateFixture(&fixture_def);
								}
							} break;
						}
						}
					}
				}
			}
		}
	}

	void destroy_entities() {
		ecs::clear();
	}
}