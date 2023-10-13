#include "ecs_graphics.h"
#include "defines.h"
#include "physics_helpers.h"

namespace tmx
{
	//TODO: dont hijack tmx namespace
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

	Tile::Tile(const tmx::Tileset* tileset, const tmx::Tileset::Tile* tile)
		: _tileset(tileset)
		, _tile(tile)
	{
	}

	bool Tile::has_animation() const {
		return !_tile->animation.frames.empty();
	}

	float Tile::get_animation_duration() const {
		return tmx::_get_total_duration_in_ms(_tile->animation) / 1000.f;
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

	bool Tile::set_id(uint32_t tile_id)
	{
		for (const auto& tile : _tileset->getTiles())
		{
			if (tile.ID == tile_id)
			{
				_tile = &tile;
				return true;
			}
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
			id = tmx::_get_tile_id_at_time(_tile->animation, time_ms);
		}
		return tmx::_get_texture_rect(*_tileset, id);
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

	void _update_tile_animation_times(float dt)
	{
		for (auto [entity, tile] : _registry.view<Tile>().each())
		{
			if (tile.has_animation())
				tile.animation_time += tile.animation_speed * dt;
		}
	}

	void _update_sprite_texture_rects()
	{
		for (auto [entity, sprite, tile]
			: _registry.view<Sprite, Tile>().each())
		{
			sprite.setTextureRect(tile.get_texture_rect());
		}
	}

	void _update_sprite_positions()
	{
		for (auto [entity, sprite, body] :
			_registry.view<Sprite, b2Body*>().each())
		{
			sf::Vector2f world_position = get_position(body);
			sf::Vector2f pixel_position = world_position * PIXELS_PER_METER;
			sprite.setPosition(pixel_position);
		}
	}

	void update_graphics(float dt)
	{
		_update_tile_types();
		_update_tile_animation_times(dt);
		_update_sprite_texture_rects();
		_update_sprite_positions();
	}
}
