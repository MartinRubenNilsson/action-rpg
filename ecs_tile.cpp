#include "stdafx.h"
#include "ecs_tile.h"
#include "tiled.h"
#include "graphics.h"
#include "sprites.h"

namespace tiled
{
	const Tile* _get_tile(Handle<Tileset> tileset_handle, unsigned int tile_id)
	{
		if (tileset_handle == Handle<Tileset>()) return nullptr;
		const Tileset* tileset = get_tileset(tileset_handle);
		if (!tileset) return nullptr;
		if (tile_id >= tileset->tiles.size()) return nullptr;
		return &tileset->tiles[tile_id];
	}
}

namespace ecs
{
	extern entt::registry _registry;
	float _tile_time_accumulator = 0.f;

	bool Tile::set_tile(const tiled::Tile* tile)
	{
		if (!tile) return false;
		if (tile->tileset == _tileset_handle && tile->id == _tile_id) return false;
		_tileset_handle = tile->tileset;
		_tile_id = tile->id;
		_animation_frame = 0;
		set_flag(TILE_FRAME_CHANGED, false);
		set_flag(TILE_LOOPED, false);
		float animation_duration = 0.f;
		for (const tiled::Frame& frame : tile->animation) {
			animation_duration += frame.duration_ms; // in milliseconds
		}
		animation_duration /= 1000.f; // in seconds
		animation_timer = Timer(animation_duration);
		animation_timer.start();
		if (texture == Handle<graphics::Texture>()) {
			texture = graphics::load_texture(tiled::get_tileset(tile->tileset)->image_path);
		}
		if (shader == Handle<graphics::Shader>()) {
			if (std::string shader_name; tile->properties.get_string("shader", shader_name)) {
				shader = graphics::load_shader(
					"assets/shaders/sprite.vert",
					"assets/shaders/" + shader_name + ".frag");
			} else if (tiled::get_tileset(tile->tileset)->properties.get_string("shader", shader_name)) {
				shader = graphics::load_shader(
					"assets/shaders/sprite.vert",
					"assets/shaders/" + shader_name + ".frag");
			} else {
				shader = graphics::sprite_shader;
			}
		}
		return true;
	}

	const tiled::Tile* Tile::_get_tile(bool account_for_animation) const
	{
		const tiled::Tile* tile = tiled::_get_tile(_tileset_handle, _tile_id);
		if (!tile) return nullptr;
		if (account_for_animation && _animation_frame < tile->animation.size()) {
			unsigned int tile_id = tile->animation[_animation_frame].tile_id;
			return &tiled::get_tileset(tile->tileset)->tiles[tile_id];
		}
		return tile;
	}

	bool Tile::set_tile(unsigned int id)
	{
		const tiled::Tile* tile = tiled::_get_tile(_tileset_handle, _tile_id);
		if (!tile) return false; // no tileset to look in
		if (id == tile->id) return false;
		if (id >= tiled::get_tileset(tile->tileset)->tiles.size()) return false;
		return set_tile(&tiled::get_tileset(tile->tileset)->tiles[id]);
	}

	bool Tile::set_tile(unsigned int id, const std::string& tileset_name)
	{
		if (tileset_name.empty()) return false;
		const tiled::Tile* tile = tiled::_get_tile(_tileset_handle, _tile_id);
		const tiled::Tileset* tileset = nullptr;
		if (tile && tileset_name == tiled::get_tileset(tile->tileset)->name) {
			if (id == tile->id) return false;
			tileset = tiled::get_tileset(tile->tileset);
		} else {
			tileset = tiled::find_tileset_by_name(tileset_name);
		}
		if (!tileset) return false;
		if (id >= tileset->tiles.size()) return false;
		return set_tile(&tileset->tiles[id]);
	}

	bool Tile::set_tile(unsigned int x, unsigned int y)
	{
		const tiled::Tile* tile = tiled::_get_tile(_tileset_handle, _tile_id);
		if (!tile) return false; // no tileset to look in
		if (x == tile->x && y == tile->y) return false;
		const tiled::Tileset* tileset = tiled::get_tileset(tile->tileset);
		if (x >= tileset->columns || y >= tileset->tile_count / tileset->columns) return false;
		return set_tile(&tileset->tiles[y * tileset->columns + x]);
	}

	bool Tile::set_tile(const std::string& class_)
	{
		if (class_.empty()) return false;
		const tiled::Tile* tile = tiled::_get_tile(_tileset_handle, _tile_id);
		if (!tile) return false; // no tileset to look in
		if (class_ == tile->class_) return false;
		return set_tile(tiled::find_tile_by_class(*tiled::get_tileset(tile->tileset), class_));
	}

	bool Tile::set_tile(const std::string& class_, const std::string& tileset_name)
	{
		if (class_.empty() || tileset_name.empty()) return false;
		const tiled::Tile* tile = tiled::_get_tile(_tileset_handle, _tile_id);
		const tiled::Tileset* tileset = nullptr;
		if (tile && tileset_name == tiled::get_tileset(tile->tileset)->name) {
			if (class_ == tile->class_) return false;
			tileset = tiled::get_tileset(tile->tileset);
		} else {
			tileset = tiled::find_tileset_by_name(tileset_name);
		}
		if (!tileset) return false;
		return set_tile(tiled::find_tile_by_class(*tileset, class_));
	}

	// For optimization purposes; returning a reference to a dummy object
	// is cheaper than having to construct and return an empty object.
	const std::string _DUMMY_EMPTY_STRING;
	const Properties _DUMMY_EMPTY_PROPERTIES;

	const std::string& Tile::get_class(bool account_for_animation) const
	{
		if (const tiled::Tile* tile = _get_tile(account_for_animation)) {
			return tile->class_;
		}
		return _DUMMY_EMPTY_STRING;
	}

	const Properties& Tile::get_properties(bool account_for_animation) const
	{
		if (const tiled::Tile* tile = _get_tile(account_for_animation)) {
			return tile->properties;
		}
		return _DUMMY_EMPTY_PROPERTIES;
	}

	void Tile::get_texture_rect(unsigned int& left, unsigned int& top, unsigned int& width, unsigned int& height) const
	{
		if (const tiled::Tile* tile = _get_tile(true)) {
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

	unsigned int Tile::get_id(bool account_for_animation) const
	{
		if (const tiled::Tile* tile = _get_tile(account_for_animation)) {
			return tile->id;
		}
		return 0;
	}

	Vector2u Tile::get_coords(bool account_for_animation) const
	{
		if (const tiled::Tile* tile = _get_tile(account_for_animation)) {
			return Vector2u(tile->x, tile->y);
		}
		return Vector2u(0, 0);
	}

	bool Tile::has_animation() const
	{
		return animation_timer.get_duration();
	}

	void Tile::update_animation(float dt)
	{
		if (!animation_timer.get_duration()) return;
		const tiled::Tile* tile = tiled::_get_tile(_tileset_handle, _tile_id);
		if (!tile) return;
		set_flag(TILE_FRAME_CHANGED, false);
		set_flag(TILE_LOOPED, false);
		const bool loop = get_flag(TILE_LOOP);
		if (animation_timer.update(animation_speed * dt, loop) && loop) {
			set_flag(TILE_LOOPED, true);
			if (get_flag(TILE_FLIP_X_ON_LOOP))
				set_flag(TILE_FLIP_X, !get_flag(TILE_FLIP_X));
		}
		unsigned int time = (unsigned int)(animation_timer.get_time() * 1000.f); // in milliseconds
		for (unsigned int frame_index = 0; frame_index < tile->animation.size(); ++frame_index) {
			unsigned int frame_duration = tile->animation[frame_index].duration_ms;
			if (time < frame_duration) {
				set_flag(TILE_FRAME_CHANGED, frame_index != _animation_frame);
				_animation_frame = frame_index;
				return;
			} else {
				time -= frame_duration;
			}
		}
		// Park on the last frame. We will for example get here if
		// animation_timer.get_time() == animation_timer.get_duration().
		_animation_frame = (unsigned int)tile->animation.size() - 1;
	}

	unsigned int Tile::get_animation_frame() const
	{
		return _animation_frame;
	}

	void Tile::set_flag(unsigned int flag, bool value)
	{
		if (value) {
			flags |= flag;
		} else {
			flags &= ~flag;
		}
	}

	bool Tile::get_flag(unsigned int flag) const
	{
		return (flags & flag) != 0;
	}

	bool _get_nth_bit(int value, int n)
	{
		return (value & (1 << n)) >> n;
	}

	void Tile::set_rotation(int clockwise_quarter_turns)
	{
		bool bit_0 = _get_nth_bit(clockwise_quarter_turns, 0);
		bool bit_1 = _get_nth_bit(clockwise_quarter_turns, 1);
		set_flag(TILE_FLIP_X, bit_0 != bit_1); // XOR
		set_flag(TILE_FLIP_Y, bit_1);
		set_flag(TILE_FLIP_DIAGONAL, bit_0);
	}

	void update_tiles(float dt)
	{
		_tile_time_accumulator += dt;
		for (auto [entity, tile] : _registry.view<Tile>().each()) {
			tile.update_animation(dt);
		}
		for (auto [entity, tile, body] : _registry.view<Tile, b2Body*>().each()) {
			tile.position = body->GetPosition();
		}
	}

	void add_tile_sprites_for_drawing(const Vector2f& camera_min, const Vector2f& camera_max)
	{
		sprites::Sprite sprite{};
		for (auto [entity, tile] : _registry.view<const Tile>().each()) {
			if (!tile.get_flag(TILE_VISIBLE)) continue;
			sprite.texture = tile.texture;
			if (sprite.texture == Handle<graphics::Texture>()) continue;
			Vector2u texture_size;
			graphics::get_texture_size(sprite.texture, texture_size.x, texture_size.y);
			sprite.min = tile.position - tile.pivot;
			if (sprite.min.x > camera_max.x || sprite.min.y > camera_max.y) continue;
			unsigned int left, top, width, height;
			tile.get_texture_rect(left, top, width, height);
			sprite.tex_min = { (float)left, (float)top };
			sprite.tex_max = { (float)left + width, (float)top + height };
			sprite.max = sprite.min + sprite.tex_max - sprite.tex_min;
			sprite.tex_min /= Vector2f(texture_size);
			sprite.tex_max /= Vector2f(texture_size);
			if (sprite.max.x < camera_min.x || sprite.max.y < camera_min.y) continue;
			sprite.shader = tile.shader;
			sprite.color = tile.color;
			sprite.sorting_layer = tile.sorting_layer;
			sprite.sorting_pos = sprite.min + tile.sorting_pivot;
			sprite.flags = 0;
			if (tile.get_flag(TILE_FLIP_X)) {
				sprite.flags |= sprites::SPRITE_FLIP_HORIZONTAL;
			}
			if (tile.get_flag(TILE_FLIP_Y)) {
				sprite.flags |= sprites::SPRITE_FLIP_VERTICAL;
			}
			if (tile.get_flag(TILE_FLIP_DIAGONAL)) {
				sprite.flags |= sprites::SPRITE_FLIP_DIAGONAL;
			}
#if 0
			sprite.pre_render_callback = nullptr;
			if (tile.get_class() == "grass") {
				sprite.pre_render_callback = [](const sprites::Sprite& sprite) {
					if (sprite.shader == Handle<graphics::Shader>()) return;
					graphics::set_uniform_1f(sprite.shader, "time", _tile_time_accumulator);
					graphics::set_uniform_2f(sprite.shader, "position", sprite.min.x, sprite.min.y);
				};
			}
#endif
			sprites::add(sprite);
		}
	}

	Tile& emplace_tile(entt::entity entity)
	{
		return _registry.emplace_or_replace<Tile>(entity);
	}

	Tile* get_tile(entt::entity entity)
	{
		return _registry.try_get<Tile>(entity);
	}

	bool remove_tile(entt::entity entity)
	{
		return _registry.remove<Tile>(entity);
	}
}
