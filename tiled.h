#pragma once

namespace tiled
{
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
		Polygon, // Object::points is nonempty
		Polyline, // Object::points is nonempty
		Tile, // Object::tile is nonnull
	};

	struct Tile;

	struct Object
	{
		std::filesystem::path path; // nonempty if object is a template
		std::string name;
		std::string class_;
		std::vector<Property> properties;
		std::vector<sf::Vector2f> points; // in pixels; relative to position
		const Tile* tile = nullptr;
		sf::Vector2f position; // in pixels
		sf::Vector2f size; // in pixels
		entt::entity entity = entt::null;
		ObjectType type = ObjectType::Rectangle;
	};

	struct Frame
	{
		uint32_t duration = 0; // in milliseconds
		const Tile* tile = nullptr;
	};

	struct Tileset;
	struct WangColor;

	struct Tile
	{
		sf::Sprite sprite;
		std::string class_;
		std::vector<Property> properties;
		std::vector<Object>	objects;
		std::vector<Frame> animation; // nonempty if tile is animated
		const Tileset* tileset = nullptr;
		// Each of the 8 corners/edges of the tile may have a Wang color. The order is:
		// [top, top-right, right, bottom-right, bottom, bottom-left, left, top-left]
		// If a corner/edge is colorless, the corresponding pointer is null.
		const WangColor* wangcolors[8] = {}; 
	};

	struct WangColor
	{
		std::string name;
		std::string class_;
		const Tile* tile = nullptr; // the tile representing this color; may be null
		float probability = 0.f;
		sf::Color color;
	};

	struct WangSet
	{
		std::string name;
		std::string class_;
		const Tile* tile = nullptr; // the tile representing this set; may be null
		std::vector<WangColor> colors;
	};

	struct Tileset
	{
		std::filesystem::path path;
		std::filesystem::path image_path;
		sf::Texture image;
		std::string name;
		std::string class_;
		std::vector<Property> properties;
		std::vector<Tile> tiles; // size = tile_count
		std::vector<WangSet> wangsets;
		uint32_t tile_width = 0; // in pixels
		uint32_t tile_height = 0; // in pixels
		uint32_t tile_count = 0;
		uint32_t columns = 0;
		uint32_t spacing = 0; // in pixels
		uint32_t margin = 0; // in pixels

		bool reload_image();
	};

	struct Layer
	{
		std::string name;
		std::string class_;
		std::vector<Property> properties;
		std::vector<const Tile*> tiles; // nonempty if tile layer; size = width * height
		std::vector<Object> objects; // nonempty if object layer 
		uint32_t width = 0; // in tiles
		uint32_t height = 0; // in tiles
	};

	struct Map
	{
		std::filesystem::path path;
		std::string name; // filename without extension
		std::string class_;
		std::vector<Property> properties;
		std::vector<Layer> layers;
		uint32_t width = 0; // in tiles
		uint32_t height = 0; // in tiles
		uint32_t tile_width = 0; // in pixels
		uint32_t tile_height = 0; // in pixels
	};

	void load_assets();
	void unload_assets();

	const std::vector<Tileset>& get_tilesets();
	const std::vector<Object>& get_templates();
	const std::vector<Map>& get_maps();

	template <typename T> bool get(const std::vector<Property>& properties, const std::string& name, T& value);
	template <typename T> void set(std::vector<Property>& properties, const std::string& name, const T& value);

	uint32_t get_animation_duration(const std::vector<Frame>& animation); // in milliseconds
	const Tile* sample_animation(const std::vector<Frame>& animation, uint32_t time_in_ms);
	const Tile* find_tile_by_class(const std::vector<Tile>& tiles, const std::string& class_);

	template <typename T>
	bool get(const std::vector<Property>& properties, const std::string& name, T& value) {
		for (const Property& property : properties) {
			if (property.name == name && std::holds_alternative<T>(property.value)) {
				value = std::get<T>(property.value);
				return true;
			}
		}
		return false;
	}

	template <typename T>
	void set(std::vector<Property>& properties, const std::string& name, const T& value) {
		for (Property& property : properties) {
			if (property.name == name) {
				property.value = value;
				return;
			}
		}
		properties.emplace_back(name, value);
	}
}

