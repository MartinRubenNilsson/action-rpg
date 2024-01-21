#include "map.h"
#include "defines.h"
#include "tiled.h"
#include "console.h"
#include "audio.h"
#include "math_vectors.h"
#include "ecs.h"
#include "ecs_tiled.h"
#include "ecs_physics.h"
#include "ecs_graphics.h"
#include "ecs_player.h"
#include "ecs_camera.h"
#include "ecs_ai_actions.h"
#include "ui_textbox.h"

namespace map
{
	const float SpawnOptions::DEFAULT = FLT_EPSILON;

	const tiled::Map* _current_map = nullptr;
	const tiled::Map* _next_map = nullptr;
	bool _force_open = false; // if true, open the next map even if it's the same as the current one
	std::string _spawnpoint_entity_name; // entity to spawn the player at when opening the next map

	bool open(const std::string& map_name, bool force)
	{
		_next_map = nullptr;
		for (const tiled::Map& map : tiled::get_maps()) {
			if (map.name == map_name) {
				_next_map = &map;
				break;
			}
		}
		_force_open = force;
		return _next_map;
	}

	void close() {
		_next_map = nullptr;
	}

	void reset()
	{
		_next_map = _current_map;
		_force_open = true;
	}

	entt::entity _spawn(const tiled::Object& object, const SpawnOptions& options)
	{
		if (!_current_map) return entt::null;

		// Attempt to use the object's UID as the entity identifier.
		// If the identifier is already in use, a new one will be generated.
		entt::entity entity = ecs::create(object.entity);
		ecs::emplace_object(entity, &object);

		float x = (options.x != SpawnOptions::DEFAULT) ? options.x : object.position.x * METERS_PER_PIXEL;
		float y = (options.y != SpawnOptions::DEFAULT) ? options.y : object.position.y * METERS_PER_PIXEL;
		float z = (options.z != SpawnOptions::DEFAULT) ? options.z : (float)_current_map->layers.size();
		float w = (options.w != SpawnOptions::DEFAULT) ? options.w : object.size.x * METERS_PER_PIXEL;
		float h = (options.h != SpawnOptions::DEFAULT) ? options.h : object.size.y * METERS_PER_PIXEL;

		if (object.type == tiled::ObjectType::Tile) {
			// All objects are positioned by their top-left corner, except for tiles,
			// which are positioned by their bottom-left corner. This is confusing,
			// so we'll adjust the position here to make it consistent.
			y -= h;
		}

		if (object.type == tiled::ObjectType::Tile) {
			assert(object.tile && "Tile not found.");
			ecs::Tile& tile = ecs::emplace_tile(entity, object.tile);
			tile.position = sf::Vector2f(x, y);
			tile.sorting_layer = ecs::SortingLayer::Objects;

			// LOAD COLLIDERS

			if (!object.tile->objects.empty()) {
				b2BodyDef body_def;
				body_def.type = b2_dynamicBody;
				body_def.fixedRotation = true;
				body_def.position.x = x;
				body_def.position.y = y;
				b2Body* body = ecs::emplace_body(entity, body_def);

				for (const tiled::Object& tile_object : object.tile->objects) {
					float x = tile_object.position.x * METERS_PER_PIXEL;
					float y = tile_object.position.y * METERS_PER_PIXEL;
					float hw = tile_object.size.x * METERS_PER_PIXEL / 2.0f;
					float hh = tile_object.size.y * METERS_PER_PIXEL / 2.0f;
					b2Vec2 center(x + hw, y + hh);

					switch (tile_object.type) {
					case tiled::ObjectType::Rectangle:
					{
						b2PolygonShape shape;
						shape.SetAsBox(hw, hh, center, 0.f);
						body->CreateFixture(&shape, 1.f);
						break;
					}
					case tiled::ObjectType::Ellipse:
					{
						b2CircleShape shape;
						shape.m_p = center;
						shape.m_radius = hw;
						body->CreateFixture(&shape, 1.f);
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
				body->CreateFixture(&fixture_def);
				break;
			}
			}
		}

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
			ecs::AIActionMoveToPlayer action;
			ecs::get_float(entity, "speed", action.speed);
			ecs::emplace_ai_action(entity, action);
			//ecs::destroy_immediately(entity);
		} else if (object.class_ == "camera") {
			ecs::Camera camera;
			camera.view.setCenter(x, y);
			ecs::get_entity(entity, "follow", camera.follow);
			camera.confining_rect = get_bounds();
			ecs::emplace_camera(entity, camera);
		}

		return entity;
	}

	void update()
	{
		if (_next_map == _current_map && !_force_open)
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
		_force_open = false;

		if (!_current_map) {
			audio::stop_all_in_bus();
		} else {
			tiled::get(_current_map->properties, "music", next_music);
			if (current_music != next_music) {
				if (!current_music.empty())
					audio::stop_all("event:/" + current_music);
				if (!next_music.empty())
					audio::play("event:/" + next_music);
			}

			// Spawn objects first. This is because we want to be sure that the
			// object UIDs we get from Tiled are free to use as entity identifiers.
			{
				SpawnOptions options;
				for (size_t z = 0; z < _current_map->layers.size(); ++z) {
					const tiled::Layer& layer = _current_map->layers[z];
					for (const tiled::Object& object : layer.objects) {
						options.z = (float)z;
						_spawn(object, options);
					}
				}
			}

			// Create tile entities second.
			for (const tiled::Layer& layer : _current_map->layers) {
				ecs::SortingLayer sorting_layer = ecs::layer_name_to_sorting_layer(layer.name);
				for (uint32_t tile_y = 0; tile_y < layer.height; tile_y++) {
					for (uint32_t tile_x = 0; tile_x < layer.width; tile_x++) {

						const tiled::Tile* tile = layer.tiles[tile_y * layer.width + tile_x];
						if (!tile) continue;
						float position_x = tile_x * _current_map->tile_width * METERS_PER_PIXEL;
						float position_y = tile_y * _current_map->tile_height * METERS_PER_PIXEL;
						float origin_x = 0.f;
						float origin_y = tile->tileset->tile_height - _current_map->tile_height;

						entt::entity entity = ecs::create();
						ecs::Tile& ecs_tile = ecs::emplace_tile(entity, tile);
						ecs_tile.visible = layer.visible;
						ecs_tile.position = sf::Vector2f(position_x, position_y); // meters
						ecs_tile.origin = sf::Vector2f(origin_x, origin_y); // pixels
						ecs_tile.sorting_layer = sorting_layer;

						// LOAD COLLIDERS

						if (tile->objects.empty())
							continue;

						b2BodyDef body_def;
						body_def.type = b2_staticBody;
						body_def.position.x = position_x;
						body_def.position.y = position_y;
						body_def.fixedRotation = true;
						b2Body* body = ecs::emplace_body(entity, body_def);

						for (const tiled::Object& collider : tile->objects) {
							float cx = collider.position.x * METERS_PER_PIXEL;
							float cy = collider.position.y * METERS_PER_PIXEL;
							switch (collider.type) {
							case tiled::ObjectType::Rectangle:
							{
								float hw = collider.size.x * METERS_PER_PIXEL / 2.0f;
								float hh = collider.size.y * METERS_PER_PIXEL / 2.0f;
								b2Vec2 center(cx + hw, cy + hh);
								b2PolygonShape shape;
								shape.SetAsBox(hw, hh, center, 0.f);
								body->CreateFixture(&shape, 0.0f);
								break;
							}
							case tiled::ObjectType::Ellipse:
							{
								//b2CircleShape shape;
								//shape.m_p = center;
								//shape.m_radius = hw;
								//body->CreateFixture(&shape, 0.0f);
								break;
							}
							case tiled::ObjectType::Polygon:
							{
								size_t count = collider.points.size();
								if (count < 3) {
									console::log_error(
										"Too few points in polygon collider! Got " +
										std::to_string(count) + ", need >= 3.");
								} else if (count < b2_maxPolygonVertices) {
									b2Vec2 points[b2_maxPolygonVertices];
									for (size_t i = 0; i < count; ++i) {
										points[i].x = cx + collider.points[i].x * METERS_PER_PIXEL;
										points[i].y = cy + collider.points[i].y * METERS_PER_PIXEL;
									}
									b2PolygonShape shape;
									shape.Set(points, (int32)count);
									body->CreateFixture(&shape, 0.0f);
								} else {
									auto triangles = triangulate(collider.points);
									for (const auto& triangle : triangles) {
										b2Vec2 points[3];
										for (size_t i = 0; i < 3; ++i) {
											points[i].x = cx + triangle[i].x * METERS_PER_PIXEL;
											points[i].y = cy + triangle[i].y * METERS_PER_PIXEL;
										}
										b2PolygonShape shape;
										shape.Set(points, 3);
										body->CreateFixture(&shape, 0.0f);
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
	}

	std::string get_name() {
		return _current_map ? _current_map->name : "";
	}

	sf::FloatRect get_bounds()
	{
		if (!_current_map) return sf::FloatRect();
		return sf::FloatRect(0.f, 0.f,
			_current_map->width * _current_map->tile_width * METERS_PER_PIXEL,
			_current_map->height * _current_map->tile_height * METERS_PER_PIXEL);
	}

	entt::entity spawn(std::string template_name, const SpawnOptions& options)
	{
		if (!_current_map) return entt::null;
		for (const tiled::Object& template_ : tiled::get_templates())
			if (template_.name == template_name)
				return _spawn(template_, options);
		console::log_error("Failed to spawn template: " + template_name);
		return entt::null;
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
			const tiled::Tile* tile = layer.tiles[y * layer.width + x];
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
