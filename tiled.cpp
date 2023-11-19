#include "stdafx.h"
#include "tiled.h"
#include <pugixml.hpp>
#include "console.h"

namespace tiled
{
	std::vector<std::pair<std::filesystem::path, sf::Texture>> _images;
	std::vector<std::pair<std::filesystem::path, Tileset>> _tilesets;
	std::vector<std::pair<std::filesystem::path, Object>> _templates;
	std::vector<std::pair<std::filesystem::path, Map>> _maps;

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
			prop.value = node.attribute("value").as_uint(); // 0 when no object is referenced
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
		if (auto template_attribute = node.attribute("template"))
		{
			for (const auto& [template_path, template_object] : _templates)
			{
				if (template_path == template_attribute.as_string())
				{
					object = template_object;
					break;
				}
			}
		}
		if (auto id = node.attribute("id"))
			object.id = id.as_uint();
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
		_load_properties(node.child("properties"), object.properties);
	}

	struct TilesetInstance
	{
		uint32_t id = UINT32_MAX;
		uint32_t first_gid = UINT32_MAX; // The global tile ID of the first tile in the tileset.
	};

	TileInstance _resolve_gid(const TilesetInstance* tilesets, size_t size, uint32_t gid)
	{
		TileInstance tile;
		for (size_t i = 0; i < size; ++i)
		{
			TilesetInstance tileset = tilesets[i];
			if (tileset.id < _tilesets.size() && 
				gid >= tileset.first_gid &&
				gid < tileset.first_gid + _tilesets[tileset.id].second.tile_count)
			{
				tile.tileset_id = tileset.id;
				tile.id = gid - tileset.first_gid;
				break;
			}
		}
		return tile;
	}

	bool _is_layer_type(const char* str)
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

	void _load_layer_recursive(const pugi::xml_node& node, std::vector<Layer>& layers, uint32_t parent_index,
		const std::vector<TilesetInstance>& tilesets)
	{
		uint32_t index = layers.size();
		Layer& layer = layers.emplace_back();
		layer.index = index;
		layer.parent_index = parent_index;
		layer.type = _parse_layer_type(node.name());
		layer.name = node.attribute("name").as_string();
		layer.class_ = node.attribute("class").as_string();
		layer.width = node.attribute("width").as_uint();
		layer.height = node.attribute("height").as_uint();
		_load_properties(node.child("properties"), layer.properties);
		switch (layer.type)
		{
		case LayerType::Tile:
		{
			auto data_node = node.child("data");
			if (strcmp(data_node.attribute("encoding").as_string(), "csv") != 0)
			{
				console::log_error("Only CSV encoding is supported.");
				layer.width = 0;
				layer.height = 0;
				break;
			}
			std::vector<uint32_t> gids;
			{
				std::stringstream ss(data_node.text().as_string());
				for (uint32_t gid; ss >> gid;) {
					gids.push_back(gid);
					if (ss.peek() == ',')
						ss.ignore();
				}
			}
			assert(gids.size() == layer.width * layer.height);
			layer.tiles.resize(gids.size());
			for (size_t i = 0; i < gids.size(); ++i)
				layer.tiles[i] = _resolve_gid(tilesets.data(), tilesets.size(), gids[i]);
			break;
		}
		case LayerType::Object:
		{
			for (pugi::xml_node object_node : node.children("object"))
				_load_object(object_node, layer.objects.emplace_back());
			break;
		}
		case LayerType::Image:
		{
			// TODO
			break;
		}
		case LayerType::Group:
		{
			for (pugi::xml_node child_node : node.children())
				if (_is_layer_type(child_node.name()))
					_load_layer_recursive(child_node, layers, index, tilesets);
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
		for (uint32_t tileset_id = 0; tileset_id < _tilesets.size(); ++tileset_id)
		{
			if (_tilesets[tileset_id].first == path)
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
		_images.clear();
		_tilesets.clear();
		_templates.clear();
		_maps.clear();

		// Find assets
		for (const auto& entry : std::filesystem::recursive_directory_iterator("assets/tiled"))
		{
			if (!entry.is_regular_file()) continue;
			std::string extension = entry.path().extension().string();
			if (extension == ".png")
				_images.emplace_back().first = entry.path();
			else if (extension == ".tsx")
				_tilesets.emplace_back().first = entry.path();
			else if (extension == ".tx")
				_templates.emplace_back().first = entry.path();
			else if (extension == ".tmx")
				_maps.emplace_back().first = entry.path();
		}

		// Load images
		for (auto& [path, image] : _images)
			if (!image.loadFromFile(path.string()))
				console::log_error("Failed to load image: " + path.string());

		// Load tilesets
		for (auto& [path, tileset] : _tilesets)
		{
			pugi::xml_document doc;
			if (!doc.load_file(path.string().c_str()))
			{
				console::log_error("Failed to load tileset: " + path.string());
				continue;
			}
			pugi::xml_node tileset_node = doc.child("tileset");
			tileset.name = tileset_node.attribute("name").as_string();
			tileset.class_ = tileset_node.attribute("class").as_string();
			tileset.tile_width = tileset_node.attribute("tilewidth").as_uint();
			tileset.tile_height = tileset_node.attribute("tileheight").as_uint();
			tileset.tile_count = tileset_node.attribute("tilecount").as_uint();
			tileset.columns = tileset_node.attribute("columns").as_uint();
			tileset.spacing = tileset_node.attribute("spacing").as_uint();
			tileset.margin = tileset_node.attribute("margin").as_uint();
			_load_properties(tileset_node.child("properties"), tileset.properties);
			tileset.tiles.resize(tileset.tile_count);
			{
				std::filesystem::path image_path = path.parent_path();
				image_path /= tileset_node.child("image").attribute("source").as_string();
				image_path = image_path.lexically_normal();
				for (uint32_t i = 0; i < _images.size(); ++i)
					if (_images[i].first == image_path)
						for (Tile& tile : tileset.tiles)
							tile.sprite.setTexture(_images[i].second);
			}
			for (uint32_t i = 0; i < tileset.tile_count; ++i)
			{
				Tile& tile = tileset.tiles[i];
				sf::IntRect rect;
				rect.left = (i % tileset.columns) * (tileset.tile_width + tileset.spacing) + tileset.margin;
				rect.top = (i / tileset.columns) * (tileset.tile_height + tileset.spacing) + tileset.margin;
				rect.width = tileset.tile_width;
				rect.height = tileset.tile_height;
				tile.sprite.setTextureRect(rect);
			}
			for (auto tile_node : tileset_node.children("tile"))
			{
				uint32_t id = tile_node.attribute("id").as_uint();
				Tile& tile = tileset.tiles[id];
				tile.class_ = tile_node.attribute("type").as_string();
				_load_properties(tile_node.child("properties"), tile.properties);
				for (pugi::xml_node frame_node : tile_node.child("animation").children("frame"))
				{
					Frame& frame = tile.animation.emplace_back();
					frame.tile_id = frame_node.attribute("tileid").as_uint();
					frame.duration = frame_node.attribute("duration").as_uint();
				}
				for (pugi::xml_node object_node : tile_node.child("objectgroup").children("object"))
					_load_object(object_node, tile.objects.emplace_back());
			}
		}

		// Load templates
		for (auto& [path, object] : _templates)
		{
			pugi::xml_document doc;
			if (!doc.load_file(path.string().c_str()))
			{
				console::log_error("Failed to load template: " + path.string());
				continue;
			}
			pugi::xml_node template_node = doc.child("template");
			_load_object(template_node.child("object"), object);
			if (pugi::xml_node tileset_node = template_node.child("tileset"))
			{
				TilesetInstance tileset = _load_tileset(tileset_node, path.parent_path());
				uint32_t gid = tileset_node.attribute("gid").as_uint();
				object.tile = _resolve_gid(&tileset, 1, gid);
			}
		}

		// Load maps
		for (auto& [path, map] : _maps)
		{
			pugi::xml_document doc;
			if (!doc.load_file(path.string().c_str()))
			{
				console::log_error("Failed to load map: " + path.string());
				continue;
			}
			pugi::xml_node map_node = doc.child("map");
			map.class_ = map_node.attribute("class").as_string();
			map.width = map_node.attribute("width").as_uint();
			map.height = map_node.attribute("height").as_uint();
			map.tile_width = map_node.attribute("tilewidth").as_uint();
			map.tile_height = map_node.attribute("tileheight").as_uint();
			_load_properties(map_node.child("properties"), map.properties);
			std::vector<TilesetInstance> tilesets;
			for (pugi::xml_node tileset_node : map_node.children("tileset"))
				tilesets.push_back(_load_tileset(tileset_node, path.parent_path()));
			for (pugi::xml_node child_node : map_node.children())
				if (_is_layer_type(child_node.name()))
					_load_layer_recursive(child_node, map.layers, UINT32_MAX, tilesets);
		}
	}

	const Tile* find_tile(TileInstance tile)
	{
		if (tile.tileset_id >= _tilesets.size()) return nullptr;
		const Tileset& tileset = _tilesets[tile.tileset_id].second;
		if (tile.id >= tileset.tiles.size()) return nullptr;
		return &tileset.tiles[tile.id];
	}

	const Map* find_map(const std::string& filename)
	{
		for (const auto& [path, map] : _maps)
			if (path.filename() == filename)
				return &map;
		return nullptr;
	}

	uint32_t get_total_duration(const std::vector<Frame>& animation)
	{
		uint32_t total_duration = 0;
		for (const Frame& frame : animation)
			total_duration += frame.duration;
		return total_duration;
	}
}