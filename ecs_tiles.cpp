#include "ecs_tiles.h"
#include "defines.h"
#include "physics_helpers.h"

namespace ecs
{
	extern entt::registry _registry;

	sf::IntRect _get_texture_rect(const tmx::Tileset& tileset, uint32_t tile_id)
	{
		if (!tileset.hasTile(tile_id))
			return sf::IntRect();

		uint32_t id_within_tileset = tile_id - tileset.getFirstGID();
		uint32_t x = id_within_tileset % tileset.getColumnCount();
		uint32_t y = id_within_tileset / tileset.getColumnCount();
		tmx::Vector2u tile_size = tileset.getTileSize();
		uint32_t left = tileset.getMargin() + (tile_size.x + tileset.getSpacing()) * x;
		uint32_t top = tileset.getMargin() + (tile_size.y + tileset.getSpacing()) * y;

		return sf::IntRect(left, top, tile_size.x, tile_size.y);
	}

	uint32_t _get_total_duration_in_ms(const tmx::Tileset::Tile::Animation& animation)
	{
		uint32_t total_duration = 0;
		for (const auto& frame : animation.frames)
			total_duration += frame.duration;
		return total_duration;
	}

	uint32_t _get_tile_id_at_time(const tmx::Tileset::Tile::Animation& animation, uint32_t time_in_ms)
	{
		uint32_t total_duration = _get_total_duration_in_ms(animation);
		if (!total_duration)
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

	Tile::Tile(const tmx::Tileset* tileset, const tmx::Tileset::Tile* tile)
		: _tileset(tileset)
		, _tile(tile)
	{
	}

	bool Tile::has_animation() const {
		return !_tile->animation.frames.empty();
	}

	float Tile::get_animation_duration() const {
		return _get_total_duration_in_ms(_tile->animation) / 1000.f;
	}

	const std::string& Tile::get_tileset_name() const {
		return _tileset->getName();
	}

	uint32_t Tile::get_id() const
	{
		// _tile->ID is local to the tileset, so we need to add the global tile ID
		// of the first tile in the tileset in order to get the global tile ID.
		return _tile->ID + _tileset->getFirstGID();
	}

	bool Tile::set_id(uint32_t tile_id)
	{
		if (auto tile = _tileset->getTile(tile_id))
		{
			_tile = tile;
			return true;
		}
		return false;
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
		{
			uint32_t time_ms = uint32_t(animation_time * 1000);
			id = _get_tile_id_at_time(_tile->animation, time_ms);
		}
		return _get_texture_rect(*_tileset, id);
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
					action = (length(velocity) > 7) ? "run" : "walk";
					direction = get_direction(velocity);
				}
				tile.set_type(action + "_" + direction);
			}
		}
	}

	void _update_tile_animations(float dt)
	{
		for (auto [entity, tile] : _registry.view<Tile>().each()) {
			if (tile.has_animation())
				tile.animation_time += tile.animation_speed * dt;
			tile.sprite.setTextureRect(tile.get_texture_rect());
		}
	}

	void _update_tile_sprite_positions()
	{
		for (auto [entity, tile, body] : _registry.view<Tile, b2Body*>().each()) {
			tile.sprite.setPosition(get_position(body) * PIXELS_PER_METER);
		}
	}

	void update_tiles(float dt)
	{
		_update_tile_types();
		_update_tile_animations(dt);
		_update_tile_sprite_positions();
	}
}