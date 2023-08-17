#pragma once

namespace ecs
{
	// NOTE: In Tiled, "type" and "class" are the same thing.

	class Tile
	{
	public:
		Tile(const void* tileset, uint32_t id);
		
		float animation_time = 0.0f; // In seconds.

		bool is_animated() const;
		uint32_t get_id() const; // Global ID.
		const std::string& get_type() const;
		bool set_type(const std::string& type);
		sf::IntRect get_texture_rect() const;

	private:
		const void* _tileset = nullptr;
		const void* _tile = nullptr;
	};
}
