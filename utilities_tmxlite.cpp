#include "utilities_tmxlite.h"

namespace tmx
{
	sf::IntRect get_texture_rect(const Tileset& tileset, uint32_t tile_id)
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

	float get_total_duration(const AnimatedTile& animated_tile)
	{
		auto tile = animated_tile.tileset->getTile(animated_tile.tile_id);
		return _get_total_duration_in_ms(tile->animation) / 1000.0f;
	}

	uint32_t get_current_tile_id(const AnimatedTile& animated_tile)
	{
		auto tile = animated_tile.tileset->getTile(animated_tile.tile_id);
		return _get_tile_id_at_time(tile->animation, animated_tile.time * 1000);
	}

	sf::IntRect get_current_texture_rect(const AnimatedTile& animated_tile)
	{
		auto tile = animated_tile.tileset->getTile(animated_tile.tile_id);
		uint32_t current_tile_id = _get_tile_id_at_time(tile->animation, animated_tile.time * 1000);
		return get_texture_rect(*animated_tile.tileset, current_tile_id);
	}
}
