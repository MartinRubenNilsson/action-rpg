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
		Polygon,
		Polyline,
		Tile,
	};

	struct Tile;

	struct Object
	{
		std::filesystem::path path; // nonempty if object is a template
		ObjectType type = ObjectType::Rectangle;
		entt::entity entity = entt::null;
		std::string name;
		std::string class_;
		std::vector<Property> properties;
		std::vector<sf::Vector2f> points; // nonempty if type == Polygon or Polyline; relative to position
		sf::Vector2f position; // in pixels
		sf::Vector2f size; // in pixels
		const Tile* tile = nullptr; // nonnull if type == Tile
	};

	struct Frame
	{
		uint32_t duration = 0; // in milliseconds
		const Tile* tile = nullptr;
	};

	struct Tileset;

	struct Tile
	{
		sf::Sprite sprite;
		std::string class_;
		std::vector<Property> properties;
		std::vector<Object>	objects;
		std::vector<Frame> animation;
		const Tileset* tileset = nullptr;

		bool is_animated() const;
		uint32_t get_animation_duration() const; // in milliseconds
		const Tile* query_animation(uint32_t time_in_ms) const;
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
		uint32_t tile_width = 0; // in pixels
		uint32_t tile_height = 0; // in pixels
		uint32_t tile_count = 0;
		uint32_t columns = 0;
		uint32_t spacing = 0; // in pixels
		uint32_t margin = 0; // in pixels

		bool reload_image();
		const Tile* find_tile(const std::string& class_) const;
	};

	struct Layer
	{
		uint32_t index = UINT32_MAX; // index in Map::layers
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
		std::string class_;
		std::vector<Property> properties;
		std::vector<Layer> layers;
		uint32_t width = 0; // in tiles
		uint32_t height = 0; // in tiles
		uint32_t tile_width = 0; // in pixels
		uint32_t tile_height = 0; // in pixels

		std::string get_name() const;
	};

	void load_assets();

	const std::vector<Tileset>& get_tilesets();
	const std::vector<Object>& get_templates();
	const std::vector<Map>& get_maps();
}

