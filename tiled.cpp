#include "stdafx.h"
#include "tiled.h"
#include <pugixml.hpp>
#include "console.h"

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

	void _load_properties(const pugi::xml_node& node, std::vector<Property>& properties)
	{
		for (pugi::xml_node prop_node : node.child("properties").children("property"))
			_load_property(prop_node, properties.emplace_back());
	}

	std::vector<sf::Vector2f> _load_points(const pugi::xml_node& node)
	{
		// Example: <polygon points="0,0 0,16 16,16"/>
		std::vector<sf::Vector2f> points;
		std::istringstream ss(node.attribute("points").as_string());
		std::string token;
		while (std::getline(ss, token, ' ')) {
			size_t comma = token.find(',');
			assert(comma != std::string::npos);
			float x = std::stof(token.substr(0, comma));
			float y = std::stof(token.substr(comma + 1));
			points.emplace_back(x, y);
		}
		return points;
	}

	void _load_object(const pugi::xml_node& node, Object& object)
	{
		for (pugi::xml_node prop_node : node.child("properties").children("property")) {
			std::string prop_name = prop_node.attribute("name").as_string();
			bool found = false;
			for (Property& prop : object.properties) {
				if (prop.name == prop_name) {
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
		if (node.child("ellipse")) {
			object.type = ObjectType::Ellipse;
		} else if (node.child("point")) {
			object.type = ObjectType::Point;
		} else if (pugi::xml_node polygon = node.child("polygon")) {
			object.type = ObjectType::Polygon;
			object.points = _load_points(polygon);
		} else if (pugi::xml_node polyline = node.child("polyline")) {
			object.type = ObjectType::Polyline;
			object.points = _load_points(polyline);
		}
	}

	bool _is_tile_layer(const char* str) { return strcmp(str, "layer") == 0; }
	bool _is_object_layer(const char* str) { return strcmp(str, "objectgroup") == 0; }
	bool _is_image_layer(const char* str) { return strcmp(str, "imagelayer") == 0; }
	bool _is_group_layer(const char* str) { return strcmp(str, "group") == 0; }
	bool _is_layer(const char* str) {
		return _is_tile_layer(str) || _is_object_layer(str) || _is_image_layer(str) || _is_group_layer(str);
	}

	void load_assets(const std::filesystem::path& dir)
	{
		unload_assets();

		// Find assets
		for (const std::filesystem::directory_entry& entry :
			std::filesystem::recursive_directory_iterator(dir))
		{
			if (!entry.is_regular_file()) continue;
			std::string extension = entry.path().extension().string();
			if (extension == ".tsx")
				_tilesets.emplace_back().path = entry.path().lexically_normal();
			else if (extension == ".tx")
				_templates.emplace_back().path = entry.path().lexically_normal();
			else if (extension == ".tmx")
				_maps.emplace_back().path = entry.path().lexically_normal();
		}

		// Load tilesets
		for (Tileset& tileset : _tilesets) {
			pugi::xml_document doc;
			if (!doc.load_file(tileset.path.string().c_str())) {
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
			_load_properties(tileset_node, tileset.properties);
			tileset.tiles.resize(tileset.tile_count);
			for (uint32_t i = 0; i < tileset.tile_count; ++i) {
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
			for (pugi::xml_node tile_node : tileset_node.children("tile")) {
				Tile& tile = tileset.tiles.at(tile_node.attribute("id").as_uint());
				tile.class_ = tile_node.attribute("type").as_string(); // SIC: "type", not "class"
				_load_properties(tile_node, tile.properties);
				for (pugi::xml_node object_node : tile_node.child("objectgroup").children("object"))
					_load_object(object_node, tile.objects.emplace_back());
				for (pugi::xml_node frame_node : tile_node.child("animation").children("frame")) {
					Frame& frame = tile.animation.emplace_back();
					frame.duration = frame_node.attribute("duration").as_uint();
					frame.tile = &tileset.tiles.at(frame_node.attribute("tileid").as_uint());
				}
			}
			for (pugi::xml_node wangset_node : tileset_node.child("wangsets").children("wangset")) {
				WangSet& wangset = tileset.wangsets.emplace_back();
				wangset.name = wangset_node.attribute("name").as_string();
				wangset.class_ = wangset_node.attribute("class").as_string();
				_load_properties(wangset_node, wangset.properties);
				{
					int tile_id = wangset_node.attribute("tile").as_int(); //-1 in case of no tile
					if (0 <= tile_id && tile_id < tileset.tiles.size())
						wangset.tile = &tileset.tiles[tile_id];
				}
				for (pugi::xml_node wangcolor_node : wangset_node.children("wangcolor")) {
					WangColor& wangcolor = wangset.colors.emplace_back();
					wangcolor.name = wangcolor_node.attribute("name").as_string();
					wangcolor.class_ = wangcolor_node.attribute("class").as_string();
					_load_properties(wangcolor_node, wangcolor.properties);
					{
						int tile_id = wangcolor_node.attribute("tile").as_int(); //-1 in case of no tile
						if (0 <= tile_id && tile_id < tileset.tiles.size())
							wangcolor.tile = &tileset.tiles[tile_id];
					}
					wangcolor.probability = wangcolor_node.attribute("probability").as_float();
					{
						std::string color_str = wangcolor_node.attribute("color").as_string();
						color_str.erase(color_str.begin()); // remove leading '#'
						color_str += "ff"; // add alpha channel
						wangcolor.color = sf::Color(std::stoul(color_str, nullptr, 16));
					}
				}
				for (pugi::xml_node wangtile_node : wangset_node.children("wangtile")) {
					Tile& tile = tileset.tiles.at(wangtile_node.attribute("tileid").as_uint());
					WangTile& wangtile = tile.wangtiles.emplace_back();
					std::istringstream ss(wangtile_node.attribute("wangid").as_string());
					std::string token;
					size_t i = 0;
					while (std::getline(ss, token, ',')) {
						assert(i < WangTile::COUNT);
						if (uint32_t wang_id = std::stoul(token)) // 0 means unset, 1 means first color, etc.
							wangtile.wangcolors[i] = &wangset.colors.at(wang_id - 1);
						++i;
					}
				}
			}
		}

		// Load templates
		for (Object& template_ : _templates) {
			pugi::xml_document doc;
			if (!doc.load_file(template_.path.string().c_str())) {
				console::log_error("Failed to load template: " + template_.path.string());
				continue;
			}
			pugi::xml_node template_node = doc.child("template");
			pugi::xml_node object_node = template_node.child("object");
			_load_object(object_node, template_);
			if (pugi::xml_node tileset_node = template_node.child("tileset")) {
				pugi::xml_attribute source_attribute = tileset_node.attribute("source");
				if (!source_attribute) {
					console::log_error("Embedded tilesets are not supported: " + template_.path.string());
					continue;
				}
				std::filesystem::path tileset_path = template_.path.parent_path();
				tileset_path /= source_attribute.as_string();
				tileset_path = tileset_path.lexically_normal();
				bool found = false;
				for (const Tileset& tileset : _tilesets) {
					if (tileset.path == tileset_path) {
						found = true;
						uint32 tile_id =
							object_node.attribute("gid").as_uint() -
							tileset_node.attribute("firstgid").as_uint();
						template_.tile = &tileset.tiles.at(tile_id);
						template_.type = ObjectType::Tile;
						break;
					}
				}
				if (!found)
					console::log_error("Failed to find tileset: " + tileset_path.string());
			}
		}

		// Load maps
		for (Map& map : _maps) {
			pugi::xml_document doc;
			if (!doc.load_file(map.path.string().c_str())) {
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
			_load_properties(map_node, map.properties);
			struct ReferencedTileset
			{
				const Tileset* tileset = nullptr;
				uint32_t first_gid = UINT32_MAX; // The global tile ID of the first tile in the tileset.
			};
			std::vector<ReferencedTileset> referenced_tilesets;
			for (pugi::xml_node tileset_node : map_node.children("tileset")) {
				pugi::xml_attribute source_attribute = tileset_node.attribute("source");
				if (!source_attribute) {
					console::log_error("Embedded tilesets are not supported: " + map.path.string());
					continue;
				}
				std::filesystem::path tileset_path = map.path.parent_path();
				tileset_path /= source_attribute.as_string();
				tileset_path = tileset_path.lexically_normal();
				bool found = false;
				for (const Tileset& tileset : _tilesets) {
					if (tileset.path == tileset_path) {
						found = true;
						referenced_tilesets.emplace_back(&tileset, tileset_node.attribute("firstgid").as_uint());
						break;
					}
				}
				if (!found)
					console::log_error("Failed to find tileset: " + tileset_path.string());
			}
			std::vector<pugi::xml_node> layer_node_stack;
			for (pugi::xml_node child_node : std::ranges::reverse_view(map_node.children()))
				if (_is_layer(child_node.name()))
					layer_node_stack.push_back(child_node);
			while (!layer_node_stack.empty()) {
				pugi::xml_node layer_node = layer_node_stack.back();
				layer_node_stack.pop_back();
				Layer& layer = map.layers.emplace_back();
				layer.name = layer_node.attribute("name").as_string();
				layer.class_ = layer_node.attribute("class").as_string();
				layer.width = layer_node.attribute("width").as_uint();
				layer.height = layer_node.attribute("height").as_uint();
				layer.visible = layer_node.attribute("visible").as_bool(true);
				_load_properties(layer_node, layer.properties);
				if (_is_tile_layer(layer_node.name())) {
					pugi::xml_node data_node = layer_node.child("data");
					if (strcmp(data_node.attribute("encoding").as_string(), "csv") != 0) {
						console::log_error("Only CSV encoding is supported: " + map.path.string());
						layer.width = 0;
						layer.height = 0;
						continue;
					}
					std::vector<uint32_t> gids;
					gids.reserve(layer.width * layer.height);
					{
						std::istringstream ss(data_node.text().as_string());
						std::string token;
						while (std::getline(ss, token, ',')) {
							gids.push_back(std::stoul(token));
						}
					}
					assert(gids.size() == layer.width * layer.height);
					layer.tiles.resize(gids.size());
					for (size_t i = 0; i < gids.size(); ++i) {
						uint32_t gid = gids[i];
						if (!gid) continue; // 0 means no tile
						for (const auto& [tileset, first_gid] : referenced_tilesets) {
							if (gid >= first_gid && gid < first_gid + tileset->tile_count) {
								layer.tiles[i] = &tileset->tiles[gid - first_gid];
								break;
							}
						}
						//if (!layer.tiles[i])
						//	console::log_error("Failed to find tile with GID: " + std::to_string(gid));
					}
				} else if (_is_object_layer(layer_node.name())) {
					for (pugi::xml_node object_node : layer_node.children("object")) {
						Object& object = layer.objects.emplace_back();
						if (pugi::xml_attribute template_attribute = object_node.attribute("template")) {
							std::filesystem::path template_path = map.path.parent_path();
							template_path /= template_attribute.as_string();
							template_path = template_path.lexically_normal();
							bool found = false;
							for (const Object& template_ : _templates) {
								if (template_.path == template_path) {
									found = true;
									object = template_;
									break;
								}
							}
							if (!found)
								console::log_error("Failed to find template: " + template_path.string());
						}
						_load_object(object_node, object);
						if (pugi::xml_attribute gid_attribute = object_node.attribute("gid")) {
							uint32_t gid = gid_attribute.as_uint();
							bool found = false;
							for (const auto& [tileset, first_gid] : referenced_tilesets) {
								if (gid >= first_gid && gid < first_gid + tileset->tile_count) {
									found = true;
									object.tile = &tileset->tiles[gid - first_gid];
									object.type = ObjectType::Tile;
									break;
								}
							}
							if (!found)
								console::log_error("Failed to find tile with GID: " + std::to_string(gid));
						}
					}
				} else if (_is_image_layer(layer_node.name())) {
					// TODO
				} else if (_is_group_layer(layer_node.name())) {
					for (pugi::xml_node child_node : std::ranges::reverse_view(layer_node.children()))
						if (_is_layer(child_node.name()))
							layer_node_stack.push_back(child_node);
				} else {
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