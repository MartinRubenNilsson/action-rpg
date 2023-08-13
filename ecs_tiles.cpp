#include "ecs_tiles.h"
#include <tmxlite/Tileset.hpp>

namespace tmx
{
	sf::IntRect _get_texture_rect(const Tileset& tileset, uint32_t tile_id)
	{
		if (!tileset.hasTile(tile_id))
			return sf::IntRect();

		uint32_t id_within_tileset = tile_id - tileset.getFirstGID();
		uint32_t x = id_within_tileset % tileset.getColumnCount();
		uint32_t y = id_within_tileset / tileset.getColumnCount();
		Vector2u tile_size = tileset.getTileSize();
		uint32_t left = tileset.getMargin() + (tile_size.x + tileset.getSpacing()) * x;
		uint32_t top = tileset.getMargin() + (tile_size.y + tileset.getSpacing()) * y;

		return sf::IntRect(left, top, tile_size.x, tile_size.y);
	}

	uint32_t _get_total_duration_in_ms(const Tileset::Tile::Animation& animation)
	{
		uint32_t total_duration = 0;
		for (const auto& frame : animation.frames)
			total_duration += frame.duration;
		return total_duration;
	}

	uint32_t _get_tile_id_at_time(const Tileset::Tile::Animation& animation, uint32_t time_in_ms)
	{
		uint32_t total_duration = _get_total_duration_in_ms(animation);
		if (total_duration == 0)
			return UINT32_MAX;

		uint32_t time = time_in_ms % total_duration;
		uint32_t current_time = 0;
		for (const auto& frame : animation.frames)
		{
			current_time += frame.duration;
			if (time < current_time)
				return frame.tileID;
		}

		return UINT32_MAX; // Should never happen.
	}
}

namespace ecs
{
	bool is_valid(const Tile& tile)
	{
		return tile._tileset && tile._tileset->hasTile(tile.id);
	}

	bool is_animated(const Tile& tile)
	{
		if (!tile._tileset) return false;
		auto tileset_tile = tile._tileset->getTile(tile.id);
		if (!tileset_tile) return false;
		return !tileset_tile->animation.frames.empty();
	}

	sf::IntRect get_texture_rect(const Tile& tile)
	{
		uint32_t tile_id = tile.id;
		if (is_animated(tile))
		{
			auto& animation = tile._tileset->getTile(tile.id)->animation;
			uint32_t time_in_ms = tile.animation_time * 1000;
			tile_id = tmx::_get_tile_id_at_time(animation, time_in_ms);
		}
		return tmx::_get_texture_rect(*tile._tileset, tile_id);
	}
}
