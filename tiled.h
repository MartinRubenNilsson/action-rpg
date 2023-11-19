#pragma once

namespace tiled
{
	struct Property
	{
		std::string name;
		std::variant<std::string, int, float, bool, uint32_t> value;
	};

	struct TileInstance
	{
		uint32_t id = UINT32_MAX; // The local tile ID of the tile within its tileset.
		uint32_t tileset_id = UINT32_MAX;
	};

	enum class ObjectType
	{
		Rectangle,
		Ellipse,
		Point,
		Polygon,
		Polyline,
		Tile,
	};

	struct Object
	{
		uint32_t id = UINT32_MAX;
		ObjectType type = ObjectType::Rectangle;
		std::string name;
		std::string class_;
		sf::Vector2f position; // in pixels
		sf::Vector2f size; // in pixels
		std::vector<Property> properties;
		std::vector<sf::Vector2f> points; // relative to position
		TileInstance tile;
	};

	struct Frame
	{
		uint32_t tile_id = UINT32_MAX; // The local ID of the tile within its tileset.
		uint32_t duration = 0; // in milliseconds
	};

	struct Tile
	{
		std::string class_;
		sf::Sprite sprite;
		std::vector<Property> properties;
		std::vector<Object>	objects;
		std::vector<Frame> animation;
	};

	struct Tileset
	{
		std::string name;
		std::string class_;
		uint32_t tile_width = 0; // in pixels
		uint32_t tile_height = 0; // in pixels
		uint32_t tile_count = 0;
		uint32_t columns = 0;
		uint32_t spacing = 0; // in pixels
		uint32_t margin = 0; // in pixels
		std::vector<Property> properties;
		std::vector<Tile> tiles; // size = tile_count
	};

	enum class LayerType
	{
		Tile,
		Object,
		Image,
		Group,
	};

	struct Layer
	{
		uint32_t index = UINT32_MAX;
		uint32_t parent_index = UINT32_MAX;
		LayerType type = LayerType::Tile;
		std::string name;
		std::string class_;
		uint32_t width = 0; // in tiles
		uint32_t height = 0; // in tiles
		std::vector<Property> properties;
		std::vector<TileInstance> tiles; // size = width * height
		std::vector<Object> objects;
	};

	struct Map
	{
		std::string class_;
		uint32_t width = 0; // in tiles
		uint32_t height = 0; // in tiles
		uint32_t tile_width = 0; // in pixels
		uint32_t tile_height = 0; // in pixels
		std::vector<Property> properties;
		std::vector<Layer> layers;
	};

	void load_assets();

	const Map* find_map(const std::string& filename);
	const Tile* find_tile(TileInstance tile);

	uint32_t get_total_duration(const std::vector<Frame>& animation); // In milliseconds.
}

