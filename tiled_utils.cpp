#include "stdafx.h"
#include "tiled.h"
#include "tiled_types.h"

namespace tiled {
	Property* find_property_by_name(std::vector<Property>& properties, std::string_view name) {
		for (Property& prop : properties) {
			if (prop.name == name) {
				return &prop;
			}
		}
		return nullptr;
	}

	const Property* find_property_by_name(const std::vector<Property>& properties, std::string_view name) {
		for (const Property& prop : properties) {
			if (prop.name == name) {
				return &prop;
			}
		}
		return nullptr;
	}

	TextureRect get_tile_texture_rect(const Tileset& tileset, unsigned int tile_id) {
		TextureRect rect{};
		rect.x = (tile_id % tileset.columns) * (tileset.tile_width + tileset.spacing) + tileset.margin;
		rect.y = (tile_id / tileset.columns) * (tileset.tile_height + tileset.spacing) + tileset.margin;
		rect.w = tileset.tile_width;
		rect.h = tileset.tile_height;
		return rect;
	}

	TilesetLink find_tileset_link_for_tile_gid(const std::vector<TilesetLink>& tileset_links, unsigned int tile_gid) {
		for (auto it = tileset_links.rbegin(); it != tileset_links.rend(); ++it) {
			if (tile_gid >= it->first_gid) {
				return *it;
			}
		}
		return TilesetLink{};
	}

	const Tile* find_tile_with_gid(
		const std::vector<TilesetLink>& tileset_links,
		const std::vector<Tileset>& tilesets,
		unsigned int tile_gid
	) {
		if (tile_gid == 0) return nullptr;
		TilesetLink link = find_tileset_link_for_tile_gid(tileset_links, tile_gid);
		if (link.first_gid == 0) return nullptr;
		if (link.tileset_id >= tilesets.size()) return nullptr;
		const Tileset& tileset = tilesets[link.tileset_id];
		if (tile_gid < link.first_gid) return nullptr;
		if (tile_gid >= link.first_gid + tileset.tile_count) return nullptr;
		return &tileset.tiles[tile_gid - link.first_gid];
	}

	const Object* find_object_with_name(const Map& map, std::string_view name) {
		if (name.empty()) return nullptr;
		for (const Layer& layer : map.layers) {
			for (const Object& object : layer.objects) {
				if (object.name == name) {
					return &object;
				}
			}
		}
		return nullptr;
	}
}