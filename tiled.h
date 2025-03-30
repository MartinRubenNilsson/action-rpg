#pragma once
#include "tiled_types.h"

namespace tiled {

	struct Map;
	struct Tileset;
	struct Object;

	std::span<const Map> get_maps();
	std::span<const Tileset> get_tilesets();
	std::span<const Object> get_templates();

	const Map* get_map(Handle<Map> handle);
	const Tileset* get_tileset(Handle<Tileset> handle);
	const Object* get_template(Handle<Object> handle);

	//TODO: return Handle instead of pointer
	const Map* find_map_by_name(const std::string& name);
	Handle<Tileset> find_tileset_by_name(const std::string& name);

	Handle<Map> load_map_from_file(const std::string& path);
	Handle<Tileset> load_tileset_from_file(const std::string& path);
	Handle<Object> load_template_from_file(const std::string& path);
}
