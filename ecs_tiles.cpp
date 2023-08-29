#include "ecs_tiles.h"

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
	extern entt::registry _registry;

	Tile::Tile(const const tmx::Tileset* tileset, uint32_t tile_id)
		: _tileset(tileset)
		, _tile(tileset->getTile(tile_id))
	{
		assert(_tile && "Tile ID not found in tileset.");
	}

	bool Tile::has_animation() const {
		return !_tile->animation.frames.empty();
	}

	bool Tile::has_colliders() const {
		return !_tile->objectGroup.getObjects().empty();
	}

	const std::string& Tile::get_tileset_name() const {
		return _tileset->getName();
	}

	uint32_t Tile::get_id() const
	{
		// The tile ID is local to the tileset, so we need to add the global tile ID
		// of the first tile in the tileset in order to get the global tile ID.
		return _tile->ID + _tileset->getFirstGID();
	}

	const std::string& Tile::get_type() const {
		return _tile->type;
	}

	bool Tile::set_type(const std::string& type)
	{
		for (const auto& tile : _tileset->getTiles())
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
		if (has_animation())
			id = tmx::_get_tile_id_at_time(_tile->animation, animation_time * 1000);
		return tmx::_get_texture_rect(*_tileset, id);
	}

	void _update_tile_animation_times(float dt)
	{
		for (auto [entity, tile] : _registry.view<Tile>().each())
		{
			if (tile.has_animation())
				tile.animation_time += dt;
		}
	}

	void _update_tile_types()
	{
		for (auto [entity, tile, body] :
			_registry.view<Tile, b2Body*>().each())
		{
			std::string current_type = tile.get_type();
			sf::Vector2f velocity = get_linear_velocity(body);
			std::string tileset_name = tile.get_tileset_name();
			if (tileset_name == "naked_human")
			{
				std::string action = "idle";
				char direction = current_type.empty() ? ' ' : current_type.back();
				if (is_zero(velocity))
				{
					tile.animation_time = 0; // should probably be done elsewhere
				}
				else
				{
					action = (length(velocity) > 9) ? "run" : "walk";
					direction = get_direction(velocity);
				}
				tile.set_type(action + "_" + direction);
			}
		}
	}

	void _update_tile_sprite_texture_rects()
	{
		for (auto [entity, sprite, tile] :
			_registry.view<sf::Sprite, Tile>().each())
		{
			sprite.setTextureRect(tile.get_texture_rect());
		}
	}

	void update_tiles(float dt)
	{
		_update_tile_animation_times(dt);
		_update_tile_types();
		_update_tile_sprite_texture_rects();
	}
}
