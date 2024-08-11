#include "stdafx.h"
#include "ecs_tile.h"
#include "tiled.h"
#include "graphics.h"
#include "sprites.h"

namespace ecs
{
	extern entt::registry _registry;
	float _tile_time_accumulator = 0.f;

	bool Tile::set_tile(const tiled::Tile* tile)
	{
		if (!tile) return false;
		if (tile->tileset == tileset && tile->id == id) return false;
		tileset = tile->tileset;
		id = tile->id;
		id_animated = tile->id;
		row = tile->x;
		column = tile->y;
		texture_rect_left = tile->left;
		texture_rect_top = tile->top;
		texture_rect_width = tile->width;
		texture_rect_height = tile->height;
		animation_frame = 0;
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
		return true;
	}

	bool Tile::set_tileset(const std::string& tileset_name)
	{
		const tiled::Tileset* tileset_ptr = tiled::find_tileset_by_name(tileset_name);
		if (!tileset_ptr) return false;
		tileset = tileset_ptr->handle;
		id = 0;
		id_animated = 0;
		row = 0;
		column = 0;
		texture_rect_left = 0;
		texture_rect_top = 0;
		texture_rect_width = tileset_ptr->tile_width;
		texture_rect_height = tileset_ptr->tile_height;
		texture = graphics::load_texture(tileset_ptr->image_path);
		animation_timer = Timer();
		animation_frame = 0;
		set_flag(TILE_FRAME_CHANGED, false);
		set_flag(TILE_LOOPED, false);
		return true;
	}

	bool Tile::set_tile(unsigned int id)
	{
		if (this->id == id) return false;
		const tiled::Tileset* tileset = tiled::get_tileset(this->tileset);
		if (!tileset) return false;
		if (id >= tileset->tiles.size()) return false;
		return set_tile(&tileset->tiles[id]);
	}

	bool Tile::set_tile(unsigned int x, unsigned int y)
	{
		const tiled::Tileset* tileset = tiled::get_tileset(this->tileset);
		if (!tileset) return false;
		const unsigned int id = y * tileset->columns + x;
		if (this->id == id) return false;
		if (id >= tileset->tiles.size()) return false;
		return set_tile(&tileset->tiles[id]);
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

			const tiled::Tileset* tileset = tiled::get_tileset(tile.tileset);
			if (!tileset) continue;
			if (tile.id >= tileset->tiles.size()) continue;
			const tiled::Tile& tiled_tile = tileset->tiles[tile.id];

			tile.row = tiled_tile.x;
			tile.column = tiled_tile.y;
			tile.texture_rect_left = tiled_tile.left;
			tile.texture_rect_top = tiled_tile.top;
			tile.texture_rect_width = tiled_tile.width;
			tile.texture_rect_height = tiled_tile.height;

			if (!tile.animation_timer.get_duration()) continue;
			tile.set_flag(TILE_FRAME_CHANGED, false);
			tile.set_flag(TILE_LOOPED, false);
			const bool loop = tile.get_flag(TILE_LOOP);

			if (tile.animation_timer.update(tile.animation_speed * dt, loop) && loop) {
				tile.set_flag(TILE_LOOPED, true);
				if (tile.get_flag(TILE_FLIP_X_ON_LOOP)) {
					tile.set_flag(TILE_FLIP_X, !tile.get_flag(TILE_FLIP_X));
				}
			}

			unsigned int time_ms = (unsigned int)(tile.animation_timer.get_time() * 1000.f); // in milliseconds

			bool found_frame = false;
			for (unsigned int f = 0; f < tiled_tile.animation.size(); ++f) {
				const tiled::Frame& frame = tiled_tile.animation[f];
				if (frame.duration_ms <= time_ms) {
					time_ms -= frame.duration_ms;
					continue;
				}
				tile.set_flag(TILE_FRAME_CHANGED, f != tile.animation_frame);
				tile.animation_frame = f;
				tile.id_animated = frame.tile_id;
				const tiled::Tile& frame_tile = tileset->tiles[frame.tile_id];
				tile.row = frame_tile.x;
				tile.column = frame_tile.y;
				tile.texture_rect_left = frame_tile.left;
				tile.texture_rect_top = frame_tile.top;
				tile.texture_rect_width = frame_tile.width;
				tile.texture_rect_height = frame_tile.height;
				found_frame = true;
				break;
			}

			if (!found_frame) {
				// Park on the last frame. We will for example get here if
				// animation_timer.get_time() == animation_timer.get_duration().
				tile.animation_frame = (unsigned int)tiled_tile.animation.size() - 1;
			}
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
			sprite.tex_min = { (float)tile.texture_rect_left, (float)tile.texture_rect_top };
			sprite.tex_max = { (float)tile.texture_rect_left + tile.texture_rect_width, (float)tile.texture_rect_top + tile.texture_rect_height };
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
			if (tile_ptr.get_class() == "grass") {
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
