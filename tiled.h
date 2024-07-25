#pragma once
#include "properties.h"

namespace tiled
{
	struct Tile;
	struct WangColor;
	struct Tileset;

	enum class ObjectType
	{
		Rectangle,
		Ellipse,
		Point,
		Polygon, // Object::points is nonempty
		Polyline, // Object::points is nonempty
		Tile, // Object::tile is nonnull
	};

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
		const Tile* tile = nullptr; // nonnull if object is a tile; TODO: replace with tile_id
		Vector2f position; // in pixels
		Vector2f size; // in pixels
		entt::entity entity = entt::null;
		ObjectType type = ObjectType::Rectangle;
		uint8_t flip_flags = 0; // only for tile objects
	};

	struct Frame
	{
		unsigned int duration = 0; // in milliseconds
		unsigned int tile_id = 0; // Index into Tileset::tiles
	};

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

	struct Tile
	{
		Handle<Tileset> tileset;
		unsigned int id = 0; // Index into Tileset::tiles
		unsigned int left = 0; // in pixels
		unsigned int top = 0; // in pixels
		unsigned int width = 0; // in pixels
		unsigned int height = 0; // in pixels
		std::string class_;
		Properties properties;
		std::vector<Object>	objects;
		std::vector<Frame> animation; // nonempty if tile is animated
		std::vector<WangTile> wangtiles; // one for each wangset the tile is part of
	};

	struct TileInstance
	{
		const Tile* tile = nullptr; //TODO: replace with index
		uint8_t flip_flags = 0;
	};

	struct WangColor
	{
		std::string name;
		std::string class_;
		Properties properties;
		unsigned int tile_id = UINT_MAX; // Index into Tileset::tiles; UINT_MAX if no tile has been chosen to represent this color
		float probability = 0.f;
		Color color;
	};

	struct WangSet
	{
		std::string name;
		std::string class_;
		Properties properties;
		unsigned int tile_id = UINT_MAX; // Index into Tileset::tiles; UINT_MAX if no tile has been chosen to represent this set
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

	enum class LayerType
	{
		Tile,
		Object,
		Image,
		Group,
	};

	struct Layer
	{
		LayerType type;
		std::string name;
		std::string class_;
		Properties properties;
		std::vector<TileInstance> tiles; // nonempty if type = LayerType::Tile; size = width * height
		std::vector<Object> objects; // nonempty if type = LayerType::Object 
		unsigned int width = 0; // in tiles
		unsigned int height = 0; // in tiles
		bool visible = true;
	};

	struct TilesetRef
	{
		unsigned int first_gid = 0;
		Handle<Tileset> tileset;
	};

	struct Map
	{
		std::string path;
		std::string name; // filename without extension
		std::string class_;
		Properties properties;
		std::vector<TilesetRef> tilesets; // sorted by first_gid
		std::vector<Layer> layers;
		unsigned int width = 0; // in tiles
		unsigned int height = 0; // in tiles
		unsigned int tile_width = 0; // in pixels
		unsigned int tile_height = 0; // in pixels
	};

	Handle<Map> load_map(const std::string& path);
	Handle<Tileset> load_tileset(const std::string& path);
	Handle<Object> load_template(const std::string& path);

	const Map* get_map(Handle<Map> handle);
	const Tileset* get_tileset(Handle<Tileset> handle);
	const Object* get_template(Handle<Object> handle);

	std::span<const Map> get_maps();
	std::span<const Tileset> get_tilesets();
	std::span<const Object> get_templates();

	const Map* find_map_by_name(const std::string& name);
	const Tileset* find_tileset_by_name(const std::string& name);
	const Object* find_object_by_name(const Layer& layer, const std::string& name);
	const Object* find_object_by_name(const Map& map, const std::string& name);
	const Tile* find_tile_by_class(const Tileset& tileset, const std::string& class_);
}

