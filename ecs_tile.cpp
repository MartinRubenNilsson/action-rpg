#include "ecs_tile.h"
#include "physics_helpers.h"
#include "tiled.h"
#include "shaders.h"
#include "textures.h"

namespace ecs
{
	extern entt::registry _registry;
	float _shader_time_accumulator = 0.f; // sent to shaders as uniform float "time"

	const std::unordered_map<std::string, SortingLayer> _LAYER_NAME_TO_SORTING_LAYER = {
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
		auto it = _LAYER_NAME_TO_SORTING_LAYER.find(name);
		if (it != _LAYER_NAME_TO_SORTING_LAYER.end()) return it->second;
		return SortingLayer::Objects;
	}

	Tile::Tile(const tiled::Tile* tile)
	{
		assert(tile);
		set(tile);

		std::string shader_name;
		if (tile->properties.get_string("shader", shader_name)) {
			shader = shaders::get(shader_name);
		} else if (tile->tileset->properties.get_string("shader", shader_name)) {
			shader = shaders::get(shader_name);
		}
	}

	bool Tile::set(const tiled::Tile* tile)
	{
		if (!tile) return false;
		if (tile == _tile) return false;
		_tile = tile;
		_animation_duration_ms = 0;
		_animation_frame_index = 0;
		_animation_loop_count = 0;
		for (const tiled::Frame& frame : _tile->animation)
			_animation_duration_ms += frame.duration;
		animation_timer = Timer(_animation_duration_ms / 1000.f);
		animation_timer.start();
		return true;
	}

	bool Tile::set(const std::string& tile_class)
	{
		if (!_tile) return false; // no tileset to look in
		if (tile_class.empty()) return false;
		if (tile_class == _tile->class_) return false;
		return set(_tile->tileset->find_tile_by_class(tile_class));
	}

	bool Tile::set(const std::string& tile_class, const std::string& tileset_name)
	{
		if (tile_class.empty() || tileset_name.empty()) return false;
		if (_tile && tile_class == _tile->class_ && tileset_name == _tile->tileset->name) return false;
		const tiled::Tileset* tileset = tiled::find_tileset_by_name(tileset_name);
		if (!tileset) return false;
		return set(tileset->find_tile_by_class(tile_class));
	}

	sf::Sprite Tile::get_sprite() const
	{
		sf::Sprite sprite;
		if (!_tile) {
			if (std::shared_ptr<sf::Texture> error_texture = textures::get_error_texture())
				sprite.setTexture(*error_texture, true);
			return sprite;
		}
		if (_animation_duration_ms && _animation_frame_index < _tile->animation.size()) {
			const tiled::Frame& frame = _tile->animation[_animation_frame_index];
			sprite = frame.tile->sprite;
		} else {
			sprite = _tile->sprite;
		}
		sf::Vector2f origin = pivot;
		sf::Vector2f scale(1.f, 1.f);
		sf::Vector2f size = sprite.getLocalBounds().getSize();
		if (flip_x) {
			origin.x = size.x - origin.x;
			scale.x *= -1.f;
		}
		if (flip_y) {
			origin.y = size.y - origin.y;
			scale.y *= -1.f;
		}
		sprite.setOrigin(origin);
		sprite.setPosition(position);
		sprite.setScale(scale);
		sprite.setColor(color);
		if (texture)
			sprite.setTexture(*texture);
		return sprite;
	}

	std::string Tile::get_tile_class() const {
		return _tile ? _tile->class_ : "";
	}

	std::string Tile::get_tileset_name() const {
		return _tile ? _tile->tileset->name : "";
	}

	bool Tile::is_animated() const {
		return _animation_duration_ms != 0;
	}

	void Tile::update_animation(float dt)
	{
		if (!_tile) return;
		if (!_animation_duration_ms) return;
		if (animation_timer.update(animation_speed * dt, animation_loop) && animation_loop) {
			++_animation_loop_count;
			if (animation_flip_x_on_loop)
				flip_x = !flip_x;
		}
		uint32_t time_in_ms = (uint32_t)(animation_timer.get_time() * 1000.f);
		uint32_t time = time_in_ms % _animation_duration_ms;
		uint32_t current_time = 0;
		for (uint32_t frame_index = 0; frame_index < _tile->animation.size(); ++frame_index) {
			const tiled::Frame& frame = _tile->animation[frame_index];
			current_time += frame.duration;
			if (time < current_time) {
				_animation_frame_index = frame_index;
				break;
			}
		}
	}

	float Tile::get_animation_duration() const {
		return _animation_duration_ms / 1000.f;
	}

	void update_tiles(float dt)
	{
		_shader_time_accumulator += dt;

		// UPDATE TILE POSITION

		for (auto [entity, tile, body] : _registry.view<Tile, b2Body*>().each()) {
			tile.position = get_position(body);
		}

		// UPDATE TILE ANIMATION AND SHADER

		for (auto [entity, tile] : _registry.view<Tile>().each()) {
			tile.update_animation(dt);
			if (tile.shader) {
				tile.shader->setUniform("time", _shader_time_accumulator);
				tile.shader->setUniform("time_delta", dt);
				tile.shader->setUniform("position", tile.position);
			}
		}
	}
	
	Tile& emplace_tile(entt::entity entity) {
		return _registry.emplace_or_replace<Tile>(entity);
	}

	Tile& emplace_tile(entt::entity entity, const tiled::Tile* tile) {
		return _registry.emplace_or_replace<Tile>(entity, tile);
	}

	Tile& get_tile(entt::entity entity) {
		return _registry.get_or_emplace<Tile>(entity);
	}

	void remove_tile(entt::entity entity) {
		_registry.remove<Tile>(entity);
	}
}
