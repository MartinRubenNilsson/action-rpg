#include "stdafx.h"
#include "map_tilegrid.h"
#include "tiled.h"

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
		sf::Vector2i parent{ -1, -1 };
		float g = FLT_MAX;
		float h = FLT_MAX;
		bool passable = true;
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

	void recreate_tilegrid(const tiled::Map& map)
	{
		_grid.size = sf::Vector2i(map.width, map.height);
		_grid.tile_size = sf::Vector2i(map.tile_width, map.tile_height);
		_grid.tiles.resize(_grid.size.x * _grid.size.y);
		_grid.open_tiles.clear();

		for (const tiled::Layer& layer : map.layers) {
			if (layer.name != "Collision") continue;
			for (int y = 0; y < _grid.size.x; ++y) {
				for (int x = 0; x < _grid.size.y; ++x) {
					int index = x + y * _grid.size.x;
					_grid.tiles[index].position = sf::Vector2i(x, y);
					_grid.tiles[index].passable = (layer.tiles[index].first == nullptr);
				}
			}
			break;
		}
	}

	sf::Vector2i get_tilegrid_size() {
		return _grid.size;
	}

	sf::Vector2i get_tilegrid_tile_size() {
		return _grid.tile_size;
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