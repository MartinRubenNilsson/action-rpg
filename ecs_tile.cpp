#include "stdafx.h"
#include "ecs_tile.h"
#include "physics_helpers.h"
#include "tiled.h"
#include "graphics.h"
#include "sprites.h"

namespace ecs
{
	extern entt::registry _registry;
	float _tile_time_accumulator = 0.f;

	Tile::Tile()
		: sorting_layer(sprites::SL_OBJECTS)
	{}

	Tile::Tile(const tiled::Tile* tile)
		: Tile()
	{
		assert(tile);
		_set_tile(tile);
		std::string shader_name;
		if (tile->properties.get_string("shader", shader_name)) {
			shader = graphics::load_shader({}, "assets/shaders/" + shader_name + ".frag");
		} else if (tile->tileset->properties.get_string("shader", shader_name)) {
			shader = graphics::load_shader({}, "assets/shaders/" + shader_name + ".frag");
		} else {
			shader = graphics::default_shader;
		}
	}

	bool Tile::_set_tile(const tiled::Tile* tile)
	{
		if (!tile) return false;
		if (tile == _tile) return false;
		_tile = tile;
		_animation_duration_ms = 0;
		_animation_frame = 0;
		set_flag(TF_FRAME_CHANGED, false);
		set_flag(TF_LOOPED, false);
		for (const tiled::Frame& frame : _tile->animation) {
			_animation_duration_ms += frame.duration;
		}
		animation_timer = Timer(_animation_duration_ms / 1000.f);
		animation_timer.start();
		if (texture_id == -1) {
			texture_id = graphics::load_texture(tile->tileset->image_path);
		}
		return true;
	}

	const tiled::Tile* Tile::_get_tile(bool account_for_animation) const
	{
		if (!_tile) return nullptr;
		if (account_for_animation && _animation_frame < _tile->animation.size())
			return _tile->animation[_animation_frame].tile;
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
		return _set_tile(tiled::find_tile_by_class(*_tile->tileset, class_));
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
		return _set_tile(tiled::find_tile_by_class(*tileset, class_));
	}

	// For optimization purposes; returning a reference to a dummy object
	// is cheaper than having to construct and return an empty object.
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

	void Tile::get_texture_rect(uint32_t& left, uint32_t& top, uint32_t& width, uint32_t& height, bool account_for_animation) const
	{
		if (const tiled::Tile* tile = _get_tile(account_for_animation)) {
			left = tile->left;
			top = tile->top;
			width = tile->width;
			height = tile->height;
		} else {
			left = 0;
			top = 0;
			width = 0;
			height = 0;
		}
	}

	bool Tile::has_animation() const {
		return _animation_duration_ms != 0;
	}

	void Tile::update_animation(float dt)
	{
		if (!_tile) return;
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
				set_flag(TF_FRAME_CHANGED, frame_index != _animation_frame);
				_animation_frame = frame_index;
				return;
			} else {
				time -= frame_duration;
			}
		}
		// Park on the last frame. We will for example get here if
		// animation_timer.get_time() == animation_timer.get_duration().
		_animation_frame = (uint32_t)_tile->animation.size() - 1;
	}

	float Tile::get_animation_duration() const {
		return _animation_duration_ms / 1000.f;
	}

	uint32_t Tile::get_animation_frame() const {
		return _animation_frame;
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

	bool _get_nth_bit(int value, int n) {
		return (value & (1 << n)) >> n;
	}

	void Tile::set_rotation(int clockwise_quarter_turns)
	{
		bool bit_0 = _get_nth_bit(clockwise_quarter_turns, 0);
		bool bit_1 = _get_nth_bit(clockwise_quarter_turns, 1);
		set_flag(TF_FLIP_X, bit_0 != bit_1); // XOR
		set_flag(TF_FLIP_Y, bit_1);
		set_flag(TF_FLIP_DIAGONAL, bit_0);
	}

	void update_tiles(float dt)
	{
		_tile_time_accumulator += dt;
		for (auto [entity, tile] : _registry.view<Tile>().each()) {
			tile.update_animation(dt);
		}
		for (auto [entity, tile, body] : _registry.view<Tile, b2Body*>().each()) {
			tile.position = get_position(body);
		}
	}

	void add_tile_sprites_to_render_queue(const Vector2f& camera_min, const Vector2f& camera_max)
	{
		sprites::Sprite sprite{};
		for (auto [entity, tile] : _registry.view<const Tile>().each()) {
			if (!tile.is_valid()) continue;
			if (!tile.get_flag(TF_VISIBLE)) continue;
			sprite.texture_id = tile.texture_id;
			if (sprite.texture_id == -1) continue;
			Vector2u texture_size;
			graphics::get_texture_size(sprite.texture_id, texture_size.x, texture_size.y);
			sprite.min = tile.position - tile.pivot;
			if (sprite.min.x > camera_max.x || sprite.min.y > camera_max.y) continue;
			uint32_t left, top, width, height;
			tile.get_texture_rect(left, top, width, height, true);
			sprite.tex_min = { (float)left, (float)top };
			sprite.tex_max = { (float)left + width, (float)top + height };
			sprite.max = sprite.min + sprite.tex_max - sprite.tex_min;
			sprite.tex_min /= texture_size;
			sprite.tex_max /= texture_size;
			//std::swap(sprite.tex_min.y, sprite.tex_max.y); // flip Y
			if (sprite.max.x < camera_min.x || sprite.max.y < camera_min.y) continue;
			sprite.shader = tile.shader;
			sprite.color = tile.color;
			sprite.sorting_layer = (uint8_t)tile.sorting_layer;
			sprite.sorting_pos = sprite.min + tile.sorting_pivot;
			sprite.flags = 0;
			if (tile.get_flag(TF_FLIP_X)) {
				sprite.flags |= sprites::SF_FLIP_X;
			}
			if (tile.get_flag(TF_FLIP_Y)) {
				sprite.flags |= sprites::SF_FLIP_Y;
			}
			if (tile.get_flag(TF_FLIP_DIAGONAL)) {
				sprite.flags |= sprites::SF_FLIP_DIAGONAL;
			}
			sprite.pre_render_callback = nullptr;
			if (tile.get_class() == "grass") {
				sprite.pre_render_callback = [](const sprites::Sprite& sprite) {
					if (sprite.shader == graphics::ShaderHandle::Invalid) return;
					graphics::bind_shader(sprite.shader);
					graphics::set_shader_uniform_1f(sprite.shader, "time", _tile_time_accumulator);
					graphics::set_shader_uniform_2f(sprite.shader, "position", sprite.min.x, sprite.min.y);
				};
			}
			sprites::add_sprite_to_render_queue(sprite);
		}
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

	bool has_tile(entt::entity entity) {
		return _registry.all_of<Tile>(entity);
	}
}
