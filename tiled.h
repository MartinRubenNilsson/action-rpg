#pragma once

namespace tiled
{
	struct Property
	{
		std::string name;
		std::variant<std::string, int, float, bool, uint32_t> value;
	};

	struct Image
	{
		std::filesystem::path path;
		sf::Texture texture;
	};

	enum class Shape
	{
		Rectangle,
		Ellipse,
		Point,
		Tile,
	};

	struct TileId
	{
		uint16_t tile_id = UINT16_MAX; // The local ID of the tile within its tileset.
		uint16_t tileset_id = UINT16_MAX;
	};

	struct Object
	{
		uint32_t id = 0; // Unique ID of the object, with valid IDs being at least 1.
		std::string name;
		std::string type;
		sf::Vector2f position; // in pixels
		sf::Vector2f size; // in pixels
		Shape shape;
		TileId tile_id;
		std::vector<Property> properties;
	};

	struct Frame
	{
		uint16_t tile_id = UINT16_MAX; // The local ID of the tile within its tileset.
		uint16_t duration = 0; // In milliseconds.
	};

	struct Tile
	{
		TileId id;
		std::string type;
		std::vector<Frame> animation;
		std::vector<Object>	objects;
		std::vector<Property> properties;
	};

	struct Tileset
	{
		std::filesystem::path path;
		std::string name;
		std::string type;
		uint32_t tile_width = 0; // in pixels
		uint32_t tile_height = 0; // in pixels
		uint32_t tile_count = 0;
		uint32_t columns = 0;
		uint32_t spacing = 0; // in pixels
		uint32_t margin = 0; // in pixels
		Image image;
		std::vector<Tile> tiles; // size = tile_count
		std::vector<Property> properties;
	};

	struct Layer
	{
		// TODO
	};

	struct Map
	{
		// TODO
	};

	void load_assets();

	uint16_t get_total_duration(const std::vector<Frame>& animation);
}

