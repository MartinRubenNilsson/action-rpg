#include "map.h"
#include "defines.h"
#include "tiled.h"
#include "console.h"
#include "audio.h"
#include "math_vectors.h"
#include "ecs.h"
#include "ecs_tiled.h"
#include "ecs_physics.h"
#include "ecs_physics_filters.h"
#include "ecs_graphics.h"
#include "ecs_player.h"
#include "ecs_camera.h"
#include "ecs_ai.h"
#include "ui_textbox.h"

namespace map
{
	const tiled::Map* _current_map = nullptr;
	const tiled::Map* _next_map = nullptr;
	bool _reset_if_already_open = false;
	std::string _spawnpoint_entity_name; // entity to spawn the player at when opening the next map

	bool open(const std::string& map_name, bool reset_if_already_open)
	{
		_next_map = nullptr;
		for (const tiled::Map& map : tiled::get_maps()) {
			if (map.name == map_name) {
				_next_map = &map;
				break;
			}
		}
		_reset_if_already_open = reset_if_already_open;
		return _next_map;
	}

	void close() {
		_next_map = nullptr;
	}

	void reset()
	{
		_next_map = _current_map;
		_reset_if_already_open = true;
	}

	void update()
	{
		if (_next_map == _current_map && !_reset_if_already_open)
			return;

		std::string current_music;
		std::string next_music;

		// CLOSE CURRENT MAP

		if (_current_map) {
			tiled::get(_current_map->properties, "music", current_music);
			ecs::clear();
			ui::close_all_textboxes();
		}

		// OPEN NEXT MAP

		_current_map = _next_map;
		_reset_if_already_open = false;

		if (!_current_map) {
			audio::stop_all_in_bus();
			return;
		}

		tiled::get(_current_map->properties, "music", next_music);
		if (current_music != next_music) {
			if (!current_music.empty())
				audio::stop_all("event:/" + current_music);
			if (!next_music.empty())
				audio::play("event:/" + next_music);
		}

		// Create object entities first. This is because we want to be sure that the
		// object UIDs we get from Tiled are free to use as entity identifiers.
		for (const tiled::Layer& layer : _current_map->layers) {
			ecs::SortingLayer sorting_layer = ecs::layer_name_to_sorting_layer(layer.name);
			for (const tiled::Object& object : layer.objects) {

				// Attempt to use the object's UID as the entity identifier.
				// If the identifier is already in use, a new one will be generated.
				entt::entity entity = ecs::create(object.entity);
				ecs::emplace_name_class_and_properties(entity, object);

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
					ecs_tile.visible = layer.visible;
					ecs_tile.flip_x = object.flip_flags & tiled::FLIP_HORIZONTAL;
					ecs_tile.flip_y = object.flip_flags & tiled::FLIP_VERTICAL;
					ecs_tile.position = sf::Vector2f(x, y);
					ecs_tile.sorting_layer = ecs::SortingLayer::Objects;
					ecs_tile.sorting_pivot = sf::Vector2f(w / 2.f, h / 2.f);

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
							case tiled::ObjectType::Rectangle:
							{
								b2PolygonShape shape;
								shape.SetAsBox(collider_hw, collider_hh, collider_center, 0.f);
								b2FixtureDef fixture_def;
								fixture_def.shape = &shape;
								fixture_def.density = 1.f;
								fixture_def.filter = ecs::get_filter_for_class(object.class_);
								body->CreateFixture(&fixture_def);
								break;
							}
							case tiled::ObjectType::Ellipse:
							{
								b2CircleShape shape;
								shape.m_p = collider_center;
								shape.m_radius = collider_hw;
								b2FixtureDef fixture_def;
								fixture_def.shape = &shape;
								fixture_def.density = 1.f;
								fixture_def.filter = ecs::get_filter_for_class(object.class_);
								body->CreateFixture(&fixture_def);
								break;
							}
							}
						}
					}
				} else { // If object is not a tile
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
					case tiled::ObjectType::Rectangle:
					{
						b2PolygonShape shape;
						shape.SetAsBox(hw, hh, center, 0.f);
						b2FixtureDef fixture_def;
						fixture_def.shape = &shape;
						fixture_def.isSensor = true;
						fixture_def.filter = ecs::get_filter_for_class(object.class_);
						body->CreateFixture(&fixture_def);
						break;
					}
					case tiled::ObjectType::Ellipse:
					{
						b2CircleShape shape;
						shape.m_p = center;
						shape.m_radius = hw;
						b2FixtureDef fixture_def;
						fixture_def.shape = &shape;
						fixture_def.isSensor = true;
						fixture_def.filter = ecs::get_filter_for_class(object.class_);
						body->CreateFixture(&fixture_def);
						break;
					}
					}
				}

				// EMPLACE SPECIFIC COMPONENTS

				if (object.class_ == "player") {
					ecs::emplace_player(entity, ecs::Player());

					ecs::Camera camera;
					camera.follow = entity;
					camera.confining_rect = get_bounds();
					ecs::emplace_camera(entity, camera);
					ecs::activate_camera(entity, true);

					// Broken at the moment
					//// TODO: put spawnpoint entity name in data?
					//if (_spawnpoint_entity_name != "player")
					//{
					//	entt::entity spawnpoint_entity = ecs::find_entity_by_name(_spawnpoint_entity_name);
					//	if (auto object = ecs::get_registry().try_get<const tmx::Object*>(spawnpoint_entity))
					//	{
					//		tmx::Vector2f position = (*object)->getPosition();
					//		position *= METERS_PER_PIXEL;
					//		ecs::set_player_center(sf::Vector2f(position.x, position.y));
					//	}
					//}
				} else if (object.class_ == "slime") {
					ecs::emplace_slime_animation_controller(entity);
					ecs::emplace_ai(entity, ecs::AiType::Slime);
				} else if (object.class_ == "camera") {
					ecs::Camera camera;
					camera.view.setCenter(x, y);
					ecs::get_entity(entity, "follow", camera.follow);
					camera.confining_rect = get_bounds();
					ecs::emplace_camera(entity, camera);
				}
			}
		}

		// Create tile entities second.
		for (const tiled::Layer& layer : _current_map->layers) {
			ecs::SortingLayer sorting_layer = ecs::layer_name_to_sorting_layer(layer.name);
			for (uint32_t tile_y = 0; tile_y < layer.height; tile_y++) {
				for (uint32_t tile_x = 0; tile_x < layer.width; tile_x++) {

					const auto [tile, flip_flags] = layer.tiles[tile_y * layer.width + tile_x];
					if (!tile) continue;

					float position_x = (float)tile_x * _current_map->tile_width;
					float position_y = (float)tile_y * _current_map->tile_height;
					float pivot_x = 0.f;
					float pivot_y = (float)(tile->tileset->tile_height - _current_map->tile_height);
					float sorting_pivot_x = tile->tileset->tile_width / 2.f;
					float sorting_pivot_y = tile->tileset->tile_height - _current_map->tile_height / 2.f;

					entt::entity entity = ecs::create();
					ecs::Tile& ecs_tile = ecs::emplace_tile(entity, tile);
					ecs_tile.visible = layer.visible;
					ecs_tile.flip_x = flip_flags & tiled::FLIP_HORIZONTAL;
					ecs_tile.flip_y = flip_flags & tiled::FLIP_VERTICAL;
					ecs_tile.position = sf::Vector2f(position_x, position_y);
					ecs_tile.pivot = sf::Vector2f(pivot_x, pivot_y);
					ecs_tile.sorting_layer = sorting_layer;
					ecs_tile.sorting_pivot = sf::Vector2f(sorting_pivot_x, sorting_pivot_y);

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
						tiled::get(collider.properties, "sensor", fixture_def.isSensor);

						switch (collider.type) {
						case tiled::ObjectType::Rectangle:
						{
							b2PolygonShape shape;
							shape.SetAsBox(collider_hw, collider_hh,
								b2Vec2(collider_cx + collider_hw, collider_cy + collider_hh), 0.f);
							fixture_def.shape = &shape;
							body->CreateFixture(&fixture_def);
							break;
						}
						case tiled::ObjectType::Ellipse:
						{
							b2CircleShape shape;
							shape.m_p.x = collider_cx;
							shape.m_p.y = collider_cy;
							shape.m_radius = collider_hw;
							fixture_def.shape = &shape;
							body->CreateFixture(&fixture_def);
							break;
						}
						case tiled::ObjectType::Polygon:
						{
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
								for (const std::array<sf::Vector2f, 3>& triangle : triangulate(collider.points)) {
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
							}
							break;
						}
						}
					}
				}
			}
		}
	}

	std::string get_name() {
		return _current_map ? _current_map->name : "";
	}

	sf::FloatRect get_bounds()
	{
		if (!_current_map) return sf::FloatRect();
		return sf::FloatRect(0.f, 0.f,
			(float)_current_map->width * _current_map->tile_width,
			(float)_current_map->height * _current_map->tile_height);
	}

	void set_player_spawnpoint(const std::string& entity_name) {
		_spawnpoint_entity_name = entity_name;
	}

	bool play_footstep_sound_at(const sf::Vector2f& position)
	{
		if (!_current_map) return false;
		if (position.x < 0.f || position.y < 0.f) return false;
		const uint32_t x = (uint32_t)position.x;
		const uint32_t y = (uint32_t)position.y;
		const bool left = (position.x - (float)x) < 0.5f;
		const bool top  = (position.y - (float)y) < 0.5f;
		const int corner =
			top ? (left ? tiled::WangTile::TOP_LEFT    : tiled::WangTile::TOP_RIGHT)
			    : (left ? tiled::WangTile::BOTTOM_LEFT : tiled::WangTile::BOTTOM_RIGHT);
		for (const tiled::Layer& layer : std::ranges::reverse_view(_current_map->layers)) {
			if (x >= layer.width || y >= layer.height) continue;
			const auto [tile, flip_flags] = layer.tiles[y * layer.width + x];
			//TODO: take into account flip flags?
			if (!tile) continue;
			for (const tiled::WangTile& wangtile : tile->wangtiles) {
				const tiled::WangColor* wangcolor = wangtile.wangcolors[corner];
				if (!wangcolor) continue;
				if (wangcolor->name.empty()) continue;
				bool is_logging_errors = audio::log_errors;
				audio::log_errors = false; // so we don't get spammed with errors
				if (audio::set_parameter_label("terrain", wangcolor->name))
					audio::play("event:/snd_step");
				audio::log_errors = is_logging_errors;
				return true;
			}
		}
		return false;
	}
}
