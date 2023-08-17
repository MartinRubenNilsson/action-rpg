#include "ecs_tiles.h"
#include <tmxlite/Map.hpp>
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
	Tile::Tile(const void* tileset, uint32_t id)
		: _tileset(tileset)
		, _tile(static_cast<const tmx::Tileset*>(tileset)->getTile(id))
	{
	}

	bool Tile::is_animated() const
	{
		auto tile = static_cast<const tmx::Tileset::Tile*>(_tile);
		return !tile->animation.frames.empty();
	}

	uint32_t Tile::get_id() const
	{
		auto tile = static_cast<const tmx::Tileset::Tile*>(_tile);
		auto tileset = static_cast<const tmx::Tileset*>(_tileset);
		return tile->ID + tileset->getFirstGID();
	}

	const std::string& Tile::get_type() const
	{
		auto tile = static_cast<const tmx::Tileset::Tile*>(_tile);
		return tile->type;
	}

	bool Tile::set_type(const std::string& type)
	{
		auto tileset = static_cast<const tmx::Tileset*>(_tileset);
		for (const auto& tile : tileset->getTiles())
		{
			if (tile.type == type)
			{
				_tile = &tile;
				return true;
			}
		}
		return false;
	}

	sf::IntRect Tile::get_texture_rect() const
	{
		uint32_t id = get_id();
		if (is_animated())
		{
			auto tile = static_cast<const tmx::Tileset::Tile*>(_tile);
			id = tmx::_get_tile_id_at_time(tile->animation, animation_time * 1000);
		}
		auto tileset = static_cast<const tmx::Tileset*>(_tileset);
		return tmx::_get_texture_rect(*tileset, id);
	}
}
