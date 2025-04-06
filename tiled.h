#pragma once
#include "tiled_types.h"
#include <string_view>

namespace tiled {

	Property* find_property_by_name(std::vector<Property>& properties, std::string_view name);
	const Property* find_property_by_name(const std::vector<Property>& properties, std::string_view name);

	template <PropertyType type>
	bool get(const std::vector<Property>& properties, std::string_view name, std::variant_alternative_t<(size_t)type, PropertyValue>& value) {
		const Property* prop = find_property_by_name(properties, name);
		if (!prop) return false;
		if (prop->value.index() != (size_t)type) return false;
		value = std::get<(size_t)type>(prop->value);
		return true;
	}

	TextureRect get_tile_texture_rect(const Tileset& tileset, unsigned int tile_id);

	TilesetLink find_tileset_link_for_tile_gid(const std::vector<TilesetLink>& tilesets, unsigned int tile_gid);
	const Tile* find_tile_with_gid(
		const std::vector<TilesetLink>& tileset_links,
		const std::vector<Tileset>& tilesets,
		unsigned int tile_gid);
	const Object* find_object_with_name(const Map& map, std::string_view name);

	// Returns the tileset ID (an index into Context::tilesets[]), or UINT_MAX if not found.
	unsigned int load_tileset_from_file(Context &context, const std::string& path);

	// Returns the template ID (an index into Context::templates[]), or UINT_MAX if not found.
	unsigned int load_template_from_file(Context &context, const std::string& path);

	// Returns the map ID (an index into Context::maps[]), or UINT_MAX if not found.
	unsigned int load_map_from_file(Context &context, const std::string& path);
}
