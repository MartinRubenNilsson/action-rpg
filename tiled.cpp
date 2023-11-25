#include "stdafx.h"
#include "tiled.h"
#include <pugixml.hpp>
#include "console.h"

#define _MAX_TILESET_COUNT 128
#define _MAX_TEMPLATE_COUNT 128
#define _MAX_MAP_COUNT 128

namespace tiled
{
	std::vector<Tileset> _tilesets;
	std::vector<Object> _templates;
	std::vector<Map> _maps;

	void _load_property(const pugi::xml_node& node, Property& prop)
	{
		prop.name = node.attribute("name").as_string();
		const pugi::char_t* type = node.attribute("type").as_string("string"); // default type is string
		if (strcmp(type, "string") == 0) {
			prop.value = node.attribute("value").as_string();
		} else if (strcmp(type, "int") == 0) {
			prop.value = node.attribute("value").as_int();
		} else if (strcmp(type, "float") == 0) {
			prop.value = node.attribute("value").as_float();
		} else if (strcmp(type, "bool") == 0) {
			prop.value = node.attribute("value").as_bool();
		} else if (strcmp(type, "color") == 0) {
			// TODO
		} else if (strcmp(type, "file") == 0) {
			// TODO
		} else if (strcmp(type, "object") == 0) {
			uint32_t id = node.attribute("value").as_uint(); // 0 when no object is referenced
			prop.value = id ? (entt::entity)id : entt::null;
		} else if (strcmp(type, "class") == 0) {
			// TODO
		} else {
			assert(false);
		}
	}

	void _load_object(const pugi::xml_node& node, Object& object)
	{
		for (pugi::xml_node prop_node : node.child("properties").children("property"))
		{
			std::string prop_name = prop_node.attribute("name").as_string();
			bool found = false;
			for (Property& prop : object.properties)
			{
				if (prop.name == prop_name)
				{
					found = true;
					_load_property(prop_node, prop);
					break;
				}
			}
			if (!found)
				_load_property(prop_node, object.properties.emplace_back());
		}
		if (pugi::xml_attribute id = node.attribute("id"))
			object.entity = (entt::entity)id.as_uint();
		if (pugi::xml_attribute name = node.attribute("name"))
			object.name = name.as_string();
		if (pugi::xml_attribute type = node.attribute("type")) // SIC: "type", not "class"
			object.class_ = type.as_string();
		if (pugi::xml_attribute x = node.attribute("x"))
			object.position.x = x.as_float();
		if (pugi::xml_attribute y = node.attribute("y"))
			object.position.y = y.as_float();
		if (pugi::xml_attribute width = node.attribute("width"))
			object.size.x = width.as_float();
		if (pugi::xml_attribute height = node.attribute("height"))
			object.size.y = height.as_float();
		if (node.child("ellipse"))
		{
			object.type = ObjectType::Ellipse;
		}
		else if (node.child("point"))
		{
			object.type = ObjectType::Point;
		}
		else if (pugi::xml_node polygon = node.child("polygon"))
		{
			object.type = ObjectType::Polygon;
			object.points.clear();
			for (pugi::xml_node point : polygon.children("point"))
			{
				object.points.emplace_back(
					point.attribute("x").as_float(),
					point.attribute("y").as_float());
			}
		}
		else if (pugi::xml_node polyline = node.child("polyline"))
		{
			object.type = ObjectType::Polyline;
			object.points.clear();
			for (pugi::xml_node point : polyline.children("point"))
			{
				object.points.emplace_back(
					point.attribute("x").as_float(),
					point.attribute("y").as_float());
			}
		}
	}

	bool _is_tile_layer(const char* str) { return strcmp(str, "layer") == 0; }
	bool _is_object_layer(const char* str) { return strcmp(str, "objectgroup") == 0; }
	bool _is_image_layer(const char* str) { return strcmp(str, "imagelayer") == 0; }
	bool _is_group_layer(const char* str) { return strcmp(str, "group") == 0; }
	bool _is_layer(const char* str) {
		return _is_tile_layer(str) || _is_object_layer(str) || _is_image_layer(str) || _is_group_layer(str);
	}

	uint32_t get_animation_duration(const std::vector<Frame>& animation)
	{
		uint32_t duration = 0;
		for (const Frame& frame : animation)
			duration += frame.duration;
		return duration;
	}

	const Tile* sample_animation(const std::vector<Frame>& animation, uint32_t time_in_ms)
	{
		uint32_t total_duration = get_animation_duration(animation);
		if (!total_duration)
			return nullptr;

		uint32_t time = time_in_ms % total_duration;
		uint32_t current_time = 0;
		for (const Frame& frame : animation)
		{
			current_time += frame.duration;
			if (time < current_time)
				return frame.tile;
		}

		return nullptr; // Should never happen.
	}

	const Tile* find_tile_by_class(const std::vector<Tile>& tiles, const std::string& class_)
	{
		if (class_.empty()) return nullptr;
		for (const Tile& tile : tiles)
			if (tile.class_ == class_)
				return &tile;
		return nullptr;
	}

	void load_assets()
	{
		_tilesets.clear();
		_templates.clear();
		_maps.clear();
		_tilesets.reserve(_MAX_TILESET_COUNT);
		_templates.reserve(_MAX_TEMPLATE_COUNT);
		_maps.reserve(_MAX_MAP_COUNT);

		// Find assets
		for (const std::filesystem::directory_entry& entry :
			std::filesystem::recursive_directory_iterator("assets/tiled"))
		{
			if (!entry.is_regular_file()) continue;
			std::string extension = entry.path().extension().string();
			if (extension == ".tsx")
			{
				assert(_tilesets.size() < _MAX_TILESET_COUNT);
				_tilesets.emplace_back().path = entry.path().lexically_normal();
			}
			else if (extension == ".tx")
			{
				assert(_templates.size() < _MAX_TEMPLATE_COUNT);
				_templates.emplace_back().path = entry.path().lexically_normal();
			}
			else if (extension == ".tmx")
			{
				assert(_maps.size() < _MAX_MAP_COUNT);
				_maps.emplace_back().path = entry.path().lexically_normal();
			}
		}

		// Load tilesets
		for (Tileset& tileset : _tilesets)
		{
			pugi::xml_document doc;
			if (!doc.load_file(tileset.path.string().c_str()))
			{
				console::log_error("Failed to load tileset: " + tileset.path.string());
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
			tileset.image_path = tileset.path.parent_path();
			tileset.image_path /= tileset_node.child("image").attribute("source").as_string();
			tileset.image_path = tileset.image_path.lexically_normal();
			tileset.reload_image();
			for (pugi::xml_node prop_node : tileset_node.child("properties").children("property"))
				_load_property(prop_node, tileset.properties.emplace_back());
			tileset.tiles.resize(tileset.tile_count);
			for (uint32_t i = 0; i < tileset.tile_count; ++i)
			{
				Tile& tile = tileset.tiles[i];
				tile.tileset = &tileset;
				tile.sprite.setTexture(tileset.image);
				sf::IntRect texture_rect;
				texture_rect.left = (i % tileset.columns) * (tileset.tile_width + tileset.spacing) + tileset.margin;
				texture_rect.top = (i / tileset.columns) * (tileset.tile_height + tileset.spacing) + tileset.margin;
				texture_rect.width = tileset.tile_width;
				texture_rect.height = tileset.tile_height;
				tile.sprite.setTextureRect(texture_rect);
			}
			for (pugi::xml_node tile_node : tileset_node.children("tile"))
			{
				Tile& tile = tileset.tiles.at(tile_node.attribute("id").as_uint());
				tile.class_ = tile_node.attribute("type").as_string(); // SIC: "type", not "class"
				for (pugi::xml_node prop_node : tile_node.child("properties").children("property"))
					_load_property(prop_node, tile.properties.emplace_back());
				for (pugi::xml_node object_node : tile_node.child("objectgroup").children("object"))
					_load_object(object_node, tile.objects.emplace_back());
				for (pugi::xml_node frame_node : tile_node.child("animation").children("frame"))
				{
					Frame& frame = tile.animation.emplace_back();
					frame.duration = frame_node.attribute("duration").as_uint();
					frame.tile = &tileset.tiles.at(frame_node.attribute("tileid").as_uint());
				}
			}
		}

		// Load templates
		for (Object& template_ : _templates)
		{
			pugi::xml_document doc;
			if (!doc.load_file(template_.path.string().c_str()))
			{
				console::log_error("Failed to load template: " + template_.path.string());
				continue;
			}
			pugi::xml_node template_node = doc.child("template");
			pugi::xml_node object_node = template_node.child("object");
			_load_object(object_node, template_);
			if (pugi::xml_node tileset_node = template_node.child("tileset"))
			{
				pugi::xml_attribute source_attribute = tileset_node.attribute("source");
				if (!source_attribute)
				{
					console::log_error("Embedded tilesets are not supported: " + template_.path.string());
					continue;
				}
				std::filesystem::path tileset_path = template_.path.parent_path();
				tileset_path /= source_attribute.as_string();
				tileset_path = tileset_path.lexically_normal();
				bool found = false;
				for (const Tileset& tileset : _tilesets)
				{
					if (tileset.path == tileset_path)
					{
						found = true;
						uint32 tile_id =
							object_node.attribute("gid").as_uint() -
							tileset_node.attribute("firstgid").as_uint();
						template_.type = ObjectType::Tile;
						template_.tile = &tileset.tiles.at(tile_id);
						break;
					}
				}
				if (!found)
					console::log_error("Failed to find tileset: " + tileset_path.string());
			}
		}

		// Load maps
		for (Map& map : _maps)
		{
			pugi::xml_document doc;
			if (!doc.load_file(map.path.string().c_str()))
			{
				console::log_error("Failed to load map: " + map.path.string());
				continue;
			}
			pugi::xml_node map_node = doc.child("map");
			map.name = map.path.stem().string();
			map.class_ = map_node.attribute("class").as_string();
			map.width = map_node.attribute("width").as_uint();
			map.height = map_node.attribute("height").as_uint();
			map.tile_width = map_node.attribute("tilewidth").as_uint();
			map.tile_height = map_node.attribute("tileheight").as_uint();
			for (pugi::xml_node prop_node : map_node.child("properties").children("property"))
				_load_property(prop_node, map.properties.emplace_back());

			// Find referenced tilesets
			struct ReferencedTileset
			{
				const Tileset* tileset = nullptr;
				uint32_t first_gid = UINT32_MAX; // The global tile ID of the first tile in the tileset.
			};
			std::vector<ReferencedTileset> referenced_tilesets;
			for (pugi::xml_node tileset_node : map_node.children("tileset"))
			{
				pugi::xml_attribute source_attribute = tileset_node.attribute("source");
				if (!source_attribute)
				{
					console::log_error("Embedded tilesets are not supported: " + map.path.string());
					continue;
				}
				std::filesystem::path tileset_path = map.path.parent_path();
				tileset_path /= source_attribute.as_string();
				tileset_path = tileset_path.lexically_normal();
				bool found = false;
				for (const Tileset& tileset : _tilesets)
				{
					if (tileset.path == tileset_path)
					{
						found = true;
						referenced_tilesets.emplace_back(&tileset, tileset_node.attribute("firstgid").as_uint());
						break;
					}
				}
				if (!found)
					console::log_error("Failed to find tileset: " + tileset_path.string());
			}

			// Load layers
			std::vector<pugi::xml_node> layer_node_stack;
			for (pugi::xml_node child_node : std::ranges::reverse_view(map_node.children()))
				if (_is_layer(child_node.name()))
					layer_node_stack.push_back(child_node);
			while (!layer_node_stack.empty())
			{
				pugi::xml_node layer_node = layer_node_stack.back();
				layer_node_stack.pop_back();

				Layer& layer = map.layers.emplace_back();
				layer.index = (uint32_t)map.layers.size() - 1u;
				layer.name = layer_node.attribute("name").as_string();
				layer.class_ = layer_node.attribute("class").as_string();
				layer.width = layer_node.attribute("width").as_uint();
				layer.height = layer_node.attribute("height").as_uint();
				for (pugi::xml_node prop_node : layer_node.child("properties").children("property"))
					_load_property(prop_node, layer.properties.emplace_back());
				if (_is_tile_layer(layer_node.name()))
				{
					pugi::xml_node data_node = layer_node.child("data");
					if (strcmp(data_node.attribute("encoding").as_string(), "csv") != 0)
					{
						console::log_error("Only CSV encoding is supported: " + map.path.string());
						layer.width = 0;
						layer.height = 0;
						continue;
					}
					std::vector<uint32_t> gids;
					gids.reserve(layer.width * layer.height);
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
					{
						if (!gids[i]) continue; // 0 means no tile
						for (const auto& [tileset, first_gid] : referenced_tilesets)
						{
							if (gids[i] >= first_gid && gids[i] < first_gid + tileset->tile_count)
							{
								layer.tiles[i] = &tileset->tiles[gids[i] - first_gid];
								break;
							}
						}
					}
				}
				else if (_is_object_layer(layer_node.name()))
				{
					for (pugi::xml_node object_node : layer_node.children("object"))
					{
						Object& object = layer.objects.emplace_back();
						if (pugi::xml_attribute template_attribute = object_node.attribute("template"))
						{
							std::filesystem::path template_path = map.path.parent_path();
							template_path /= template_attribute.as_string();
							template_path = template_path.lexically_normal();
							bool found = false;
							for (const Object& template_ : _templates)
							{
								if (template_.path == template_path)
								{
									found = true;
									object = template_;
									break;
								}
							}
							if (!found)
								console::log_error("Failed to find template: " + template_path.string());
						}
						_load_object(object_node, object);
						if (pugi::xml_attribute gid_attribute = object_node.attribute("gid"))
						{
							uint32_t gid = gid_attribute.as_uint();
							bool found = false;
							for (const auto& [tileset, first_gid] : referenced_tilesets)
							{
								if (gid >= first_gid && gid < first_gid + tileset->tile_count)
								{
									found = true;
									object.type = ObjectType::Tile;
									object.tile = &tileset->tiles[gid - first_gid];
									break;
								}
							}
							if (!found)
								console::log_error("Failed to find tile with GID: " + std::to_string(gid));
						}
					}
				}
				else if (_is_image_layer(layer_node.name()))
				{
					// TODO
				}
				else if (_is_group_layer(layer_node.name()))
				{
					for (pugi::xml_node child_node : std::ranges::reverse_view(layer_node.children()))
						if (_is_layer(child_node.name()))
							layer_node_stack.push_back(child_node);
				}
				else
				{
					assert(false);
				}
			}
		}
	}

	void unload_assets()
	{
		_tilesets.clear();
		_templates.clear();
		_maps.clear();
	}

	const std::vector<Tileset>& get_tilesets() {
		return _tilesets;
	}

	const std::vector<Object>& get_templates() {
		return _templates;
	}

	const std::vector<Map>& get_maps() {
		return _maps;
	}

	bool Tileset::reload_image()
	{
		bool success = image.loadFromFile(image_path.string());
		if (!success)
			console::log_error("Failed to load tileset image: " + image_path.string());
		return success;
	}
}