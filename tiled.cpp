#include "stdafx.h"
#include "tiled.h"
#include <pugixml.hpp>
#include <zlib.h>
#include "pool.h"
#include "console.h"
#include "filesystem.h"

namespace tiled
{
	constexpr unsigned int _FLIP_HORIZONTAL_BIT = 0x80000000;
	constexpr unsigned int _FLIP_VERTICAL_BIT   = 0x40000000;
	constexpr unsigned int _FLIP_DIAGONAL_BIT   = 0x20000000;
	constexpr unsigned int _FLIP_ROTATE_120_BIT = 0x10000000;
	constexpr unsigned int _FLIP_FLAGS_BITMASK = _FLIP_HORIZONTAL_BIT | _FLIP_VERTICAL_BIT | _FLIP_DIAGONAL_BIT | _FLIP_ROTATE_120_BIT;

	unsigned int _get_gid(unsigned int gid_with_flip_flags)
	{
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

	Pool<Map> _maps;
	std::unordered_map<std::string, Handle<Map>> _path_to_map_handle;
	Pool<Tileset> _tilesets;
	std::unordered_map<std::string, Handle<Tileset>> _path_to_tileset_handle;
	Pool<Object> _templates;
	std::unordered_map<std::string, Handle<Object>> _path_to_template_handle;

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
		if (pugi::xml_attribute id = node.attribute("id")) {
			object.entity = (entt::entity)id.as_uint();
		}
		if (pugi::xml_attribute name = node.attribute("name")) {
			object.name = name.as_string();
		}
		if (pugi::xml_attribute type = node.attribute("type")) { // SIC: "type", not "class"
			object.class_ = type.as_string();
		}
		if (pugi::xml_attribute x = node.attribute("x")) {
			object.position.x = x.as_float();
		}
		if (pugi::xml_attribute y = node.attribute("y")) {
			object.position.y = y.as_float();
		}
		if (pugi::xml_attribute width = node.attribute("width")) {
			object.size.x = width.as_float();
		}
		if (pugi::xml_attribute height = node.attribute("height")) {
			object.size.y = height.as_float();
		}
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

	bool _string_to_layer_type(const char* str, LayerType& type)
	{
		if (strcmp(str, "layer") == 0) {
			type = LayerType::Tile;
		} else if (strcmp(str, "objectgroup") == 0) {
			type = LayerType::Object;
		} else if (strcmp(str, "imagelayer") == 0) {
			type = LayerType::Image;
		} else if (strcmp(str, "group") == 0) {
			type = LayerType::Group;
		} else {
			return false;
		}
		return true;
	}

	// Maps each ASCII character to its corresponding 6-bit Base64 value,
	// or 0 if the character is not part of the Base64 alphabet.
	const unsigned char _BASE64_DECODING_TABLE[256] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		/* + */ 62, 0, 0, 0, /* / */ 63, /* 0-9 */ 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0, 0, 0, /* = */ 0, 0, 0, 0, 0,
		/* A-Z */ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0, 0, 0, 0, 0, 0,
		/* a-z */ 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51 };

	void _base64_decode(unsigned char* dest, size_t dest_len, const unsigned char* source, size_t source_len)
	{
		if (!dest || !source) return;
		size_t i = 0;
		size_t j = 0;
		while (i + 3 < source_len && j + 2 < dest_len) {
			unsigned char a = _BASE64_DECODING_TABLE[source[i++]];
			unsigned char b = _BASE64_DECODING_TABLE[source[i++]];
			unsigned char c = _BASE64_DECODING_TABLE[source[i++]];
			unsigned char d = _BASE64_DECODING_TABLE[source[i++]];
			dest[j++] = (a << 2) | (b >> 4);
			dest[j++] = (b << 4) | (c >> 2);
			dest[j++] = (c << 6) | d;
		}
	}

	void _load_layer_recursive(Map& map, pugi::xml_node node)
	{
		// PITFALL: node may be of type <tileset>, and we are only interested in layers,
		// hence we have an early return if we fail to convert the node name to a LayerType.

		LayerType type;
		if (!_string_to_layer_type(node.name(), type)) return;

		Layer& layer = map.layers.emplace_back();
		layer.type = type;
		layer.name = node.attribute("name").as_string();
		layer.class_ = node.attribute("class").as_string();
		layer.width = node.attribute("width").as_uint();
		layer.height = node.attribute("height").as_uint();
		layer.visible = node.attribute("visible").as_bool(true);
		_load_properties(node, layer.properties);

		switch (layer.type) {
		case LayerType::Tile: {
			pugi::xml_node data_node = node.child("data");
			const pugi::char_t* encoding = data_node.attribute("encoding").as_string();
			layer.tiles.resize(layer.width * layer.height);
			if (strcmp(encoding, "csv") == 0) {
				std::istringstream ss(data_node.text().as_string());
				std::string token;
				size_t i = 0;
				while (i < layer.tiles.size() && std::getline(ss, token, ',')) {
					layer.tiles[i++].value = std::stoul(token);
				}
			} else if (strcmp(encoding, "base64") == 0) {

				std::vector<unsigned char> compressed_data;

				// DECODE BASE64
				{
					std::string_view base64_string = data_node.text().as_string();
					// Skip leading whitespace
					while (!base64_string.empty() && isspace(base64_string.front())) {
						base64_string.remove_prefix(1);
					}
					// Skip trailing whitespace
					while (!base64_string.empty() && isspace(base64_string.back())) {
						base64_string.remove_suffix(1);
					}
					// Base64 string length must be a multiple of 4
					assert(base64_string.size() % 4 == 0);
					compressed_data.resize((base64_string.size() / 4) * 3);
					_base64_decode(compressed_data.data(), compressed_data.size(),
						(const unsigned char*)base64_string.data(), base64_string.size());
				}

				// DECOMPRESS

				const pugi::char_t* compression = data_node.attribute("compression").as_string();
				if (strcmp(compression, "zlib") == 0) {
					uLongf data_size = (uLongf)(layer.tiles.size() * sizeof(unsigned int));
					uncompress((Bytef*)layer.tiles.data(), &data_size,
						(Bytef*)compressed_data.data(), (uLongf)compressed_data.size());
				} else if (strcmp(compression, "") == 0) { // No compression
					// We may have up to 3 bytes of padding at the end of compressed_data
					// as a result of the Base64 decoding process; these can be discarded.
					assert(compressed_data.size() >= layer.tiles.size() * sizeof(unsigned int));
					memcpy(layer.tiles.data(), compressed_data.data(), layer.tiles.size() * sizeof(unsigned int));
				} else {
					console::log_error(
						"Unknown Tiled map tile layer compression: " + std::string(compression) + "\n"
						"  Map: " + map.path + "\n"
						"  Layer: " + layer.name);
				}

			} else {
				console::log_error(
					"Unknown Tiled map tile layer encoding.\n"
					"  Map: " + map.path + "\n"
					"  Layer: " + layer.name);
			}
		} break;
		case LayerType::Object: {
			for (pugi::xml_node object_node : node.children("object")) {
				Object& object = layer.objects.emplace_back();
				if (pugi::xml_attribute template_attribute = object_node.attribute("template")) {
					std::string template_path = filesystem::get_parent_path(map.path);
					template_path += '/';
					template_path += template_attribute.as_string();
					template_path = filesystem::get_normalized_path(template_path);
					const Object* template_ = _templates.get(load_template(template_path));
					if (!template_) {
						console::log_error("Failed to find template: " + template_path);
						continue;
					}
					object = *template_;
				}
				_load_object(object_node, object);
				if (pugi::xml_attribute gid_attribute = object_node.attribute("gid")) {
					unsigned int gid_with_flip_flags = gid_attribute.as_uint();
					unsigned int gid = _get_gid(gid_with_flip_flags);
					const Tile* tile = nullptr;
					for (const TilesetRef& tileset_ref : map.tilesets) {
						if (gid < tileset_ref.first_gid) continue;
						const Tileset* tileset = _tilesets.get(tileset_ref.tileset);
						if (!tileset) continue;
						if (gid >= tileset_ref.first_gid + tileset->tile_count) continue;
						tile = &tileset->tiles[gid - tileset_ref.first_gid];
						break;
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
		} break;
		case LayerType::Image: {
			// TODO
		} break;
		case LayerType::Group: {
			for (pugi::xml_node child_node : std::ranges::reverse_view(node.children())) {
				_load_layer_recursive(map, child_node);
			}
		} break;
		}
	}

	Handle<Map> load_map(const std::string& path)
	{
		const std::string normalized_path = filesystem::get_normalized_path(path);
		auto handle_it = _path_to_map_handle.find(normalized_path);
		if (handle_it != _path_to_map_handle.end()) {
			return handle_it->second;
		}

		pugi::xml_document doc;
		if (!doc.load_file(normalized_path.c_str())) {
			console::log_error("Failed to load Tiled map: " + normalized_path);
			return Handle<Map>();
		}
		pugi::xml_node map_node = doc.child("map");

		Map map{};
		map.path = normalized_path;
		map.name = filesystem::get_stem(map.path);
		map.class_ = map_node.attribute("class").as_string();
		map.width = map_node.attribute("width").as_uint();
		map.height = map_node.attribute("height").as_uint();
		map.tile_width = map_node.attribute("tilewidth").as_uint();
		map.tile_height = map_node.attribute("tileheight").as_uint();
		_load_properties(map_node, map.properties);

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

			TilesetRef& tileset_ref = map.tilesets.emplace_back();
			tileset_ref.first_gid = tileset_node.attribute("firstgid").as_uint();
			tileset_ref.tileset = load_tileset(tileset_path);
		}
		// Sort tilesets by first_gid in ascending order. This is shouldn't be necessary
		// since Tiled already sorts them this way, but it doesn't hurt to be safe.
		std::sort(map.tilesets.begin(), map.tilesets.end(), [](const TilesetRef& a, const TilesetRef& b) {
			return a.first_gid < b.first_gid;
		});

		for (pugi::xml_node child_node : std::ranges::reverse_view(map_node.children())) {
			_load_layer_recursive(map, child_node);
		}

		const Handle<Map> handle = _maps.emplace(std::move(map));
		_path_to_map_handle[normalized_path] = handle;
		return handle;
	}

	Handle<Tileset> load_tileset(const std::string& path)
	{
		const std::string normalized_path = filesystem::get_normalized_path(path);
		auto handle_it = _path_to_tileset_handle.find(normalized_path);
		if (handle_it != _path_to_tileset_handle.end()) {
			return handle_it->second;
		}

		pugi::xml_document doc;
		if (!doc.load_file(normalized_path.c_str())) {
			console::log_error("Failed to load Tiled tileset: " + normalized_path);
			return Handle<Tileset>();
		}
		pugi::xml_node tileset_node = doc.child("tileset");

		const Handle<Tileset> handle = _tilesets.emplace();
		_path_to_tileset_handle[normalized_path] = handle;

		Tileset* tileset = _tilesets.get(handle);
		assert(tileset);
		tileset->path = normalized_path;
		tileset->name = tileset_node.attribute("name").as_string();
		tileset->class_ = tileset_node.attribute("class").as_string();
		tileset->tile_width = tileset_node.attribute("tilewidth").as_uint();
		tileset->tile_height = tileset_node.attribute("tileheight").as_uint();
		tileset->tile_count = tileset_node.attribute("tilecount").as_uint();
		tileset->columns = tileset_node.attribute("columns").as_uint();
		tileset->spacing = tileset_node.attribute("spacing").as_uint();
		tileset->margin = tileset_node.attribute("margin").as_uint();
		tileset->image_path = filesystem::get_parent_path(tileset->path);
		tileset->image_path += '/';
		tileset->image_path += tileset_node.child("image").attribute("source").as_string();
		tileset->image_path = filesystem::get_normalized_path(tileset->image_path);
		_load_properties(tileset_node, tileset->properties);
		tileset->tiles.resize(tileset->tile_count);
		for (unsigned int id = 0; id < tileset->tile_count; ++id) {
			Tile& tile = tileset->tiles[id];
			tile.tileset = handle;
			tile.id = id;
			tile.left = (id % tileset->columns) * (tileset->tile_width + tileset->spacing) + tileset->margin;
			tile.top = (id / tileset->columns) * (tileset->tile_height + tileset->spacing) + tileset->margin;
			tile.width = tileset->tile_width;
			tile.height = tileset->tile_height;
		}
		for (pugi::xml_node tile_node : tileset_node.children("tile")) {
			Tile& tile = tileset->tiles.at(tile_node.attribute("id").as_uint());
			tile.class_ = tile_node.attribute("type").as_string(); // SIC: "type", not "class"
			_load_properties(tile_node, tile.properties);
			for (pugi::xml_node object_node : tile_node.child("objectgroup").children("object")) {
				_load_object(object_node, tile.objects.emplace_back());
			}
			for (pugi::xml_node frame_node : tile_node.child("animation").children("frame")) {
				Frame& frame = tile.animation.emplace_back();
				frame.duration = frame_node.attribute("duration").as_uint();
				frame.tile_id = frame_node.attribute("tileid").as_uint();
			}
		}
		for (pugi::xml_node wangset_node : tileset_node.child("wangsets").children("wangset")) {
			WangSet& wangset = tileset->wangsets.emplace_back();
			wangset.name = wangset_node.attribute("name").as_string();
			wangset.class_ = wangset_node.attribute("class").as_string();
			_load_properties(wangset_node, wangset.properties);
			wangset.tile_id = (unsigned int)wangset_node.attribute("tile").as_int(); // -1 in case of no tile
			for (pugi::xml_node wangcolor_node : wangset_node.children("wangcolor")) {
				WangColor& wangcolor = wangset.colors.emplace_back();
				wangcolor.name = wangcolor_node.attribute("name").as_string();
				wangcolor.class_ = wangcolor_node.attribute("class").as_string();
				_load_properties(wangcolor_node, wangcolor.properties);
				wangcolor.tile_id = (unsigned int)wangcolor_node.attribute("tile").as_int(); // -1 in case of no tile
				wangcolor.probability = wangcolor_node.attribute("probability").as_float();
				std::string color_str = wangcolor_node.attribute("color").as_string();
				color_str.erase(color_str.begin()); // remove leading '#'
				color_str += "ff"; // add alpha channel
				const unsigned long color = std::stoul(color_str, nullptr, 16);
				wangcolor.color.r = (color >> 24) & 0xFF;
				wangcolor.color.g = (color >> 16) & 0xFF;
				wangcolor.color.b = (color >> 8) & 0xFF;
				wangcolor.color.a = color & 0xFF;
			}
			for (pugi::xml_node wangtile_node : wangset_node.children("wangtile")) {
				Tile& tile = tileset->tiles.at(wangtile_node.attribute("tileid").as_uint());
				WangTile& wangtile = tile.wangtiles.emplace_back();
				std::istringstream ss(wangtile_node.attribute("wangid").as_string());
				std::string token;
				size_t i = 0;
				while (std::getline(ss, token, ',')) {
					assert(i < WangTile::COUNT);
					if (unsigned int wang_id = std::stoul(token)) { // 0 means unset, 1 means first color, etc.
						wangtile.wangcolors[i] = &wangset.colors.at(wang_id - 1);
					}
					++i;
				}
			}
		}

		return handle;
	}

	Handle<Object> load_template(const std::string& path)
	{
		const std::string normalized_path = filesystem::get_normalized_path(path);
		auto handle_it = _path_to_template_handle.find(normalized_path);
		if (handle_it != _path_to_template_handle.end()) {
			return handle_it->second;
		}

		pugi::xml_document doc;
		if (!doc.load_file(normalized_path.c_str())) {
			console::log_error("Failed to load Tiled template: " + normalized_path);
			return Handle<Object>();
		}
		pugi::xml_node template_node = doc.child("template");
		pugi::xml_node object_node = template_node.child("object");

		Object template_{};
		template_.path = normalized_path;
		_load_object(object_node, template_);
		if (pugi::xml_node tileset_node = template_node.child("tileset")) {
			pugi::xml_attribute source_attribute = tileset_node.attribute("source");
			if (!source_attribute) {
				console::log_error("Embedded tilesets are not supported: " + template_.path);
				return Handle<Object>();
			}
			std::string tileset_path = filesystem::get_parent_path(template_.path);
			tileset_path += '/';
			tileset_path += source_attribute.as_string();
			tileset_path = filesystem::get_normalized_path(tileset_path);

			const Tileset* tileset = _tilesets.get(load_tileset(tileset_path));
			if (!tileset) {
				console::log_error("Failed to find tileset: " + tileset_path);
				return Handle<Object>();
			}

			unsigned int gid_with_flip_flags = object_node.attribute("gid").as_uint();
			unsigned int gid = _get_gid(gid_with_flip_flags);
			unsigned int id = gid - tileset_node.attribute("firstgid").as_uint();

			template_.tile = &tileset->tiles.at(id);
			template_.type = ObjectType::Tile;
			template_.flip_flags = _get_flip_flags(gid_with_flip_flags);
		}

		const Handle<Object> handle = _templates.emplace(std::move(template_));
		_path_to_template_handle[normalized_path] = handle;
		return handle;
	}

	const Map* get_map(Handle<Map> handle)
	{
		return _maps.get(handle);
	}

	const Tileset* get_tileset(Handle<Tileset> handle)
	{
		return _tilesets.get(handle);
	}

	const Object* get_template(Handle<Object> handle)
	{
		return _templates.get(handle);
	}

	std::span<const Map> get_maps()
	{
		return { _maps.data(), _maps.size() };
	}

	std::span<const Tileset> get_tilesets()
	{
		return { _tilesets.data(), _tilesets.size() };
	}

	std::span<const Object> get_templates()
	{
		return { _templates.data(), _templates.size() };
	}

	const Map* find_map_by_name(const std::string& name)
	{
		if (name.empty()) return nullptr;
		for (const Map& map : get_maps())
			if (map.name == name)
				return &map;
		return nullptr;
	}

	const Tileset* find_tileset_by_name(const std::string& name)
	{
		if (name.empty()) return nullptr;
		for (const Tileset& tileset : get_tilesets())
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

	const Tile* Map::get_tile(unsigned int gid) const
	{
		for (const TilesetRef& tileset_ref : tilesets) {
			if (gid < tileset_ref.first_gid) continue;
			const Tileset* tileset = _tilesets.get(tileset_ref.tileset);
			if (!tileset) continue;
			if (gid >= tileset_ref.first_gid + tileset->tile_count) continue;
			return &tileset->tiles[gid - tileset_ref.first_gid];
		}
		return nullptr;
	}
}