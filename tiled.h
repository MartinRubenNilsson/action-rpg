#pragma once

namespace tiled
{
	struct TileInstance
	{
		uint16_t id = UINT16_MAX; // The local tile ID of the tile within its tileset.
		uint16_t tileset_id = UINT16_MAX;
	};

	struct TilesetInstance
	{
		uint16_t id = UINT16_MAX;
		uint16_t first_gid = UINT16_MAX; // The global tile ID of the first tile in the tileset.
	};

	struct Property
	{
		std::string name;
		std::variant<std::string, int, float, bool, entt::entity> value;
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
		entt::entity id = entt::null;
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
		uint16_t tile_id = UINT16_MAX; // The local ID of the tile within its tileset.
		uint16_t duration = 0; // In milliseconds.
	};

	struct Tile
	{
		std::string class_;
		sf::IntRect rect; // in pixels
		sf::Sprite sprite; // TODO: create in load_assets()
		std::vector<Property> properties;
		std::vector<Object>	objects;
		std::vector<Frame> animation;
	};

	struct Tileset
	{
		std::filesystem::path path;
		std::string name;
		std::string class_;
		uint32_t tile_width = 0; // in pixels
		uint32_t tile_height = 0; // in pixels
		uint32_t tile_count = 0;
		uint32_t columns = 0;
		uint32_t spacing = 0; // in pixels
		uint32_t margin = 0; // in pixels
		std::vector<Property> properties;
		std::filesystem::path image_path;
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
		LayerType type = LayerType::Tile;
		uint32_t parent_index = UINT32_MAX; // index of parent group layer in map.layers
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
		std::vector<TilesetInstance> tilesets;
		std::vector<Layer> layers;
	};

	void load_assets();

	const Map* get_map(const std::filesystem::path& path);
	const Tile* get_tile(TileInstance tile);

	uint16_t get_total_duration(const std::vector<Frame>& animation);
}

