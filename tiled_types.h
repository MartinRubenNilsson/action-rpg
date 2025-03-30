#pragma once
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace tiled {

	struct Color {
		unsigned char r = 0;
		unsigned char g = 0;
		unsigned char b = 0;
		unsigned char a = 0;
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

	Property*       find_property_by_name(std::vector<Property>& properties, std::string_view name);
	const Property* find_property_by_name(const std::vector<Property>& properties, std::string_view name);

	template <PropertyType type>
	bool get(const std::vector<Property>& properties, std::string_view name, std::variant_alternative_t<(size_t)type, PropertyValue>& value) {
		const Property* prop = find_property_by_name(properties, name);
		if (!prop) return false;
		if (prop->value.index() != (size_t)type) return false;
		value = std::get<(size_t)type>(prop->value);
		return true;
	}

	// A 32-bit integer that stores a tile GID in the lower 28 bits and flip flags in the upper 4 bits.
	struct TileGID {
		union {
			unsigned int value = 0;
			struct {
				unsigned int gid : 28; // global tile ID
				unsigned int rotated_hexagonal_120 : 1; // only for hexagonal maps
				unsigned int flipped_diagonally : 1;
				unsigned int flipped_vertically : 1;
				unsigned int flipped_horizontally : 1;
			};
		};
	};

	struct TilesetRef {
		unsigned int first_gid = 0;
		Handle<Tileset> tileset; // FIXME: don't use Handle
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

	struct Object {
		unsigned int id = 0; // valid IDs are >= 1
		ObjectType type = ObjectType::Rectangle;
		std::string template_path;
		std::string name;
		std::string class_;
		std::vector<Property> properties;
		std::vector<Vector2f> points; // in pixels; relative to position; only relevant if type = ObjectType::Polygon/Polyline
		TileGID tile_gid; // only relevant if type = ObjectType::Tile
		TilesetRef tileset_ref; // only relevant if type = ObjectType::Tile
		Vector2f position; // in pixels
		Vector2f size; // in pixels
	};

	struct Frame {
		unsigned int duration_ms = 0; // in milliseconds
		unsigned int tile_id = 0; // Index into Tileset::tiles[].
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

		// Index into Tileset::tiles[].
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

	struct TextureRect {
		unsigned int x = 0; // in pixels
		unsigned int y = 0; // in pixels
		unsigned int w = 0; // in pixels
		unsigned int h = 0; // in pixels
	};

	struct Tileset {
		std::string path;
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

		TextureRect get_texture_rect(unsigned int id) const;
	};

	enum class LayerType {
		Tile,
		Object,
		Image,
		Group,
	};

	struct Layer {
		LayerType type{};
		std::string name;
		std::string class_;
		std::vector<Property> properties;
		std::vector<TileGID> tiles; // nonempty if type = LayerType::Tile; size = width * height
		std::vector<Object> objects; // nonempty if type = LayerType::Object 
		unsigned int width = 0; // in tiles
		unsigned int height = 0; // in tiles
		bool visible = true;
	};

	struct Map {
		std::string path;
		std::string name; // filename without extension
		std::string class_;
		std::vector<Property> properties;
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
}