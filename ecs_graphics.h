#pragma once
#include <tmxlite/Tileset.hpp>

namespace ecs
{
	class Tile
	{
	public:
		Tile(const tmx::Tileset* tileset, uint32_t tile_id);
		
		float animation_time = 0.0f; // In seconds.
		float animation_speed = 1.0f; // 1 is normal speed, 2 is double speed, etc.

		bool has_animation() const;
		float get_animation_duration() const; // In seconds.

		bool has_colliders() const; //TODO: remove

		const std::string& get_tileset_name() const;
		uint32_t get_id() const; // Global tile ID.
		const std::string& get_type() const; // "Type" is called "Class" in the Tiled editor.
		bool set_type(const std::string& type);
		sf::IntRect get_texture_rect() const;

	private:
		const tmx::Tileset* _tileset = nullptr;
		const tmx::Tileset::Tile* _tile = nullptr;
	};

	struct Sprite : sf::Sprite
	{
		bool visible = true; // If false, the sprite is not drawn.
		float depth = 0.f; // Sprites with lower depth values are drawn first.
	};

	void update_graphics(float dt);
}
