#include "ecs_tile.h"
#include "physics_helpers.h"
#include "tiled.h"
#include "shaders.h"

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

	Tile::Tile()
		: Tile(&tiled::get_error_tile())
	{
		_invalid = true;
	}

	Tile::Tile(const tiled::Tile* tile)
		: _tile(tile)
		, _frame(tile)
	{
		assert(tile);
		initialize_animation();

		std::string shader_name;
		if (tiled::get(tile->properties, "shader", shader_name)) {
			shader = shaders::get(shader_name);
		} else if (tiled::get(tile->tileset->properties, "shader", shader_name)) {
			shader = shaders::get(shader_name);
		}
	}

	bool Tile::set_class(const std::string& class_)
	{
		if (class_.empty()) return false;
		if (class_ == _tile->class_) return false;
		for (const tiled::Tile& tile : _tile->tileset->tiles) {
			if (tile.class_ == class_) {
				_tile = &tile;
				_frame = &tile;
				_invalid = false;
				initialize_animation();
				return true;
			}
		}
		_invalid = true;
		return false;
	}

	//bool Tile::set_class_and_tileset(const std::string& class_, const std::string& tileset_name)
	//{
	//	if (class_.empty() || tileset_name.empty()) return false;
	//	if (class_ == _tile->class_ && tileset_name == _tile->tileset->name) return false;
	//	// Find tileset
	//	const tiled::Tileset* tileset = nullptr;
	//	for (const tiled::Tileset& ts : tiled::get_tilesets()) {
	//		if (ts.name == tileset_name) {
	//			tileset = &ts;
	//			break;
	//		}
	//	}
	//	if (!tileset) return false;
	//	// Find tile
	//	for (const tiled::Tile& tile : tileset->tiles) {
	//		if (tile.class_ == class_) {
	//			_tile = &tile;
	//			_frame = &tile;
	//			initialize_animation();
	//			return true;
	//		}
	//	}
	//	return false;
	//}

	std::string Tile::get_class() const {
		return _tile->class_;
	}

	std::string Tile::get_tileset_name() const {
		return _tile->tileset->name;
	}

	bool Tile::is_animated() const {
		return _animation_duration_ms != 0;
	}

	float Tile::get_animation_duration() const {
		return _animation_duration_ms / 1000.f;
	}

	void Tile::update_animation(float dt)
	{
		if (!_animation_duration_ms) return;
		if (animation_timer.update(animation_speed * dt, animation_loop) && animation_loop) {
			++_animation_loop_count;
			if (animation_flip_x_on_loop)
				flip_x = !flip_x;
		}
		uint32_t time_in_ms = (uint32_t)(animation_timer.get_time() * 1000.f);
		uint32_t time = time_in_ms % _animation_duration_ms;
		uint32_t current_time = 0;
		for (const tiled::Frame& frame : _tile->animation) {
			current_time += frame.duration;
			if (time < current_time) {
				_frame = frame.tile;
				break;
			}
		}
	}

	sf::Sprite Tile::get_sprite() const
	{
		sf::Sprite sprite = _frame->sprite;
		sf::Vector2f origin = pivot;
		sf::Vector2f scale(1.f, 1.f);
		sf::Vector2f size = sprite.getGlobalBounds().getSize();
		if (_invalid) {
			sprite = tiled::get_error_tile().sprite;
			sf::Vector2f new_size = sprite.getGlobalBounds().getSize();
			origin *= new_size / size;
			scale *= size / new_size;
			size = new_size;
		}
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
		return sprite;
	}

	void Tile::initialize_animation()
	{
		_animation_duration_ms = 0;
		for (const tiled::Frame& frame : _tile->animation)
			_animation_duration_ms += frame.duration;
		animation_timer = Timer(_animation_duration_ms / 1000.f);
		animation_timer.start();
		_animation_loop_count = 0;
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

	Tile& emplace_tile(entt::entity entity, const tiled::Tile* tile)
	{
		assert(tile);
		return _registry.emplace_or_replace<Tile>(entity, tile);
	}

	Tile* emplace_tile(entt::entity entity, const std::string& tileset_name, const std::string& tile_class)
	{
		for (const tiled::Tileset& tileset : tiled::get_tilesets())
			if (tileset.name == tileset_name)
				for (const tiled::Tile& tile : tileset.tiles)
					if (tile.class_ == tile_class)
						return &emplace_tile(entity, &tile);
		return nullptr;
	}

	void remove_tile(entt::entity entity) {
		_registry.remove<Tile>(entity);
	}
}
