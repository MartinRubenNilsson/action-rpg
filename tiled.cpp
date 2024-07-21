#include "stdafx.h"
#include "tiled.h"
#include <pugixml.hpp>
#include "console.h"
#include "filesystem.h"

namespace tiled
{
	constexpr unsigned int _FLIP_HORIZONTAL_BIT = 0x80000000;
	constexpr unsigned int _FLIP_VERTICAL_BIT   = 0x40000000;
	constexpr unsigned int _FLIP_DIAGONAL_BIT   = 0x20000000;
	constexpr unsigned int _FLIP_ROTATE_120_BIT = 0x10000000;
	constexpr unsigned int _FLIP_FLAGS_BITMASK =
		_FLIP_HORIZONTAL_BIT | _FLIP_VERTICAL_BIT | _FLIP_DIAGONAL_BIT | _FLIP_ROTATE_120_BIT;

	unsigned int _get_gid(unsigned int gid_with_flip_flags) {
		return gid_with_flip_flags & ~_FLIP_FLAGS_BITMASK;
	}

	uint8_t _get_flip_flags(unsigned int gid_with_flip_flags)
	{
		uint8_t flip_flags = 0;
		if (gid_with_flip_flags & _FLIP_HORIZONTAL_BIT) flip_flags |= FLIP_HORIZONTAL;
		if (gid_with_flip_flags & _FLIP_VERTICAL_BIT)   flip_flags |= FLIP_VERTICAL;
		if (gid_with_flip_flags & _FLIP_DIAGONAL_BIT)   flip_flags |= FLIP_DIAGONAL;
		if (gid_with_flip_flags & _FLIP_ROTATE_120_BIT) flip_flags |= FLIP_ROTATE_120;
		return flip_flags;
	}

	std::vector<Tileset> _tilesets;
	std::vector<Object> _templates;
	std::vector<Map> _maps;

	void _load_properties(const pugi::xml_node& node, Properties& properties)
	{
		for (pugi::xml_node prop_node : node.child("properties").children("property")) {
			const pugi::char_t* name = prop_node.attribute("name").as_string();
			const pugi::char_t* type = prop_node.attribute("type").as_string("string"); // default type is string
			pugi::xml_attribute value = prop_node.attribute("value");
			if (strcmp(type, "string") == 0) {
				properties.set_string(name, value.as_string());
			} else if (strcmp(type, "int") == 0) {
				properties.set_int(name, value.as_int());
			} else if (strcmp(type, "float") == 0) {
				properties.set_float(name, value.as_float());
			} else if (strcmp(type, "bool") == 0) {
				properties.set_bool(name, value.as_bool());
			} else if (strcmp(type, "color") == 0) {
				// TODO
			} else if (strcmp(type, "file") == 0) {
				// TODO
			} else if (strcmp(type, "object") == 0) {
				unsigned int id = value.as_uint(); // 0 when no object is referenced
				properties.set_entity(name, id ? (entt::entity)id : entt::null);
			} else if (strcmp(type, "class") == 0) {
				// TODO
			} else {
				assert(false);
			}
		}
	}

	std::vector<Vector2f> _load_points(const pugi::xml_node& node)
	{
		// Example: <polygon points="0,0 0,16 16,16"/>
		std::vector<Vector2f> points;
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
		_load_properties(node, object.properties);
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

	bool _is_tile_layer(const char* str)
	{
		return strcmp(str, "layer") == 0;
	
	}
	bool _is_object_layer(const char* str)
	{
		return strcmp(str, "objectgroup") == 0;
	}

	bool _is_image_layer(const char* str)
	{
		return strcmp(str, "imagelayer") == 0;
	}

	bool _is_group_layer(const char* str)
	{
		return strcmp(str, "group") == 0;
	}

	bool _is_layer(const char* str)
	{
		return _is_tile_layer(str) || _is_object_layer(str) || _is_image_layer(str) || _is_group_layer(str);
	}

	// Maps each ASCII character to its corresponding 6-bit Base64 value,
	// or 0 if the character is not part of the Base64 alphabet.
	const unsigned char _BASE64_DECODING_TABLE[256] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		/* + */ 62, 0, 0, 0, /* / */ 63, /* 0-9 */ 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0, 0, 0, /* = */ 0, 0, 0, 0, 0,
		/* A-Z */ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0, 0, 0, 0, 0, 0,
		/* a-z */ 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51 };

	void _base64_decode(const unsigned char* input, size_t input_size, unsigned char* output, size_t output_size)
	{
		if (!input || !output) return;
		size_t i = 0;
		size_t j = 0;
		while (i + 3 < input_size && j + 2 < output_size) {
			unsigned char a = _BASE64_DECODING_TABLE[input[i++]];
			unsigned char b = _BASE64_DECODING_TABLE[input[i++]];
			unsigned char c = _BASE64_DECODING_TABLE[input[i++]];
			unsigned char d = _BASE64_DECODING_TABLE[input[i++]];
			output[j++] = (a << 2) | (b >> 4);
			output[j++] = (b << 4) | (c >> 2);
			output[j++] = (c << 6) | d;
		}
	}

	void load_assets(const std::string& dir)
	{
		// FIND AND ALLOCATE ROOM FOR TILESETS, TEMPLATES, AND MAPS
		
		for (const filesystem::File& file : filesystem::get_files_in_directory(dir)) {
			switch (file.format) {
			case filesystem::FileFormat::TiledMap: {
				_maps.emplace_back(file.path);
			} break;
			case filesystem::FileFormat::TiledTileset: {
				_tilesets.emplace_back(file.path);
			} break;
			case filesystem::FileFormat::TiledTemplate: {
				_templates.emplace_back(file.path);
			} break;
			}
		}

		// LOAD TILESETS
		
		for (Tileset& tileset : _tilesets) {
			pugi::xml_document doc;
			if (!doc.load_file(tileset.path.c_str())) {
				console::log_error("Failed to load tileset: " + tileset.path);
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
			tileset.image_path = filesystem::get_parent_path(tileset.path);
			tileset.image_path += '/';
			tileset.image_path += tileset_node.child("image").attribute("source").as_string();
			tileset.image_path = filesystem::get_normalized_path(tileset.image_path);
			_load_properties(tileset_node, tileset.properties);
			tileset.tiles.resize(tileset.tile_count);
			for (unsigned int id = 0; id < tileset.tile_count; ++id) {
				Tile& tile = tileset.tiles[id];
				tile.tileset = &tileset;
				tile.id = id;
				tile.left = (id % tileset.columns) * (tileset.tile_width + tileset.spacing) + tileset.margin;
				tile.top = (id / tileset.columns) * (tileset.tile_height + tileset.spacing) + tileset.margin;
				tile.width = tileset.tile_width;
				tile.height = tileset.tile_height;
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
				wangset.tile_id = wangset_node.attribute("tile").as_int(); //-1 in case of no tile
				for (pugi::xml_node wangcolor_node : wangset_node.children("wangcolor")) {
					WangColor& wangcolor = wangset.colors.emplace_back();
					wangcolor.name = wangcolor_node.attribute("name").as_string();
					wangcolor.class_ = wangcolor_node.attribute("class").as_string();
					_load_properties(wangcolor_node, wangcolor.properties);
					wangset.tile_id = wangcolor_node.attribute("tile").as_int(); //-1 in case of no tile
					wangcolor.probability = wangcolor_node.attribute("probability").as_float();
					{
						std::string color_str = wangcolor_node.attribute("color").as_string();
						color_str.erase(color_str.begin()); // remove leading '#'
						color_str += "ff"; // add alpha channel
						const unsigned long color = std::stoul(color_str, nullptr, 16);
						wangcolor.color.r = (color >> 24) & 0xFF;
						wangcolor.color.g = (color >> 16) & 0xFF;
						wangcolor.color.b = (color >> 8) & 0xFF;
						wangcolor.color.a = color & 0xFF;
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
						if (unsigned int wang_id = std::stoul(token)) // 0 means unset, 1 means first color, etc.
							wangtile.wangcolors[i] = &wangset.colors.at(wang_id - 1);
						++i;
					}
				}
			}
		}

		// LOAD TEMPLATES
		
		for (Object& template_ : _templates) {
			pugi::xml_document doc;
			if (!doc.load_file(template_.path.c_str())) {
				console::log_error("Failed to load template: " + template_.path);
				continue;
			}
			pugi::xml_node template_node = doc.child("template");
			pugi::xml_node object_node = template_node.child("object");
			_load_object(object_node, template_);
			if (pugi::xml_node tileset_node = template_node.child("tileset")) {
				pugi::xml_attribute source_attribute = tileset_node.attribute("source");
				if (!source_attribute) {
					console::log_error("Embedded tilesets are not supported: " + template_.path);
					continue;
				}
				std::string tileset_path = filesystem::get_parent_path(template_.path);
				tileset_path += '/';
				tileset_path += source_attribute.as_string();
				tileset_path = filesystem::get_normalized_path(tileset_path);
				unsigned int gid_with_flip_flags = object_node.attribute("gid").as_uint();
				unsigned int gid = _get_gid(gid_with_flip_flags);
				unsigned int id = gid - tileset_node.attribute("firstgid").as_uint();
				const Tile* tile = nullptr;
				for (const Tileset& tileset : _tilesets) {
					if (tileset.path == tileset_path) {
						tile = &tileset.tiles.at(id);
						break;
					}
				}
				if (!tile) {
					console::log_error("Failed to find tile with GID: " + std::to_string(gid));
					continue;
				}
				template_.tile = tile;
				template_.type = ObjectType::Tile;
				template_.flip_flags = _get_flip_flags(gid_with_flip_flags);
			}
		}

		// LOAD MAPS

		for (Map& map : _maps) {
			pugi::xml_document doc;
			if (!doc.load_file(map.path.c_str())) {
				console::log_error("Failed to load map: " + map.path);
				continue;
			}
			pugi::xml_node map_node = doc.child("map");
			map.name = filesystem::get_stem(map.path);
			map.class_ = map_node.attribute("class").as_string();
			map.width = map_node.attribute("width").as_uint();
			map.height = map_node.attribute("height").as_uint();
			map.tile_width = map_node.attribute("tilewidth").as_uint();
			map.tile_height = map_node.attribute("tileheight").as_uint();
			_load_properties(map_node, map.properties);
			struct ReferencedTileset
			{
				const Tileset* tileset = nullptr;
				unsigned int first_gid = UINT32_MAX; // The global tile ID of the first tile in the tileset.
			};
			std::vector<ReferencedTileset> referenced_tilesets;
			for (pugi::xml_node tileset_node : map_node.children("tileset")) {
				pugi::xml_attribute source_attribute = tileset_node.attribute("source");
				if (!source_attribute) {
					console::log_error("Embedded tilesets are not supported: " + map.path);
					continue;
				}
				std::string tileset_path = filesystem::get_parent_path(map.path);
				tileset_path += '/';
				tileset_path += source_attribute.as_string();
				tileset_path = filesystem::get_normalized_path(tileset_path);
				bool found = false;
				for (const Tileset& tileset : _tilesets) {
					if (tileset.path == tileset_path) {
						found = true;
						referenced_tilesets.emplace_back(&tileset, tileset_node.attribute("firstgid").as_uint());
						break;
					}
				}
				if (!found) {
					console::log_error("Failed to find tileset: " + tileset_path);
				}
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
					const pugi::char_t* encoding = data_node.attribute("encoding").as_string();
					// Each unsigned 32-bit integer in the data array stores a global tile ID
					// in the lower 28 bits and flip flags in the higher 4 bits.
					std::vector<unsigned int> data; 
					if (strcmp(encoding, "csv") == 0) {
						std::istringstream ss(data_node.text().as_string());
						std::string token;
						data.reserve(layer.width * layer.height);
						while (std::getline(ss, token, ',')) {
							data.push_back(std::stoul(token));
						}
					} else if (strcmp(encoding, "base64") == 0) {
						data.resize(layer.width * layer.height);
						const char* base64_string = data_node.text().as_string();
						// Skip leading whitespace
						while (base64_string && *base64_string && isspace(*base64_string)) {
							++base64_string;
						}
						size_t string_size = strlen(base64_string);
						unsigned char* buffer = (unsigned char*)data.data();
						size_t buffer_size = data.size() * sizeof(unsigned int);
						_base64_decode((unsigned char*)base64_string, string_size, buffer, buffer_size);
					} else {
						console::log_error(
							"Unknown Tiled map tile layer encoding.\n"
							"  Map: " + map.path + "\n"
							"  Layer: " + layer.name);
						layer.width = 0;
						layer.height = 0;
						continue;
					}
					assert(data.size() == layer.width * layer.height);
					layer.tiles.resize(data.size());
					for (size_t i = 0; i < data.size(); ++i) {
						unsigned int gid_with_flip_flag = data[i];
						if (!gid_with_flip_flag) continue; // 0 means no tile
						unsigned int gid = _get_gid(gid_with_flip_flag);
						const Tile* tile = nullptr;
						for (const auto& [tileset, first_gid] : referenced_tilesets) {
							if (first_gid <= gid && gid < first_gid + tileset->tile_count) {
								tile = &tileset->tiles[gid - first_gid];
								break;
							}
						}
						if (!tile) {
							console::log_error("Failed to find tile with GID: " + std::to_string(gid));
							continue;
						}
						layer.tiles[i] = { tile, _get_flip_flags(gid_with_flip_flag) };
					}
				} else if (_is_object_layer(layer_node.name())) {
					for (pugi::xml_node object_node : layer_node.children("object")) {
						Object& object = layer.objects.emplace_back();
						if (pugi::xml_attribute template_attribute = object_node.attribute("template")) {
							std::string template_path = filesystem::get_parent_path(map.path);
							template_path += '/';
							template_path += template_attribute.as_string();
							template_path = filesystem::get_normalized_path(template_path);
							bool found = false;
							for (const Object& template_ : _templates) {
								if (template_.path == template_path) {
									found = true;
									object = template_;
									break;
								}
							}
							if (!found) {
								console::log_error("Failed to find template: " + template_path);
							}
						}
						_load_object(object_node, object);
						if (pugi::xml_attribute gid_attribute = object_node.attribute("gid")) {
							unsigned int gid_with_flip_flags = gid_attribute.as_uint();
							unsigned int gid = _get_gid(gid_with_flip_flags);
							const Tile* tile = nullptr;
							for (const auto& [tileset, first_gid] : referenced_tilesets) {
								if (gid >= first_gid && gid < first_gid + tileset->tile_count) {
									tile = &tileset->tiles[gid - first_gid];
									break;
								}
							}
							if (!tile) {
								console::log_error("Failed to find tile with GID: " + std::to_string(gid));
								continue;
							}
							object.tile = tile; 
							object.type = ObjectType::Tile;
							object.flip_flags = _get_flip_flags(gid_with_flip_flags);
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

	std::span<const Map> get_maps() {
		return _maps;
	}

	std::span<const Tileset> get_tilesets() {
		return _tilesets;
	}

	std::span<const Object> get_templates() {
		return _templates;
	}

	const Map* find_map_by_name(const std::string& name)
	{
		if (name.empty()) return nullptr;
		for (const Map& map : _maps)
			if (map.name == name)
				return &map;
		return nullptr;
	}

	const Tileset* find_tileset_by_name(const std::string& name)
	{
		if (name.empty()) return nullptr;
		for (const Tileset& tileset : _tilesets)
			if (tileset.name == name)
				return &tileset;
		return nullptr;
	}

	const Object* find_object_by_name(const Layer& layer, const std::string& name)
	{
		if (name.empty()) return nullptr;
		for (const Object& object : layer.objects)
			if (object.name == name)
				return &object;
		return nullptr;
	}

	const Object* find_object_by_name(const Map& map, const std::string& name)
	{
		if (name.empty()) return nullptr;
		for (const Layer& layer : map.layers)
			for (const Object& object : layer.objects)
				if (object.name == name)
					return &object;
		return nullptr;
	}

	const Tile* find_tile_by_class(const Tileset& tileset, const std::string& class_)
	{
		if (class_.empty()) return nullptr;
		for (const Tile& tile : tileset.tiles)
			if (tile.class_ == class_)
				return &tile;
		return nullptr;
	}
}