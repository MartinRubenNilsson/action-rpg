#pragma once
#include <tmxlite/Tileset.hpp>

namespace ecs
{
	// TODO: make it easier for client code to create a tile of a specific type,
	// without having to know the tileset name or tile ID.

	class Tile
	{
	public:
		Tile(const tmx::Tileset* tileset,
			const tmx::Tileset::Tile* tile);
		
		sf::Sprite sprite; // Uses pixel coordinates.
		bool visible = true; // If false, the tile is not drawn.
		float depth = 0.f; // Tiles with lower depth values are drawn first.
		float animation_time = 0.0f; // In seconds.
		float animation_speed = 1.0f; // 1 is normal speed, 2 is double speed, etc.

		bool has_animation() const;
		float get_animation_duration() const; // In seconds.

		const std::string& get_tileset_name() const;

		uint32_t get_id() const; // Global tile ID.
		bool set_id(uint32_t tile_id);
		const std::string& get_type() const; // "Type" is called "Class" in Tiled.
		bool set_type(const std::string& type);

		// TODO: const sf::Texture* get_texture() const
		sf::IntRect get_texture_rect() const;

	private:
		const tmx::Tileset* _tileset = nullptr;
		const tmx::Tileset::Tile* _tile = nullptr;
	};

	void update_graphics(float dt);
}
