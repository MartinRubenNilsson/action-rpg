#include "stdafx.h"
#include "map_tilegrid.h"
#include "tiled.h"
#include "console.h"

namespace map
{
	struct Tile
	{
		enum State : unsigned char
		{
			UNVISITED,
			OPEN,
			CLOSED,
		};

		sf::Vector2i position{ -1, -1 };
		bool passable = true;
		TerrainType terrains[tiled::WangTile::COUNT] = {};
		sf::Vector2i parent{ -1, -1 };
		float g = FLT_MAX;
		float h = FLT_MAX;
		State state = UNVISITED;
	};

	void _reset_a_star_state(Tile& tile)
	{
		tile.parent = sf::Vector2i(-1, -1);
		tile.g = FLT_MAX;
		tile.h = FLT_MAX;
		tile.state = Tile::UNVISITED;
	}

	class TilePriorityQueue
	{
		struct CompareByF
		{
			bool operator()(const Tile* a, const Tile* b) const {
				return a->g + a->h > b->g + b->h;
			}
		};

		std::vector<Tile*> _queue;

	public:
		void push(Tile* tile)
		{
			_queue.push_back(tile);
			std::push_heap(_queue.begin(), _queue.end(), CompareByF());
		}

		void pop()
		{
			std::pop_heap(_queue.begin(), _queue.end(), CompareByF());
			_queue.pop_back();
		}

		Tile* top() {
			return _queue.front();
		}

		bool empty() const {
			return _queue.empty();
		}

		void clear() {
			_queue.clear();
		}
	};

	struct TileGrid
	{
		sf::Vector2i size; // in tiles
		sf::Vector2i tile_size; // in pixels
		std::vector<Tile> tiles; // tiles.size() == size.x * size.y
		TilePriorityQueue open_tiles;
	};

	const sf::Vector2i _ALLOWED_MOVEMENT_DIRECTIONS[] =
	{
		// The order of these directions has been chosen to minimize cache misses
		// when iterating over the array while pathfinding, so don't change it.
		sf::Vector2i(-1, -1),
		sf::Vector2i( 0, -1),
		sf::Vector2i( 1, -1),
		sf::Vector2i(-1,  0),
		sf::Vector2i( 1,  0),
		sf::Vector2i(-1,  1),
		sf::Vector2i( 0,  1),
		sf::Vector2i( 1,  1),
	};

	TileGrid _grid;

	Tile& _get_tile(const sf::Vector2i& position) {
		return _grid.tiles[position.x + position.y * _grid.size.x];
	}

	Tile* _try_get_tile(const sf::Vector2i& position)
	{
		if (position.x < 0) return nullptr;
		if (position.y < 0) return nullptr;
		if (position.x >= _grid.size.x) return nullptr;
		if (position.y >= _grid.size.y) return nullptr;
		return &_grid.tiles[position.x + position.y * _grid.size.x];
	}

	TerrainType _terrain_name_to_type(const std::string& name)
	{
		if (name == "dirt")          return TerrainType::Dirt;
		if (name == "light grass")   return TerrainType::LightGrass;
		if (name == "dark grass")    return TerrainType::DarkGrass;
		if (name == "cobblestone")   return TerrainType::Cobblestone;
		if (name == "shallow water") return TerrainType::ShallowWater;
		if (name == "deep water")    return TerrainType::DeepWater;
		console::log_error("Unknown terrain type: " + name);
		return TerrainType::None;
	}

	void recreate_tilegrid(const tiled::Map& map)
	{
		_grid.size = sf::Vector2i(map.width, map.height);
		_grid.tile_size = sf::Vector2i(map.tile_width, map.tile_height);
		_grid.tiles.resize(_grid.size.x * _grid.size.y);
		_grid.open_tiles.clear();

		for (int y = 0; y < _grid.size.x; ++y) {
			for (int x = 0; x < _grid.size.y; ++x) {
				int index = x + y * _grid.size.x;
				_grid.tiles[index].position = sf::Vector2i(x, y);
				_grid.tiles[index].passable = true;
			}
		}

		for (const tiled::Layer& layer : map.layers) {
			if (layer.tiles.size() != _grid.tiles.size())
				continue;
			if (layer.name == "Collision") {
				for (int y = 0; y < _grid.size.x; ++y) {
					for (int x = 0; x < _grid.size.y; ++x) {
						int index = x + y * _grid.size.x;
						_grid.tiles[index].passable = (layer.tiles[index].first == nullptr);
					}
				}
			} else if (layer.name == "Under Sprite 1") {
				for (int y = 0; y < _grid.size.x; ++y) {
					for (int x = 0; x < _grid.size.y; ++x) {
						int index = x + y * _grid.size.x;
						const tiled::Tile* layer_tile = layer.tiles[index].first;
						if (!layer_tile) continue;
						if (layer_tile->wangtiles.empty()) continue;
						const tiled::WangTile& wangtile = layer_tile->wangtiles[0];
						Tile& grid_tile = _grid.tiles[index];
						for (int i = 0; i < tiled::WangTile::COUNT; ++i) {
							if (!wangtile.wangcolors[i]) continue;
							grid_tile.terrains[i] = _terrain_name_to_type(wangtile.wangcolors[i]->name);
						}
					}
				}
			}
		}
	}

	sf::Vector2i get_grid_size() {
		return _grid.size;
	}

	sf::Vector2i get_tile_size() {
		return _grid.tile_size;
	}

	TerrainType get_terrain_type_at(const sf::Vector2f& world_pos)
	{
		sf::Vector2f pos = world_pos / sf::Vector2f(_grid.tile_size);
		sf::Vector2i floored_pos((int)floor(pos.x), (int)floor(pos.y));
		Tile* tile = _try_get_tile(floored_pos);
		if (!tile) return TerrainType::None;
		const bool left = (pos.x - floored_pos.x) < 0.5f;
		const bool top = (pos.y - floored_pos.y) < 0.5f;
		const int corner =
			top ? (left ? tiled::WangTile::TOP_LEFT : tiled::WangTile::TOP_RIGHT)
			: (left ? tiled::WangTile::BOTTOM_LEFT : tiled::WangTile::BOTTOM_RIGHT);
		return tile->terrains[corner];
	}

	int _manhattan_distance(const sf::Vector2i& a, const sf::Vector2i& b) {
		return std::abs(b.x - a.x) + std::abs(b.y - a.y);
	}

	float _euclidean_distance(const sf::Vector2i& a, const sf::Vector2i& b)
	{
		int dx = b.x - a.x;
		int dy = b.y - a.y;
		return std::sqrt((float)(dx * dx + dy * dy));
	}

	float _euclidean_distance_on_grid(const sf::Vector2i& a, const sf::Vector2i& b)
	{
		constexpr float SQRT_2 = 1.41421356237f;
		int dx = std::abs(b.x - a.x);
		int dy = std::abs(b.y - a.y);
		return std::abs(dx - dy) + std::min(dx, dy) * SQRT_2;
	}

	std::vector<sf::Vector2i> pathfind(const sf::Vector2i& start, const sf::Vector2i& end)
	{
		Tile* start_tile = _try_get_tile(start);
		if (!start_tile || !start_tile->passable) return {};
		Tile* end_tile = _try_get_tile(end);
		if (!end_tile || !end_tile->passable) return {};

		if (start_tile == end_tile) return { start };

		for (Tile& tile : _grid.tiles)
			_reset_a_star_state(tile);

		start_tile->g = 0.f;
		start_tile->h = _euclidean_distance_on_grid(start, end);
		start_tile->state = Tile::OPEN;

		_grid.open_tiles.clear();
		_grid.open_tiles.push(start_tile);

		bool path_found = false;
		while (!_grid.open_tiles.empty()) {

			Tile* current_tile = _grid.open_tiles.top();
			if (current_tile == end_tile) {
				path_found = true;
				break;
			}

			_grid.open_tiles.pop();
			current_tile->state = Tile::CLOSED;

			const sf::Vector2i current_pos = current_tile->position;
			const float current_g = current_tile->g;

			for (const sf::Vector2i& direction : _ALLOWED_MOVEMENT_DIRECTIONS) {

				sf::Vector2i neighbor_pos = current_pos + direction;
				Tile* neighbor_tile = _try_get_tile(neighbor_pos);
				if (!neighbor_tile) continue;
				if (!neighbor_tile->passable) continue;
				if (neighbor_tile->state == Tile::CLOSED) continue;

				float tentative_neighbor_g = current_g +
					_euclidean_distance_on_grid(current_pos, neighbor_pos);
				if (tentative_neighbor_g >= neighbor_tile->g) continue;

				neighbor_tile->parent = current_pos;
				neighbor_tile->g = tentative_neighbor_g;
				neighbor_tile->h = _euclidean_distance_on_grid(neighbor_pos, end);
				if (neighbor_tile->state == Tile::OPEN) continue;

				neighbor_tile->state = Tile::OPEN;
				_grid.open_tiles.push(neighbor_tile);
			}
		}

		if (!path_found) return {};

		std::vector<sf::Vector2i> path;
		for (Tile* tile = end_tile; tile; tile = _try_get_tile(tile->parent))
			path.push_back(tile->position);
		std::reverse(path.begin(), path.end());

		return path;
	}

	std::vector<sf::Vector2f> pathfind(const sf::Vector2f& start, const sf::Vector2f& end)
	{
		sf::Vector2f tile_size = sf::Vector2f(_grid.tile_size);
		sf::Vector2i start_i(start / tile_size);
		sf::Vector2i end_i(end / tile_size);

		std::vector<sf::Vector2i> path = pathfind(start_i, end_i);

		std::vector<sf::Vector2f> pathf;
		pathf.resize(path.size());
		for (size_t i = 0; i < path.size(); ++i)
			pathf[i] = sf::Vector2f(path[i]) * tile_size + tile_size * 0.5f;
		return pathf;
	}
}