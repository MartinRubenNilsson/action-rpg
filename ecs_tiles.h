#pragma once
#include <tmxlite/Tileset.hpp>
#include <tmxlite/Property.hpp>

namespace ecs
{
	class Tile
	{
	public:
		Tile(const tmx::Tileset* tileset, uint32_t tile_id);
		
		float animation_time = 0.0f; // In seconds.

		bool has_animation() const;
		bool has_colliders() const;
		uint32_t get_id() const; // Global tile ID.
		const std::string& get_type() const; // "Type" is called "Class" in the Tiled editor.
		bool set_type(const std::string& type);
		sf::IntRect get_texture_rect() const;
		const tmx::Property* get_property(const std::string& name) const;

	private:
		const tmx::Tileset* _tileset = nullptr;
		const tmx::Tileset::Tile* _tile = nullptr;
	};
}
