#include "stdafx.h"
#include "tiled.h"
#include <pugixml.hpp>

namespace tiled
{
	std::vector<Tileset> _tilesets;
	std::unordered_map<std::filesystem::path, Object> _templates;
	std::vector<Map> _maps;

	void _load_property(const pugi::xml_node& node, Property& prop)
	{
		prop.name = node.attribute("name").as_string();
		std::string type = node.attribute("type").as_string();
		if (type == "string") {
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
		object.id = node.attribute("id").as_uint();
		object.name = node.attribute("name").as_string();
		object.type = node.attribute("type").as_string();
		object.position.x = node.attribute("x").as_float();
		object.position.y = node.attribute("y").as_float();
		object.size.x = node.attribute("width").as_float();
		object.size.y = node.attribute("height").as_float();
		//object.tile_gid = node.attribute("gid").as_uint();
		_load_properties(node.child("properties"), object.properties);
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
			uint16_t tileset_id = (uint16_t)_tilesets.size();
			Tileset& tileset = _tilesets.emplace_back();
			tileset.path = path;
			tileset.name = tileset_node.attribute("name").as_string();
			tileset.type = tileset_node.attribute("class").as_string();
			tileset.tile_width = tileset_node.attribute("tilewidth").as_uint();
			tileset.tile_height = tileset_node.attribute("tileheight").as_uint();
			tileset.tile_count = tileset_node.attribute("tilecount").as_uint();
			tileset.columns = tileset_node.attribute("columns").as_uint();
			tileset.spacing = tileset_node.attribute("spacing").as_uint();
			tileset.margin = tileset_node.attribute("margin").as_uint();
			if (auto image_node = tileset_node.child("image"))
			{
				tileset.image.path = path.parent_path() / image_node.attribute("source").as_string();
				tileset.image.texture.loadFromFile(tileset.image.path.string());
			}
			tileset.tiles.resize(tileset.tile_count);
			for (uint16_t tile_id = 0; tile_id < tileset.tile_count; ++tile_id)
			{
				tileset.tiles[tile_id].id.tile_id = tile_id;
				tileset.tiles[tile_id].id.tileset_id = tileset_id;
			}
			for (auto tile_node : tileset_node.children("tile"))
			{
				Tile& tile = tileset.tiles[tile_node.attribute("id").as_uint()];
				tile.type = tile_node.attribute("type").as_string();
				for (auto frame_node : tile_node.child("animation").children("frame"))
				{
					Frame& frame = tile.animation.emplace_back();
					frame.tile_id = frame_node.attribute("tileid").as_uint();
					frame.duration = frame_node.attribute("duration").as_uint();
				}
				_load_properties(tile_node.child("properties"), tile.properties);
			}
			_load_properties(tileset_node.child("properties"), tileset.properties);
		}

		// Load templates
		for (const auto& entry : std::filesystem::recursive_directory_iterator("assets/tiled/templates"))
		{
			if (!entry.is_regular_file()) continue;
			const auto& path = entry.path();
			if (path.extension() != ".tx") continue;
			pugi::xml_document doc;
			if (!doc.load_file(path.string().c_str())) continue;
			// TODO
		}
	}

	uint16_t get_total_duration(const std::vector<Frame>& animation)
	{
		uint16_t total_duration = 0;
		for (const auto& frame : animation)
			total_duration += frame.duration;
		return total_duration;
	}
}