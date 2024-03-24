#include "ecs_tile.h"
#include "physics_helpers.h"
#include "tiled.h"
#include "shaders.h"
#include "textures.h"

namespace ecs
{
	extern entt::registry _registry;

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
		_set_tile(tile);
		std::string shader_name;
		if (tile->properties.get_string("shader", shader_name)) {
			shader = shaders::get(shader_name);
		} else if (tile->tileset->properties.get_string("shader", shader_name)) {
			shader = shaders::get(shader_name);
		}
	}

	bool Tile::_set_tile(const tiled::Tile* tile)
	{
		if (!tile) return false;
		if (tile == _tile) return false;
		_tile = tile;
		_animation_duration_ms = 0;
		_animation_frame_index = 0;
		set_flag(TF_FRAME_CHANGED, false);
		set_flag(TF_LOOPED, false);
		for (const tiled::Frame& frame : _tile->animation)
			_animation_duration_ms += frame.duration;
		animation_timer = Timer(_animation_duration_ms / 1000.f);
		animation_timer.start();
		return true;
	}

	const tiled::Tile* Tile::_get_tile(bool account_for_animation) const
	{
		if (!_tile) return nullptr;
		if (account_for_animation && _animation_frame_index < _tile->animation.size())
			return _tile->animation[_animation_frame_index].tile;
		return _tile;
	}

	bool Tile::set_tile(uint32_t id)
	{
		if (!_tile) return false; // no tileset to look in
		if (id == _tile->id) return false;
		if (id >= _tile->tileset->tiles.size()) return false;
		return _set_tile(&_tile->tileset->tiles[id]);
	}

	bool Tile::set_tile(uint32_t id, const std::string& tileset_name)
	{
		if (tileset_name.empty()) return false;
		const tiled::Tileset* tileset = nullptr;
		if (_tile && tileset_name == _tile->tileset->name) {
			if (id == _tile->id) return false;
			tileset = _tile->tileset;
		} else {
			tileset = tiled::find_tileset_by_name(tileset_name);
		}
		if (!tileset) return false;
		if (id >= tileset->tiles.size()) return false;
		return _set_tile(&tileset->tiles[id]);
	}

	bool Tile::set_tile(const std::string& class_)
	{
		if (class_.empty()) return false;
		if (!_tile) return false; // no tileset to look in
		if (class_ == _tile->class_) return false;
		return _set_tile(_tile->tileset->find_tile_by_class(class_));
	}

	bool Tile::set_tile(const std::string& class_, const std::string& tileset_name)
	{
		if (class_.empty() || tileset_name.empty()) return false;
		const tiled::Tileset* tileset = nullptr;
		if (_tile && tileset_name == _tile->tileset->name) {
			if (class_ == _tile->class_) return false;
			tileset = _tile->tileset;
		} else {
			tileset = tiled::find_tileset_by_name(tileset_name);
		}
		if (!tileset) return false;
		return _set_tile(tileset->find_tile_by_class(class_));
	}

	const std::string _DUMMY_EMPTY_STRING;
	const Properties _DUMMY_EMPTY_PROPERTIES;

	const std::string& Tile::get_class(bool account_for_animation) const
	{
		if (const tiled::Tile* tile = _get_tile(account_for_animation))
			return tile->class_;
		return _DUMMY_EMPTY_STRING;
	}

	const std::string& Tile::get_tileset_name() const {
		return _tile ? _tile->tileset->name : _DUMMY_EMPTY_STRING;
	}

	const Properties& Tile::get_properties(bool account_for_animation) const
	{
		if (const tiled::Tile* tile = _get_tile(account_for_animation))
			return tile->properties;
		return _DUMMY_EMPTY_PROPERTIES;
	}

	std::shared_ptr<sf::Texture> Tile::get_texture() const
	{
		std::shared_ptr<sf::Texture> ret;
		if (texture) {
			ret = texture;
		} else if (const tiled::Tile* tile = _get_tile(false)) {
			ret = tile->tileset->image;
		}
		return ret;
	}

	sf::IntRect Tile::get_texture_rect(bool account_for_animation) const
	{
		if (const tiled::Tile* tile = _get_tile(account_for_animation))
			return tile->image_rect;
		return sf::IntRect();
	}

	bool Tile::has_animation() const {
		return _animation_duration_ms != 0;
	}

	void Tile::update_animation(float dt)
	{
		if (!_tile || _tile->animation.empty()) return;
		if (!_animation_duration_ms) return;
		set_flag(TF_FRAME_CHANGED, false);
		set_flag(TF_LOOPED, false);
		const bool loop = get_flag(TF_LOOP);
		if (animation_timer.update(animation_speed * dt, loop) && loop) {
			set_flag(TF_LOOPED, true);
			if (get_flag(TF_FLIP_X_ON_LOOP))
				set_flag(TF_FLIP_X, !get_flag(TF_FLIP_X));
		}
		uint32_t time = (uint32_t)(animation_timer.get_time() * 1000.f); // in milliseconds
		for (uint32_t frame_index = 0; frame_index < _tile->animation.size(); ++frame_index) {
			uint32_t frame_duration = _tile->animation[frame_index].duration;
			if (time < frame_duration) {
				set_flag(TF_FRAME_CHANGED, frame_index != _animation_frame_index);
				_animation_frame_index = frame_index;
				return;
			} else {
				time -= frame_duration;
			}
		}
		// Park on the last frame. We will for example get here if
		// animation_timer.get_time() == animation_timer.get_duration().
		_animation_frame_index = (uint32_t)_tile->animation.size() - 1;
	}

	float Tile::get_animation_duration() const {
		return _animation_duration_ms / 1000.f;
	}

	void Tile::set_flag(TileFlags flag, bool value)
	{
		if (value) {
			_flags |= flag;
		} else {
			_flags &= ~flag;
		}
	}

	bool Tile::get_flag(TileFlags flag) const {
		return (_flags & flag) != 0;
	}

	void update_tiles(float dt)
	{
		for (auto [entity, tile] : _registry.view<Tile>().each()) {
			tile.update_animation(dt);
		}
		for (auto [entity, tile, body] : _registry.view<Tile, b2Body*>().each()) {
			tile.position = get_position(body);
		}
	}
	
	bool has_tile(entt::entity entity) {
		return _registry.all_of<Tile>(entity);
	}

	Tile& emplace_tile(entt::entity entity) {
		return _registry.emplace_or_replace<Tile>(entity);
	}

	Tile& emplace_tile(entt::entity entity, const tiled::Tile* tile) {
		return _registry.emplace_or_replace<Tile>(entity, tile);
	}

	Tile& get_tile(entt::entity entity) {
		return _registry.get<Tile>(entity);
	}

	Tile* try_get_tile(entt::entity entity) {
		return _registry.try_get<Tile>(entity);
	}

	bool remove_tile(entt::entity entity) {
		return _registry.remove<Tile>(entity);
	}
}
