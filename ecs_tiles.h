#pragma once
#include <tmxlite/Tileset.hpp>

namespace ecs
{
	class Tile
	{
	public:
		Tile(const tmx::Tileset* tileset,
			const tmx::Tileset::Tile* tile);
		
		sf::Sprite sprite; // Uses pixel coordinates.
		bool visible = true;
		float depth = 0.f; // Tiles with lower depth values are drawn first.
		float animation_time = 0.0f; // In seconds.
		float animation_speed = 1.0f; // 1 is normal speed, 2 is double speed, etc.

		bool has_animation() const;
		float get_animation_duration() const; // In seconds.

		const std::string& get_tileset_name() const;

		uint32_t get_id() const; // Global tile ID.
		bool set_id(uint32_t tile_id);
		const std::string& get_type() const;
		bool set_type(const std::string& type);

		sf::IntRect get_texture_rect() const;

	private:
		const tmx::Tileset* _tileset = nullptr;
		const tmx::Tileset::Tile* _tile = nullptr;
	};

	void update_tiles(float dt);
}
