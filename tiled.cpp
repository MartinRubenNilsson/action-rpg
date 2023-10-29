#include "stdafx.h"
#include "tiled.h"
#include <pugixml.hpp>
#include "console.h"

namespace tiled
{
	std::unordered_map<std::filesystem::path, std::unique_ptr<sf::Texture>> _images;
	std::vector<Tileset> _tilesets;
	std::unordered_map<std::filesystem::path, Object> _templates;
	std::unordered_map<std::filesystem::path, Map> _maps;

	sf::Texture& _get_image(const std::filesystem::path& path)
	{
		auto& image = _images[path];
		if (!image)
		{
			image = std::make_unique<sf::Texture>();
			image->loadFromFile(path.string());
		}
		return *image;
	}

	void _load_property(const pugi::xml_node& node, Property& prop)
	{
		prop.name = node.attribute("name").as_string();
		std::string type = node.attribute("type").as_string();
		if (type.empty()) { // default type is string
			prop.value = node.attribute("value").as_string();
		} else if (type == "int") {
			prop.value = node.attribute("value").as_int();
		} else if (type == "float") {
			prop.value = node.attribute("value").as_float();
		} else if (type == "bool") {
			prop.value = node.attribute("value").as_bool();
		} else if (type == "color") {
			// TODO
		} else if (type == "file") {
			// TODO
		} else if (type == "object") {
			uint32_t id = node.attribute("value").as_uint(); // 0 when no object is referenced
			prop.value = id ? (entt::entity)id : entt::null;
		} else if (type == "class") {
			// TODO
		} else {
			assert(false);
		}
	}

	void _load_properties(const pugi::xml_node& node, std::vector<Property>& properties)
	{
		for (auto prop_node : node.children("property"))
			_load_property(prop_node, properties.emplace_back());
	}

	void _load_object(const pugi::xml_node& node, Object& object)
	{
		if (uint32_t id = node.attribute("id").as_uint())
			object.id = (entt::entity)id;
		if (auto name = node.attribute("name"))
			object.name = name.as_string();
		if (auto type = node.attribute("type"))
			object.class_ = type.as_string();
		if (auto x = node.attribute("x"))
			object.position.x = x.as_float();
		if (auto y = node.attribute("y"))
			object.position.y = y.as_float();
		if (auto width = node.attribute("width"))
			object.size.x = width.as_float();
		if (auto height = node.attribute("height"))
			object.size.y = height.as_float();
		if (auto gid = node.attribute("gid"))
		{
			object.type = ObjectType::Tile;
		}
		else if (node.child("ellipse"))
		{
			object.type = ObjectType::Ellipse;
		}
		else if (node.child("point"))
		{
			object.type = ObjectType::Point;
		}
		else if (auto polygon = node.child("polygon"))
		{
			object.type = ObjectType::Polygon;
			object.points.clear();
			for (auto point : polygon.children("point"))
			{
				float x = point.attribute("x").as_float();
				float y = point.attribute("y").as_float();
				object.points.emplace_back(x, y);
			}
		}
		else if (auto polyline = node.child("polyline"))
		{
			object.type = ObjectType::Polyline;
			object.points.clear();
			for (auto point : polyline.children("point"))
			{
				float x = point.attribute("x").as_float();
				float y = point.attribute("y").as_float();
				object.points.emplace_back(x, y);
			}
		}
		else
		{
			object.type = ObjectType::Rectangle;
		}
		//TODO: make sure _load_properties patches the properties vector
		_load_properties(node.child("properties"), object.properties);
	}

	TileInstance _resolve_gid(const TilesetInstance* tilesets, size_t tileset_count, uint32_t gid)
	{
		TileInstance tile;
		for (size_t i = 0; i < tileset_count; ++i)
		{
			TilesetInstance tileset = tilesets[i];
			if (tileset.id < _tilesets.size() && 
				gid >= tileset.first_gid &&
				gid < tileset.first_gid + _tilesets[tileset.id].tile_count)
			{
				tile.tileset_id = tileset.id;
				tile.id = gid - tileset.first_gid;
				break;
			}
		}
		return tile;
	}

	bool _is_layer(const char* str)
	{
		if (strcmp(str, "layer") == 0) return true;
		if (strcmp(str, "objectgroup") == 0) return true;
		if (strcmp(str, "imagelayer") == 0) return true;
		if (strcmp(str, "group") == 0) return true;
		return false;
	}

	LayerType _parse_layer_type(const char* str)
	{
		if (strcmp(str, "layer") == 0) return LayerType::Tile;
		if (strcmp(str, "objectgroup") == 0) return LayerType::Object;
		if (strcmp(str, "imagelayer") == 0) return LayerType::Image;
		if (strcmp(str, "group") == 0) return LayerType::Group;
		assert(false);
	}

	void _load_layer_recursive(const pugi::xml_node& node, uint32_t parent_index, std::vector<Layer>& layers)
	{
		uint32_t index = (uint32_t)layers.size();
		Layer& layer = layers.emplace_back();
		layer.type = _parse_layer_type(node.name());
		layer.parent_index = parent_index;
		layer.name = node.attribute("name").as_string();
		layer.class_ = node.attribute("class").as_string();
		layer.width = node.attribute("width").as_uint();
		layer.height = node.attribute("height").as_uint();
		_load_properties(node.child("properties"), layer.properties);
		switch (layer.type)
		{
		case LayerType::Tile:
		{

			break;
		}
		case LayerType::Object:
		{
			// TODO: handle templates
			//for (auto object_node : node.children("object"))
			//	_load_object(object_node, layer.objects.emplace_back(), layer.tiles.emplace_back().gid);
			break;
		}
		case LayerType::Image:
		{
			// TODO
			break;
		}
		case LayerType::Group:
		{
			for (auto child_node : node.children())
				if (_is_layer(child_node.name()))
					_load_layer_recursive(child_node, index, layers);
			break;
		}
		}
	}

	TilesetInstance _load_tileset(const pugi::xml_node& node, const std::filesystem::path& parent_path)
	{
		TilesetInstance tileset;
		auto source_attribute = node.attribute("source");
		if (!source_attribute)
		{
			console::log_error("Embedded tilesets are not supported.");
			return tileset;
		}
		auto path = (parent_path / source_attribute.as_string()).lexically_normal();
		for (uint16_t tileset_id = 0; tileset_id < _tilesets.size(); ++tileset_id)
		{
			if (_tilesets[tileset_id].path == path)
			{
				tileset.id = tileset_id;
				tileset.first_gid = node.attribute("firstgid").as_uint();
				break;
			}
		}
		return tileset;
	}

	void load_assets()
	{
		_tilesets.clear();
		_templates.clear();
		_maps.clear();

		_tilesets.reserve(64);
		_templates.reserve(64);
		_maps.reserve(64);

		// Load tilesets
		for (const auto& entry : std::filesystem::recursive_directory_iterator("assets/tiled/tilesets"))
		{
			if (!entry.is_regular_file()) continue;
			const auto& path = entry.path();
			if (path.extension() != ".tsx") continue;
			pugi::xml_document doc;
			if (!doc.load_file(path.string().c_str())) continue;
			pugi::xml_node tileset_node = doc.child("tileset");
			if (!tileset_node) continue;
			Tileset& tileset = _tilesets.emplace_back();
			tileset.path = path;
			tileset.name = tileset_node.attribute("name").as_string();
			tileset.class_ = tileset_node.attribute("class").as_string();
			tileset.tile_width = tileset_node.attribute("tilewidth").as_uint();
			tileset.tile_height = tileset_node.attribute("tileheight").as_uint();
			tileset.tile_count = tileset_node.attribute("tilecount").as_uint();
			tileset.columns = tileset_node.attribute("columns").as_uint();
			tileset.spacing = tileset_node.attribute("spacing").as_uint();
			tileset.margin = tileset_node.attribute("margin").as_uint();
			_load_properties(tileset_node.child("properties"), tileset.properties);
			tileset.image_path = path.parent_path();
			tileset.image_path /= tileset_node.child("image").attribute("source").as_string();
			tileset.image_path = tileset.image_path.lexically_normal();
			sf::Texture& image = _get_image(tileset.image_path);
			tileset.tiles.resize(tileset.tile_count);
			for (auto tile_node : tileset_node.children("tile"))
			{
				uint32_t id = tile_node.attribute("id").as_uint();
				Tile& tile = tileset.tiles[id];
				tile.class_ = tile_node.attribute("type").as_string();
				tile.rect.left = (id % tileset.columns) * (tileset.tile_width + tileset.spacing) + tileset.margin;
				tile.rect.top = (id / tileset.columns) * (tileset.tile_height + tileset.spacing) + tileset.margin;
				tile.rect.width = tileset.tile_width;
				tile.rect.height = tileset.tile_height;
				tile.sprite.setTexture(image);
				tile.sprite.setTextureRect(tile.rect);
				_load_properties(tile_node.child("properties"), tile.properties);
				for (auto frame_node : tile_node.child("animation").children("frame"))
				{
					Frame& frame = tile.animation.emplace_back();
					frame.tile_id = frame_node.attribute("tileid").as_uint();
					frame.duration = frame_node.attribute("duration").as_uint();
				}
				for (auto object_node : tile_node.child("objectgroup").children("object"))
					_load_object(object_node, tile.objects.emplace_back());
			}
		}

		// Load templates
		for (const auto& entry : std::filesystem::recursive_directory_iterator("assets/tiled/templates"))
		{
			if (!entry.is_regular_file()) continue;
			const auto& path = entry.path();
			if (path.extension() != ".tx") continue;
			pugi::xml_document doc;
			if (!doc.load_file(path.string().c_str())) continue;
			pugi::xml_node template_node = doc.child("template");
			if (!template_node) continue;
			pugi::xml_node object_node = template_node.child("object");
			Object& object = _templates[path];
			_load_object(object_node, object);
			if (auto tileset_node = template_node.child("tileset"))
			{
				TilesetInstance tileset = _load_tileset(tileset_node, path.parent_path());
				uint32_t gid = tileset_node.attribute("gid").as_uint();
				object.tile = _resolve_gid(&tileset, 1, gid);
			}
		}

		// Load maps
		for (const auto& entry : std::filesystem::recursive_directory_iterator("assets/tiled/maps"))
		{
			if (!entry.is_regular_file()) continue;
			const auto& path = entry.path();
			if (path.extension() != ".tmx") continue;
			pugi::xml_document doc;
			if (!doc.load_file(path.string().c_str())) continue;
			pugi::xml_node map_node = doc.child("map");
			if (!map_node) continue;
			Map& map = _maps[path];
			map.class_ = map_node.attribute("class").as_string();
			map.width = map_node.attribute("width").as_uint();
			map.height = map_node.attribute("height").as_uint();
			map.tile_width = map_node.attribute("tilewidth").as_uint();
			map.tile_height = map_node.attribute("tileheight").as_uint();
			_load_properties(map_node.child("properties"), map.properties);
			auto parent_path = path.parent_path();
			for (auto tileset_node : map_node.children("tileset"))
				map.tilesets.push_back(_load_tileset(tileset_node, parent_path));
			for (auto child_node : map_node.children())
				if (_is_layer(child_node.name()))
					_load_layer_recursive(child_node, UINT32_MAX, map.layers);
		}
	}

	const Tile* get_tile(TileInstance tile)
	{
		if (tile.tileset_id >= _tilesets.size()) return nullptr;
		const Tileset& tileset = _tilesets[tile.tileset_id];
		if (tile.id >= tileset.tiles.size()) return nullptr;
		return &tileset.tiles[tile.id];
	}

	const Map* get_map(const std::filesystem::path& path)
	{
		// TODO
		//for (const Map& map : _maps)
		//	if (map.path == path)
		//		return &map;
		return nullptr;
	}

	uint16_t get_total_duration(const std::vector<Frame>& animation)
	{
		uint16_t total_duration = 0;
		for (const auto& frame : animation)
			total_duration += frame.duration;
		return total_duration;
	}
}