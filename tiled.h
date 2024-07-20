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
		FLIP_HORIZONTAL = (1 << 0),
		FLIP_VERTICAL   = (1 << 1),
		FLIP_DIAGONAL   = (1 << 2), // in orthogonal and isometric maps
		FLIP_ROTATE_60  = FLIP_DIAGONAL, // in hexagonal maps
		FLIP_ROTATE_120 = (1 << 3), // in hexagonal maps
	};

	struct Object
	{
		std::string path; // nonempty if object is a template
		std::string name;
		std::string class_;
		Properties properties;
		std::vector<Vector2f> points; // in pixels; relative to position
		const Tile* tile = nullptr; // nonnull if object is a tile
		Vector2f position; // in pixels
		Vector2f size; // in pixels
		entt::entity entity = entt::null;
		ObjectType type = ObjectType::Rectangle;
		uint8_t flip_flags = 0; // only for tile objects
	};

	struct Frame
	{
		unsigned int duration = 0; // in milliseconds
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
		unsigned int id = 0; // index into Tileset::tiles
		unsigned int left = 0; // in pixels
		unsigned int top = 0; // in pixels
		unsigned int width = 0; // in pixels
		unsigned int height = 0; // in pixels
		std::string class_;
		Properties properties;
		std::vector<Object>	objects;
		std::vector<Frame> animation; // nonempty if tile is animated
		std::vector<WangTile> wangtiles; // one for each wangset the tile is part of
		const Tileset* tileset = nullptr;
	};

	struct TileInstance
	{
		const Tile* tile = nullptr;
		uint8_t flip_flags = 0;
	};

	struct WangColor
	{
		std::string name;
		std::string class_;
		Properties properties;
		int tile_id = -1; // Index into Tileset::tiles; -1 if no tile has been chosen to represent this color
		float probability = 0.f;
		Color color;
	};

	struct WangSet
	{
		std::string name;
		std::string class_;
		Properties properties;
		int tile_id = -1; // Index into Tileset::tiles; -1 if no tile has been chosen to represent this set
		std::vector<WangColor> colors;
	};

	struct Tileset
	{
		std::string path;
		std::string image_path;
		std::string name;
		std::string class_;
		Properties properties;
		std::vector<Tile> tiles; // size = tile_count
		std::vector<WangSet> wangsets;
		unsigned int tile_count = 0;
		unsigned int columns = 0;
		unsigned int tile_width = 0; // in pixels
		unsigned int tile_height = 0; // in pixels
		unsigned int spacing = 0; // in pixels
		unsigned int margin = 0; // in pixels
	};

	struct Layer
	{
		std::string name;
		std::string class_;
		Properties properties;
		std::vector<TileInstance> tiles; // nonempty if tile layer; size = width * height
		std::vector<Object> objects; // nonempty if object layer 
		unsigned int width = 0; // in tiles
		unsigned int height = 0; // in tiles
		bool visible = true;
	};

	struct Map
	{
		std::string path;
		std::string name; // filename without extension
		std::string class_;
		Properties properties;
		std::vector<Layer> layers;
		unsigned int width = 0; // in tiles
		unsigned int height = 0; // in tiles
		unsigned int tile_width = 0; // in pixels
		unsigned int tile_height = 0; // in pixels
	};

	// Loads all tilesets (.tsx), then all templates (.tx), then all maps (.tmx).
	void load_assets(const std::string& dir);

	std::span<const Map> get_maps();
	std::span<const Tileset> get_tilesets();
	std::span<const Object> get_templates();

	const Map* find_map_by_name(const std::string& name);
	const Tileset* find_tileset_by_name(const std::string& name);
	const Object* find_object_by_name(const Layer& layer, const std::string& name);
	const Object* find_object_by_name(const Map& map, const std::string& name);
	const Tile* find_tile_by_class(const Tileset& tileset, const std::string& class_);
}

