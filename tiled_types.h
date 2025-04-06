#pragma once
#include <string>
#include <variant>
#include <vector>

namespace tiled {

	struct Color {
		unsigned char r = 0;
		unsigned char g = 0;
		unsigned char b = 0;
		unsigned char a = 0;
	};

	struct TextureRect {
		unsigned int x = 0; // in pixels
		unsigned int y = 0; // in pixels
		unsigned int w = 0; // in pixels
		unsigned int h = 0; // in pixels
	};

	enum class PropertyType {
		String,
		Int,
		Float,
		Bool,
		Color,
		File,
		Object,
		Class
	};

	using PropertyValue = std::variant<
		std::string, // string
		int, // int
		float, // float
		bool, // bool
		Color, // color
		std::string, // file
		unsigned int, // object
		std::string // class
	>;

	struct Property {
		std::string name;
		PropertyValue value;
	};

	// A 32-bit integer that stores a tile GID in the lower 28 bits and flip flags in the upper 4 bits.
	// 
	// These GIDs are "global" because they may refer to a tile from any of the tilesets used by a map,
	// rather than being local to a specific tileset. To get at a specific tile from a GID, you will
	// need to determine which tileset the tile belongs to, and which tile within the tileset it is.
	//
	struct TileGid {
		union {
			uint32_t value = 0;
			struct {
				uint32_t gid : 28; // global tile ID
				uint32_t rotated_hexagonal_120 : 1; // only for hexagonal maps
				uint32_t flipped_diagonally : 1;
				uint32_t flipped_vertically : 1;
				uint32_t flipped_horizontally : 1;
			};
		};
	};

	struct TilesetLink {
		unsigned int first_gid = 0; // the global tile ID of the first tile in the tileset
		unsigned int tileset_id = 0; // index into Context::tilesets[]
	};

	enum class ObjectType {
		Rectangle,
		Ellipse,
		Point,
		Polygon,
		Polyline,
		Tile,
		Text, // not supported right now
	};

	struct Point {
		float x = 0.f;
		float y = 0.f;
	};

	struct Object {
		unsigned int id = 0; // valid IDs are >= 1
		ObjectType type = ObjectType::Rectangle;
		std::string template_path; //TODO: remove
		std::string name;
		std::string class_;
		std::vector<Property> properties;
		std::vector<Point> points; // in pixels; relative to position; only relevant if type = Polygon/Polyline
		TileGid tile; // only relevant if type = Tile
		TilesetLink tileset; // only relevant if type = Tile
		float x = 0.f; // in pixels
		float y = 0.f; // in pixels
		float width = 0.f; // in pixels
		float height = 0.f; // in pixels
	};

	struct Frame {
		unsigned int duration_ms = 0; // in milliseconds
		unsigned int tile_id = 0; // index into Tileset::tiles[].
	};

	struct Tile {
		std::string class_;
		std::vector<Property> properties;
		std::vector<Object>	objects;
		std::vector<Frame> animation;
	};

	struct WangColor {
		std::string name;
		std::string class_;
		std::vector<Property> properties;
		// Index into Tileset::tiles[] of the tile representing this wangcolor,
		// or UINT_MAX if no such tile has been chosen.
		unsigned int tile_id = 0;
		float probability = 0.f;
		Color color;
	};

	struct WangTile {
		enum { // Enumerates the edges and corners of the tile.
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

		// index into Tileset::tiles[].
		unsigned int tile_id = 0;
		// Indices into Tileset::wangsets[], or UNINT_MAX when the edge/corner is not assigned.
		unsigned int wang_ids[COUNT] = {};
	};

	struct WangSet {
		std::string name;
		std::string class_;
		std::vector<Property> properties;
		// Index into Tileset::tiles[] of the tile representing this wangset,
		// or UINT_MAX if no such tile has been chosen.
		unsigned int tile_id = 0;
		std::vector<WangColor> colors;
		std::vector<WangTile> tiles;
	};

	struct Tileset {
		std::string path; // empty if embedded in map
		std::string image_path;
		std::string name;
		std::string class_;
		std::vector<Property> properties;
		std::vector<Tile> tiles; // size = tile_count
		std::vector<WangSet> wangsets;
		unsigned int tile_count = 0;
		unsigned int columns = 0;
		unsigned int tile_width = 0; // in pixels
		unsigned int tile_height = 0; // in pixels
		unsigned int spacing = 0; // in pixels
		unsigned int margin = 0; // in pixels
	};

	enum class LayerType {
		Tile,
		Object,
		Image,
		Group,
	};

	struct Layer {
		LayerType type = LayerType::Tile;
		std::string name;
		std::string class_;
		std::vector<Property> properties;
		std::vector<TileGid> tiles; // nonempty if type = Tile; in that case size = width * height
		std::vector<Object> objects; // (possibly) nonempty if type = Object 
		unsigned int width = 0; // in tiles
		unsigned int height = 0; // in tiles
		bool visible = true;
	};

	struct Map {
		std::string path;
		std::string class_;
		std::vector<Property> properties;
		std::vector<TilesetLink> tilesets; // sorted by first_gid in ascending order
		std::vector<Layer> layers;
		unsigned int width = 0; // in tiles
		unsigned int height = 0; // in tiles
		unsigned int tile_width = 0; // in pixels
		unsigned int tile_height = 0; // in pixels
	};

	struct Context {
		void (*debug_message_callback)(std::string_view message) = nullptr;
		std::vector<Tileset> tilesets;
		std::vector<Object> templates;
		std::vector<Map> maps;
	};
}