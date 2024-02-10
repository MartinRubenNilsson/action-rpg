#include "map.h"
#include "tiled.h"
#include "console.h"
#include "audio.h"
#include "math_vectors.h"
#include "ecs.h"
#include "ecs_tiled.h"
#include "ecs_physics.h"
#include "ecs_physics_filters.h"
#include "ecs_tile.h"
#include "ecs_player.h"
#include "ecs_camera.h"
#include "ecs_ai.h"
#include "ui_textbox.h"

namespace map
{
	enum class Request
	{
		None,
		Open,
		Close,
		Reset,
	};

	const tiled::Map* _map = nullptr;
	Request _request = Request::None;
	std::string _name_of_map_to_open;

	void open(const std::string& map_name, bool reset_if_open)
	{
		if (_map && _map->name == map_name) {
			_request = reset_if_open ? Request::Reset : Request::None;
		} else {
			_request = Request::Open;
			_name_of_map_to_open = map_name;
		}
	}

	void close() {
		_request = Request::Close;
	}

	void reset() {
		_request = Request::Reset;
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
		case Request::None:
			return;
		case Request::Open:
			next_map = find_map(_name_of_map_to_open);
			if (!next_map)
				console::log_error("Map not found: " + _name_of_map_to_open);
			break;
		case Request::Close:
			break;
		case Request::Reset:
			next_map = _map;
			break;
		}

		_request = Request::None;
		_name_of_map_to_open.clear();

		// CLOSE MAP

		if (_map) {
			ecs::clear();
			ui::close_all_textboxes();
			audio::stop_all_in_bus(audio::BUS_SOUND);
		}

		// OPEN MAP

		_map = next_map;
		if (!_map) {
			audio::stop_all_in_bus();
			return;
		}

		// PLAY MUSIC
		{
			std::string music;
			if (tiled::get(_map->properties, "music", music)) {
				std::string event_path = "event:/" + music;
				if (!audio::is_any_playing(event_path)) {
					audio::stop_all_in_bus(audio::BUS_MUSIC);
					audio::play(event_path);
				}
			}
		}

		// Create object entities first. This is because we want to be sure that the
		// object UIDs we get from Tiled are free to use as entity identifiers.
		for (const tiled::Layer& layer : _map->layers) {
			if (layer.objects.empty()) continue;
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

				// CLASS-SPECIFIC COMPONENTS

				if (object.class_ == "player") {
					ecs::emplace_player(entity);

					ecs::Camera camera;
					camera.follow = entity;
					camera.confining_rect = get_world_bounds();
					ecs::emplace_camera(entity, camera);
					ecs::activate_camera(entity, true);

				} else if (object.class_ == "slime") {
					ecs::emplace_ai(entity, ecs::AiType::Slime);
				} else if (object.class_ == "camera") {
					ecs::Camera camera;
					camera.view.setCenter(x, y);
					ecs::get_entity(entity, "follow", camera.follow);
					camera.confining_rect = get_world_bounds();
					ecs::emplace_camera(entity, camera);
				} else if (object.class_ == "audio_source") {
					std::string event_name;
					if (tiled::get(object.properties, "event", event_name))
						audio::play_at_position("event:/" + event_name, sf::Vector2f(x, y));
				}
			}
		}

		// Create tile entities second.
		for (const tiled::Layer& layer : _map->layers) {
			if (layer.tiles.empty()) continue;
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

	sf::Vector2f grid_to_world(const sf::Vector2i& gridPos) {
		float worldX = static_cast<float>(gridPos.x * _map->tile_width) + _map->tile_width / 2.0f;
		float worldY = static_cast<float>(gridPos.y * _map->tile_height) + _map->tile_height / 2.0f;
		return sf::Vector2f(worldX, worldY);
	}

	// Cast sf::Vector2i to uint32_t safely, ensuring no negative values
	uint32_t safe_cast_to_uint32(int value) {
		if (value < 0) {
			// Handle the error or perform an appropriate action when value is negative
			// For example, log an error, throw an exception, or use a default positive value
			throw std::runtime_error("Negative value encountered when casting to uint32_t");
		}
		return static_cast<uint32_t>(value);
	}

	std::vector<sf::Vector2i> get_neighbors(const sf::Vector2i& pos, const std::vector<std::vector<bool>>& path_finding_grid, const tiled::Layer* collision_layer) {
		std::vector<sf::Vector2i> neighbors;

		// Directions: up, down, left, right, and diagonals
		std::vector<sf::Vector2i> directions = {
			{0, -1}, {0, 1}, {-1, 0}, {1, 0},
			{-1, -1}, {-1, 1}, {1, -1}, {1, 1}
		};

		for (const auto& dir : directions) {
			sf::Vector2i neighbor_pos = pos + dir;
			uint32_t neighbor_pos_x = static_cast<uint32_t>(neighbor_pos.x);
			uint32_t neighbor_pos_y = static_cast<uint32_t>(neighbor_pos.y);

			// Check if within grid bounds
			if (neighbor_pos_x >= 0 && neighbor_pos_x < collision_layer->width &&
				neighbor_pos_y >= 0 && neighbor_pos_y < collision_layer->height) {
				// Check if the tile is passable
				if (path_finding_grid[neighbor_pos.y][neighbor_pos.x]) {
					neighbors.push_back(neighbor_pos);
				}
			}
		}

		return neighbors;
	}

	float distance_between(const sf::Vector2i& start, const sf::Vector2i& end) {
		int dx = std::abs(end.x - start.x);
		int dy = std::abs(end.y - start.y);

		// Cost of straight moves
		int straight = std::abs(dx - dy);

		// Cost of diagonal moves
		int diagonal = std::min(dx, dy);

		// Straight move cost: 1, diagonal move cost: sqrt(2)
		return straight + diagonal * std::sqrt(2.0f);
	}

	float heuristic(const sf::Vector2i& start, const sf::Vector2i& end) {
		float dx = static_cast<float>(end.x - start.x);
		float dy = static_cast<float>(end.y - start.y);

		// Euclidean distance
		return sqrt(dx * dx + dy * dy);
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
		std::vector<std::vector<bool>> path_finding_grid(collision_layer->height, std::vector<bool>(collision_layer->width, true)); // true indicates passable

		// Loop through each tile in the collision layer
		for (uint32_t y = 0; y < collision_layer->height; ++y) {
			for (uint32_t x = 0; x < collision_layer->width; ++x) {
				const tiled::Tile* tile = collision_layer->tiles[y * collision_layer->width + x].first;

				if (tile) {
					// Tile is nonempty = impassable
					path_finding_grid[y][x] = false; // false indicates impassable
				}
				else {
					// Tile is empty = passable
					path_finding_grid[y][x] = true; // true indicates passable
				}
			}
		}

		// Convert world coordinates to grid coordinates and compute the start and end tile indices.
		sf::Vector2i grid_start = world_to_grid(start);
		sf::Vector2i grid_end = world_to_grid(end);

		uint32_t grid_start_x = static_cast<uint32_t>(grid_start.x);
		uint32_t grid_start_y = static_cast<uint32_t>(grid_start.y);

		uint32_t grid_end_x = static_cast<uint32_t>(grid_end.x);
		uint32_t grid_end_y = static_cast<uint32_t>(grid_end.y);

		// If the start or end tile is out of bounds, we can't pathfind.
		if (grid_start_x < 0 || grid_start_x >= collision_layer->width ||
			grid_start_y < 0 || grid_start_y >= collision_layer->height ||
			grid_end_x < 0 || grid_end_x >= collision_layer->width ||
			grid_end_y < 0 || grid_end_y >= collision_layer->height) {
			// Start or end point is out of bounds, return empty path or handle error
			return {};
		}

		// If the start or end tile is impassable, we can't pathfind.
		if (!path_finding_grid[grid_start.y][grid_start.x] || !path_finding_grid[grid_end.y][grid_end.x]) {
			// Start or end point is impassable, return empty path or handle error
			return {};
		}

		struct AStarNode
		{
			// NOTE TO TIM: you might want to change the members below
			// to better suit your needs. i just made copilot generate this.

			sf::Vector2i position;   // tile position in grid
			sf::Vector2i parent;     // parent tile position in grid
			float g = 0.f;           // cost from start to this tile
			float h = 0.f;           // estimated cost from this tile to end
			float f = 0.f;           // g + h
		};

		// Pathfind using A* algorithm
		// Initialize start and end nodes
		AStarNode start_node;
		start_node.position = grid_start;
		start_node.g = 0.f;
		start_node.h = heuristic(grid_start, grid_end);
		start_node.f = start_node.g + start_node.h;

		AStarNode end_node;
		end_node.position = grid_end;

		std::vector<std::vector<AStarNode>> all_nodes(collision_layer->height, std::vector<AStarNode>(collision_layer->width));

		struct CompareNodes
		{
			bool operator()(const sf::Vector2i& a, const sf::Vector2i& b) const
			{
				return a.x < b.x || (a.x == b.x && a.y < b.y);
			}
		};

		// Open list and closed list
		std::set<sf::Vector2i, CompareNodes> open_list;
		std::set<sf::Vector2i, CompareNodes> closed_list;
		open_list.insert(start_node.position);

		all_nodes[grid_start_y][grid_start_x] = start_node;

		while (!open_list.empty()) {
			// Find the node with the lowest f score
			sf::Vector2i current_pos = *open_list.begin();
			AStarNode& current_node = all_nodes[current_pos.y][current_pos.x];
			for (const auto& pos : open_list) {
				if (all_nodes[pos.y][pos.x].f < current_node.f) {
					current_pos = pos;
					current_node = all_nodes[pos.y][pos.x];
				}
			}

			// Move current node from open to closed list
			open_list.erase(current_pos);
			closed_list.insert(current_pos);

			// Process each neighbor of the current node
			for (const auto& neighbor_pos : get_neighbors(current_pos, path_finding_grid, collision_layer)) {
				uint32_t neighbor_pos_x = static_cast<uint32_t>(neighbor_pos.x);
				uint32_t neighbor_pos_y = static_cast<uint32_t>(neighbor_pos.y);

				if (closed_list.find(neighbor_pos) != closed_list.end()) continue; // Already evaluated

				float tentative_g_score = current_node.g + distance_between(current_pos, neighbor_pos); // or 1 if uniform cost

				if (open_list.find(neighbor_pos) == open_list.end()) { // Discover a new node
					open_list.insert(neighbor_pos);
				}
				else if (tentative_g_score >= all_nodes[neighbor_pos.y][neighbor_pos.x].g) {
					continue; // Not a better path
				}

				// This is the best path until now. Record it
				all_nodes[neighbor_pos_y][neighbor_pos_x].parent = current_pos;
				all_nodes[neighbor_pos_y][neighbor_pos_x].g = tentative_g_score;
				all_nodes[neighbor_pos_y][neighbor_pos_x].h = heuristic(neighbor_pos, grid_end);
				all_nodes[neighbor_pos_y][neighbor_pos_x].f = all_nodes[neighbor_pos_y][neighbor_pos_x].g + all_nodes[neighbor_pos_y][neighbor_pos_x].h;
			}
		}

		// Convert back to world coordinates. The world position of a tile is the center of the tile.
		std::vector<sf::Vector2f> path;

		// Backtrack from the end node to the start node
		sf::Vector2i current_pos = grid_end;
		while (current_pos != grid_start) {
			// Check bounds manually, not using find
			if (current_pos.y < 0 || current_pos.y >= all_nodes.size() ||
				current_pos.x < 0 || current_pos.x >= all_nodes[current_pos.y].size()) {
				// If the current position is out of bounds, something went wrong
				return {}; // Return an empty path or handle the error
			}

			// Add the current position to the path
			path.push_back(grid_to_world(current_pos));

			// Move to the parent of the current node
			current_pos = all_nodes[current_pos.y][current_pos.x].parent; // Access using y for row, then x for column
		}


		// Add the start position to the path
		path.push_back(grid_to_world(grid_start));

		// Reverse the path so it goes from start to end
		std::reverse(path.begin(), path.end());

		//return { start, end };
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
