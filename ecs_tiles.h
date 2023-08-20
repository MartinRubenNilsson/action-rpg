#pragma once
#include <tmxlite/Tileset.hpp>

namespace ecs
{
	// NOTE: In Tiled, "type" and "class" are the same thing.

	class Tile
	{
	public:
		Tile(const tmx::Tileset* tileset, uint32_t id); // Global ID.
		
		float animation_time = 0.0f; // In seconds.

		bool has_animation() const;
		bool has_colliders() const;
		uint32_t get_id() const; // Global ID.
		const std::string& get_type() const;
		bool set_type(const std::string& type);
		sf::IntRect get_texture_rect() const;

	private:
		const tmx::Tileset* _tileset = nullptr;
		const tmx::Tileset::Tile* _tile = nullptr;
	};
}
