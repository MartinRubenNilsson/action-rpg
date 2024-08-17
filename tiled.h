#pragma once
#include "properties.h"

namespace tiled
{
	struct Tile;
	struct WangColor;

	// A 32-bit integer that stores a tile GID in the lower 28 bits and flip flags in the upper 4 bits.
	struct TileRef
	{
		union
		{
			unsigned int value = 0;
			struct
			{
				unsigned int gid : 28; // global tile ID
				unsigned int rotated_hexagonal_120 : 1; // only for hexagonal maps
				unsigned int flipped_diagonally : 1;
				unsigned int flipped_vertically : 1;
				unsigned int flipped_horizontally : 1;
			};
		};
	};

	struct TilesetRef
	{
		unsigned int first_gid = 0;
		Handle<Tileset> tileset;
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
		ObjectType type{};
		std::string template_path;
		std::string name;
		std::string class_;
		Properties properties;
		std::vector<Vector2f> points; // in pixels; relative to position; only relevant if type = ObjectType::Polygon/Polyline
		TileRef tile_ref; // only relevant if type = ObjectType::Tile
		TilesetRef tileset_ref; // only relevant if type = ObjectType::Tile
		Vector2f position; // in pixels
		Vector2f size; // in pixels
	};

	struct Frame
	{
		unsigned int duration_ms = 0; // in milliseconds
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

		// TODO: don't use pointers
		const WangColor* wangcolors[COUNT] = {}; // null if uncolored
	};

	struct Tile
	{
		std::string class_;
		Properties properties;
		std::vector<Object>	objects;
		std::vector<Frame> animation; // nonempty if tile is animated
		std::vector<WangTile> wangtiles; // one for each wangset the tile is part of
	};

	struct WangColor
	{
		std::string name;
		std::string class_;
		Properties properties;
		unsigned int tile_id = 0; // Index into Tileset::tiles, or UINT_MAX if no tile has been chosen to represent this color.
		float probability = 0.f;
		Color color;
	};

	struct WangSet
	{
		std::string name;
		std::string class_;
		Properties properties;
		unsigned int tile_id = 0; // Index into Tileset::tiles, or UINT_MAX if no tile has been chosen to represent this set.
		std::vector<WangColor> colors;
	};

	struct TextureRect
	{
		unsigned int x = 0; // in pixels
		unsigned int y = 0; // in pixels
		unsigned int w = 0; // in pixels
		unsigned int h = 0; // in pixels
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

		TextureRect get_texture_rect(unsigned int id) const;
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
		LayerType type{};
		std::string name;
		std::string class_;
		Properties properties;
		std::vector<TileRef> tiles; // nonempty if type = LayerType::Tile; size = width * height
		std::vector<Object> objects; // nonempty if type = LayerType::Object 
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
		std::vector<TilesetRef> tilesets; // sorted by first_gid
		std::vector<Layer> layers;
		unsigned int width = 0; // in tiles
		unsigned int height = 0; // in tiles
		unsigned int tile_width = 0; // in pixels
		unsigned int tile_height = 0; // in pixels

		TilesetRef get_tileset_ref(unsigned int gid) const;
		const Tile* get_tile(unsigned int gid) const;
		const Object* get_object(const std::string& name) const;
	};

	std::span<const Map> get_maps();
	std::span<const Tileset> get_tilesets();
	std::span<const Object> get_templates();

	const Map* get_map(Handle<Map> handle);
	const Tileset* get_tileset(Handle<Tileset> handle);
	const Object* get_template(Handle<Object> handle);

	//TODO: return Handle instead of pointer
	const Map* find_map_by_name(const std::string& name);
	Handle<Tileset> find_tileset_by_name(const std::string& name);

	Handle<Map> load_map_from_file(const std::string& path);
	Handle<Tileset> load_tileset_from_file(const std::string& path);
	Handle<Object> load_template_from_file(const std::string& path);
}

