#include "ecs_graphics.h"
#include "defines.h"
#include "physics_helpers.h"
#include "tiled.h"

namespace ecs
{
	extern entt::registry _registry;

	const std::unordered_map<std::string, SortingLayer> _layer_name_to_sorting_layer = {
		{ "Under Sprite 1", SortingLayer::Background1 },
		{ "Under Sprite 2", SortingLayer::Background2 },
		{ "Object Layer",   SortingLayer::Objects     },
		{ "Entities",       SortingLayer::Objects     },
		{ "Over Sprite 1",  SortingLayer::Foreground1 },
		{ "Over Sprite 2",  SortingLayer::Foreground2 },
		{ "Collision",      SortingLayer::Collision   },
	};

	SortingLayer layer_name_to_sorting_layer(const std::string& name)
	{
		auto it = _layer_name_to_sorting_layer.find(name);
		if (it != _layer_name_to_sorting_layer.end()) return it->second;
		return SortingLayer::Objects;
	}

	Tile::Tile(const tiled::Tile* tile)
		: _tile(tile)
		, _frame(tile)
	{
		assert(tile);
		animation_timer = Timer(get_animation_duration() / 1000.f);
		animation_timer.start();
	}

	std::string Tile::get_class() const {
		return _tile->class_;
	}

	bool Tile::set_class(const std::string& class_)
	{
		if (class_.empty()) return false;
		if (class_ == _tile->class_) return false;
		for (const tiled::Tile& tile : _tile->tileset->tiles) {
			if (tile.class_ == class_) {
				*this = Tile(&tile);
				return true;
			}
		}
		return false;
	}

	bool Tile::is_animated() const {
		return !_tile->animation.empty();
	}

	bool Tile::update_animation(float dt)
	{
		animation_timer.update(animation_speed * dt, animation_loop);
		uint32_t time_in_ms = (uint32_t)(animation_timer.get_time() * 1000.f);
		uint32_t duration = get_animation_duration();
		if (!duration) return false;
		uint32_t time = time_in_ms % duration;
		uint32_t current_time = 0;
		for (const tiled::Frame& frame : _tile->animation) {
			current_time += frame.duration;
			if (time < current_time) {
				bool changed = (frame.tile != _frame);
				_frame = frame.tile;
				return changed;
			}
		}
		return false;
	}

	sf::Sprite Tile::get_sprite() const
	{
		sf::Sprite sprite = _frame->sprite;
		sf::Vector2f sprite_scale(1.f, 1.f);
		sf::Vector2f sprite_origin = origin;
		sf::Vector2f sprite_size = sprite.getGlobalBounds().getSize();
		if (flip_x) {
			sprite_scale.x *= -1.f;
			sprite_origin.x = sprite_size.x - sprite_origin.x;
		}
		if (flip_y) {
			sprite_scale.y *= -1.f;
			sprite_origin.y = sprite_size.y - sprite_origin.y;
		}
		sprite.setPosition(position);
		sprite.setScale(sprite_scale);
		sprite.setOrigin(sprite_origin);
		sprite.setColor(color);
		return sprite;
	}

	uint32_t Tile::get_animation_duration() const
	{
		uint32_t duration_in_ms = 0;
		for (const tiled::Frame& frame : _tile->animation)
			duration_in_ms += frame.duration;
		return duration_in_ms;
	}

	void update_graphics(float dt)
	{
		// TODO: Move somewhere else
		for (auto [entity, anim, body] :
			_registry.view<SlimeAnimationController, Tile, b2Body*>().each())
		{
			sf::Vector2f velocity = get_linear_velocity(body);
			anim.set_class({ get_direction(velocity) });
			anim.animation_speed = length(velocity) / 32.f;
		}

		for (auto [entity, tile] : _registry.view<Tile>().each()) {
			if (tile.is_animated())
				tile.update_animation(dt);
		}

		for (auto [entity, tile, body] : _registry.view<Tile, b2Body*>().each()) {
			tile.position = get_position(body);
		}
	}

	Tile& emplace_tile(entt::entity entity, const tiled::Tile* tile)
	{
		assert(tile);
		return _registry.emplace_or_replace<Tile>(entity, tile);
	}

	Tile* emplace_tile(entt::entity entity, const std::string& tileset_name, const std::string& tile_class)
	{
		for (const tiled::Tileset& tileset : tiled::get_tilesets()) {
			if (tileset.name == tileset_name) {
				for (const tiled::Tile& tile : tileset.tiles) {
					if (tile.class_ == tile_class) {
						return &emplace_tile(entity, &tile);
					}
				}
			}
		}
		return nullptr;
	}

	void remove_tile(entt::entity entity) {
		_registry.remove<Tile>(entity);
	}

	void emplace_slime_animation_controller(entt::entity entity){
		_registry.emplace_or_replace<SlimeAnimationController>(entity);
	}
}
