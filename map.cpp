#include "map.h"
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
	enum class Event
	{
		None,
		Open,
		Close,
		Reset,
	};

	const tiled::Map* _map = nullptr;
	Event _request = Event::None;
	std::string _name_of_map_to_open;

	void open(const std::string& map_name, bool reset_if_open)
	{
		if (_map && _map->name == map_name) {
			_request = reset_if_open ? Event::Reset : Event::None;
		} else {
			_request = Event::Open;
			_name_of_map_to_open = map_name;
		}
	}

	void close() {
		_request = Event::Close;
	}

	void reset() {
		_request = Event::Reset;
	}

	const tiled::Map* find_map(const std::string& map_name)
	{
		for (const tiled::Map& map : tiled::get_maps())
			if (map.name == map_name)
				return &map;
		return nullptr;
	}

	void update()
	{
		const tiled::Map* next_map = nullptr;

		switch (_request) {
		case Event::None:
			return;
		case Event::Open:
			next_map = find_map(_name_of_map_to_open);
			if (!next_map)
				console::log_error("Map not found: " + _name_of_map_to_open);
			break;
		case Event::Close:
			break;
		case Event::Reset:
			next_map = _map;
			break;
		}

		_request = Event::None;
		_name_of_map_to_open.clear();

		std::string current_music;
		std::string next_music;

		// CLOSE MAP

		if (_map) {
			tiled::get(_map->properties, "music", current_music);
			ecs::clear();
			ui::close_all_textboxes();
		}

		// OPEN MAP

		_map = next_map;
		if (!_map) {
			audio::stop_all_in_bus();
			return;
		}

		tiled::get(_map->properties, "music", next_music);
		if (current_music != next_music) {
			if (!current_music.empty())
				audio::stop_all("event:/" + current_music);
			if (!next_music.empty())
				audio::play("event:/" + next_music);
		}

		// Create object entities first. This is because we want to be sure that the
		// object UIDs we get from Tiled are free to use as entity identifiers.
		for (const tiled::Layer& layer : _map->layers) {
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
					camera.confining_rect = get_world_bounds();
					ecs::emplace_camera(entity, camera);
					ecs::activate_camera(entity, true);

				} else if (object.class_ == "slime") {
					ecs::emplace_slime_animation_controller(entity);
					ecs::emplace_ai(entity, ecs::AiType::Slime);
				} else if (object.class_ == "camera") {
					ecs::Camera camera;
					camera.view.setCenter(x, y);
					ecs::get_entity(entity, "follow", camera.follow);
					camera.confining_rect = get_world_bounds();
					ecs::emplace_camera(entity, camera);
				}
			}
		}

		// Create tile entities second.
		for (const tiled::Layer& layer : _map->layers) {
			ecs::SortingLayer sorting_layer = ecs::layer_name_to_sorting_layer(layer.name);
			for (uint32_t tile_y = 0; tile_y < layer.height; tile_y++) {
				for (uint32_t tile_x = 0; tile_x < layer.width; tile_x++) {

					const auto [tile, flip_flags] = layer.tiles[tile_y * layer.width + tile_x];
					if (!tile) continue;

					float position_x = (float)tile_x * _map->tile_width;
					float position_y = (float)tile_y * _map->tile_height;
					float pivot_x = 0.f;
					float pivot_y = (float)(tile->tileset->tile_height - _map->tile_height);
					float sorting_pivot_x = tile->tileset->tile_width / 2.f;
					float sorting_pivot_y = tile->tileset->tile_height - _map->tile_height / 2.f;

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
		return _map ? _map->name : "";
	}

	sf::IntRect get_tile_bounds()
	{
		if (!_map) return sf::IntRect();
		return sf::IntRect(0, 0, (int)_map->width, (int)_map->height);
	}

	sf::FloatRect get_world_bounds()
	{
		if (!_map) return sf::FloatRect();
		return sf::FloatRect(0.f, 0.f,
			(float)_map->width * _map->tile_width,
			(float)_map->height * _map->tile_height);
	}

	sf::Vector2i world_to_grid(const sf::Vector2f& worldPos) {
		if (!_map) return sf::Vector2i(-1, -1); // Return invalid coordinate if no map loaded

		int gridX = static_cast<int>(floor(worldPos.x / _map->tile_width));
		int gridY = static_cast<int>(floor(worldPos.y / _map->tile_height));

		return sf::Vector2i(gridX, gridY);
	}

	std::vector<sf::Vector2f> pathfind(const sf::Vector2f& start, const sf::Vector2f& end)
	{
		// Uses the A* algorithm to find a path from start to end.
		// Returns a vector of points that the entity should follow.
		// If no path is found, an empty vector is returned.
		// The tile layer named "Collision" is used for pathfinding;
		// if a tile is present in this layer, it is considered impassable.

		// If there is no current map, we can't pathfind.
		if (!_map) return {};

		// Find the collision layer.
		const tiled::Layer* collision_layer = nullptr;
		for (const tiled::Layer& layer : _map->layers) {
			if (layer.name == "Collision") {
				collision_layer = &layer;
				break;
			}
		}

		// If there is no collision layer, we can't pathfind.
		if (!collision_layer) return {};


		// NOTES TO TIM:
		collision_layer->width; // number of tiles in x direction
		collision_layer->height; // number of tiles in y direction
		uint32_t x = 0;
		uint32_t y = 0;

		// Create a grid of booleans indicating which tiles are passable.
		std::vector<std::vector<bool>> pathfindingGrid(collision_layer->height, std::vector<bool>(collision_layer->width, true)); // true indicates passable

		// Loop through each tile in the collision layer
		for (uint32_t y = 0; y < collision_layer->height; ++y) {
			for (uint32_t x = 0; x < collision_layer->width; ++x) {
				const tiled::Tile* tile = collision_layer->tiles[y * collision_layer->width + x].first;

				if (tile) {
					// Tile is nonempty = impassable
					pathfindingGrid[y][x] = false; // false indicates impassable
				}
				else {
					// Tile is empty = passable
					pathfindingGrid[y][x] = true; // true indicates passable
				}
			}
		}

		// Convert world coordinates to grid coordinates and compute the start and end tile indices.
		sf::Vector2i grid_start = world_to_grid(start);
		sf::Vector2i grid_end = world_to_grid(end);

		// If the start or end tile is out of bounds, we can't pathfind.
		if (grid_start.x < 0 || grid_start.x >= collision_layer->width ||
			grid_start.y < 0 || grid_start.y >= collision_layer->height ||
			grid_end.x < 0 || grid_end.x >= collision_layer->width ||
			grid_end.y < 0 || grid_end.y >= collision_layer->height) {
			// Start or end point is out of bounds, return empty path or handle error
			return {};
		}

		// If the start or end tile is impassable, we can't pathfind.
		if (!pathfindingGrid[grid_start.y][grid_start.x] || !pathfindingGrid[grid_end.y][grid_end.x]) {
			// Start or end point is impassable, return empty path or handle error
			return {};
		}

		struct AStarNode
		{
			enum State
			{
				UNVISITED,
				OPEN,
				CLOSED,
			};

			// NOTE TO TIM: you might want to change the members below
			// to better suit your needs. i just made copilot generate this.

			sf::Vector2i position;   // tile position in grid
			sf::Vector2i parent;     // parent tile position in grid
			float g = 0.f;           // cost from start to this tile
			float h = 0.f;           // estimated cost from this tile to end
			float f = 0.f;           // g + h
			State state = UNVISITED; // unvisited, open, or closed
		};

		// Pathfind using A* algorithm
		// TODO

		// Convert back to world coordinates. The world position of a tile is the center of the tile.
		// TODO
		
		std::vector<sf::Vector2f> path;
		// (Martin) I've added these two push_back:s just so the debug draw actually draws something.
		// You should remove them once you've implemented the pathfinding.
		path.push_back(start);
		path.push_back(end);
		return path;
	}

	bool play_footstep_sound_at(const sf::Vector2f& position)
	{
		if (!_map) return false;
		if (position.x < 0.f || position.y < 0.f) return false;
		const uint32_t x = (uint32_t)position.x;
		const uint32_t y = (uint32_t)position.y;
		const bool left = (position.x - (float)x) < 0.5f;
		const bool top  = (position.y - (float)y) < 0.5f;
		const int corner =
			top ? (left ? tiled::WangTile::TOP_LEFT    : tiled::WangTile::TOP_RIGHT)
			    : (left ? tiled::WangTile::BOTTOM_LEFT : tiled::WangTile::BOTTOM_RIGHT);
		for (const tiled::Layer& layer : std::ranges::reverse_view(_map->layers)) {
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
