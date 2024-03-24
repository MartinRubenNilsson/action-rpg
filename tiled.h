#pragma once
#include "properties.h"

namespace tiled
{
	enum class ObjectType
	{
		Rectangle,
		Ellipse,
		Point,
		Polygon, // Object::points is nonempty
		Polyline, // Object::points is nonempty
		Tile, // Object::tile is nonnull
	};

	struct Tile;

	enum FlipFlags : uint8_t
	{
		FLIP_NONE       = 0,
		FLIP_HORIZONTAL = 1 << 0,
		FLIP_VERTICAL   = 1 << 1,
		FLIP_DIAGONAL   = 1 << 2, // in orthogonal and isometric maps
		FLIP_ROTATE_60  = FLIP_DIAGONAL, // in hexagonal maps
		FLIP_ROTATE_120 = 1 << 3, // in hexagonal maps
	};

	struct Object
	{
		std::filesystem::path path; // nonempty if object is a template
		std::string name;
		std::string class_;
		Properties properties;
		std::vector<sf::Vector2f> points; // in pixels; relative to position
		const Tile* tile = nullptr; // nonnull if object is a tile
		sf::Vector2f position; // in pixels
		sf::Vector2f size; // in pixels
		entt::entity entity = entt::null;
		ObjectType type = ObjectType::Rectangle;
		uint8_t flip_flags = FLIP_NONE; // only for tile objects
	};

	struct Frame
	{
		uint32_t duration = 0; // in milliseconds
		const Tile* tile = nullptr;
	};
	
	struct WangColor;

	struct WangTile
	{
		enum // Enumerates the edges and corners of the tile.
		{
			TOP = 0,	  // top edge
			TOP_RIGHT,	  // top-right corner
			RIGHT,		  // right edge
			BOTTOM_RIGHT, // bottom-right corner
			BOTTOM,		  // bottom edge
			BOTTOM_LEFT,  // bottom-left corner
			LEFT,		  // left edge
			TOP_LEFT,	  // top-left corner
			COUNT
		};

		const WangColor* wangcolors[COUNT] = {}; // null if uncolored
	};

	struct Tileset;

	struct Tile
	{
		uint32_t id = 0; // index into Tileset::tiles
		sf::IntRect image_rect; // in pixels
		std::string class_;
		Properties properties;
		std::vector<Object>	objects;
		std::vector<Frame> animation; // nonempty if tile is animated
		std::vector<WangTile> wangtiles; // one for each wangset the tile is part of
		const Tileset* tileset = nullptr;
	};

	struct WangColor
	{
		std::string name;
		std::string class_;
		Properties properties;
		const Tile* tile = nullptr; // the tile representing this color; can be null
		float probability = 0.f;
		sf::Color color;
	};

	struct WangSet
	{
		std::string name;
		std::string class_;
		Properties properties;
		const Tile* tile = nullptr; // the tile representing this set; can be null
		std::vector<WangColor> colors;
	};

	struct Tileset
	{
		std::filesystem::path path;
		std::filesystem::path image_path;
		std::shared_ptr<sf::Texture> image;
		std::string name;
		std::string class_;
		Properties properties;
		std::vector<Tile> tiles; // size = tile_count
		std::vector<WangSet> wangsets;
		uint32_t tile_count = 0;
		uint32_t columns = 0;
		uint32_t tile_width = 0; // in pixels
		uint32_t tile_height = 0; // in pixels
		uint32_t spacing = 0; // in pixels
		uint32_t margin = 0; // in pixels

		const Tile* find_tile_by_class(const std::string& class_) const;
	};

	struct Layer
	{
		std::string name;
		std::string class_;
		Properties properties;
		std::vector<std::pair<const Tile*, FlipFlags>> tiles; // nonempty if tile layer; size = width * height
		std::vector<Object> objects; // nonempty if object layer 
		uint32_t width = 0; // in tiles
		uint32_t height = 0; // in tiles
		bool visible = true;
	};

	struct Map
	{
		std::filesystem::path path;
		std::string name; // filename without extension
		std::string class_;
		Properties properties;
		std::vector<Layer> layers;
		uint32_t width = 0; // in tiles
		uint32_t height = 0; // in tiles
		uint32_t tile_width = 0; // in pixels
		uint32_t tile_height = 0; // in pixels
	};

	// Loads all tilesets (.tsx), then all templates (.tx), then all maps (.tmx).
	void load_assets(const std::filesystem::path& dir);
	void unload_assets();

	const std::vector<Tileset>& get_tilesets();
	const std::vector<Object>& get_templates();
	const std::vector<Map>& get_maps();

	const Tileset* find_tileset_by_name(const std::string& name);
}

