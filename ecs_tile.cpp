#include "stdafx.h"
#include "ecs_tile.h"
#include "tiled.h"
#include "graphics.h"
#include "sprites.h"

namespace ecs
{
	extern entt::registry _registry;

	void Tile::set_texture_rect(unsigned int id)
	{
		const tiled::Tileset* tileset = tiled::get_tileset(this->tileset);
		if (!tileset) return;
		if (id >= tileset->tiles.size()) return;
		const unsigned int x = id % tileset->columns;
		const unsigned int y = id / tileset->columns;
		this->texture_rect_left = x * (tileset->tile_width + tileset->spacing) + tileset->margin;
		this->texture_rect_top = y * (tileset->tile_height + tileset->spacing) + tileset->margin;
		this->texture_rect_width = tileset->tile_width;
		this->texture_rect_height = tileset->tile_height;
	}

	bool Tile::set_tileset(Handle<tiled::Tileset> handle)
	{
		const tiled::Tileset* tileset = tiled::get_tileset(handle);
		if (!tileset) return false;
		this->tileset = handle;
		this->texture = graphics::load_texture(tileset->image_path);
		this->id = UINT_MAX; // force an update in set_tile()
		set_tile(0); // set to the first tile in the tileset
		return true;
	}

	bool Tile::set_tileset(const std::string& tileset_name)
	{
		const tiled::Tileset* tileset = tiled::find_tileset_by_name(tileset_name);
		if (!tileset) return false;
		return set_tileset(tileset->handle);
	}

	bool Tile::set_tile(unsigned int id)
	{
		if (id == this->id) return false;
		const tiled::Tileset* tileset = tiled::get_tileset(this->tileset);
		if (!tileset) return false;
		if (id >= tileset->tiles.size()) return false;
		this->id = id;
		set_texture_rect(id);
		return true;
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

	void update_tile_positions(float dt)
	{
		for (auto [entity, tile, body] : _registry.view<Tile, b2Body*>().each()) {
			tile.position = body->GetPosition();
		}
	}

	void update_tile_animations(float dt)
	{
		for (auto [entity, animation] : _registry.view<TileAnimation>().each()) {
			animation.frame_changed = false;
		}

		for (auto [entity, tile, animation] : _registry.view<Tile, TileAnimation>().each()) {

			const tiled::Tileset* tiled_tileset = tiled::get_tileset(tile.tileset);
			if (!tiled_tileset) continue;
			if (tile.id >= tiled_tileset->tiles.size()) continue;
			const tiled::Tile& tiled_tile = tiled_tileset->tiles[tile.id];

			if (tiled_tile.animation.empty()) continue;

			unsigned int duration_ms = 0; // in milliseconds
			for (const tiled::Frame& frame : tiled_tile.animation) {
				duration_ms += frame.duration_ms;
			}

			if (!duration_ms) continue;

			constexpr float MILLISECONDS_PER_SECOND = 1000.f;
			// TODO: support for negative speed
			const float delta_progress = std::max(animation.speed, 0.f) * dt * MILLISECONDS_PER_SECOND / duration_ms;

			animation.progress += delta_progress;
			if (animation.progress >= 1.f) {
				if (animation.loop) {
					animation.progress = fmodf(animation.progress, 1.f);
					if (tile.get_flag(TILE_FLIP_X_ON_LOOP)) {
						tile.set_flag(TILE_FLIP_X, !tile.get_flag(TILE_FLIP_X));
					}
				} else {
					animation.progress = 1.f;
				}
			}

			unsigned int elapsed_time_ms = (unsigned int)(animation.progress * duration_ms);

			for (unsigned int frame = 0; frame < tiled_tile.animation.size(); ++frame) {
				const tiled::Frame& tiled_frame = tiled_tile.animation[frame];
				if (elapsed_time_ms >= tiled_frame.duration_ms) {
					elapsed_time_ms -= tiled_frame.duration_ms;
					continue;
				}
				if (frame != animation.frame) {
					animation.frame = frame;
					animation.frame_changed = true;
					tile.set_texture_rect(tiled_frame.tile_id);
				}
				break;
			}
		}
	}

	void add_tile_sprites_for_drawing(const Vector2f& camera_min, const Vector2f& camera_max)
	{
		sprites::Sprite sprite{};
		for (auto [entity, tile] : _registry.view<const Tile>().each()) {
			if (!tile.get_flag(TILE_VISIBLE)) continue;
			sprite.min = tile.position - tile.pivot;
			if (sprite.min.x > camera_max.x || sprite.min.y > camera_max.y) continue;
			const Vector2f tex_size = { (float)tile.texture_rect_width, (float)tile.texture_rect_height };
			sprite.max = sprite.min + tex_size;
			if (sprite.max.x < camera_min.x || sprite.max.y < camera_min.y) continue;
			sprite.tex_min = { (float)tile.texture_rect_left, (float)tile.texture_rect_top };
			sprite.tex_max = sprite.tex_min + tex_size;
			Vector2u texture_size;
			graphics::get_texture_size(tile.texture, texture_size.x, texture_size.y);
			sprite.tex_min /= Vector2f(texture_size);
			sprite.tex_max /= Vector2f(texture_size);
			sprite.shader = tile.shader;
			sprite.texture = tile.texture;
			sprite.color = tile.color;
			sprite.sorting_layer = tile.sorting_layer;
			sprite.sorting_pos = sprite.min + tile.sorting_pivot;
			sprite.flags = 0;
			if (tile.get_flag(TILE_FLIP_X)) {
				sprite.flags |= sprites::SPRITE_FLIP_HORIZONTALLY;
			}
			if (tile.get_flag(TILE_FLIP_Y)) {
				sprite.flags |= sprites::SPRITE_FLIP_VERTICALLY;
			}
			if (tile.get_flag(TILE_FLIP_DIAGONAL)) {
				sprite.flags |= sprites::SPRITE_FLIP_DIAGONALLY;
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

	TileAnimation& emplace_tile_animation(entt::entity entity)
	{
		return _registry.emplace_or_replace<TileAnimation>(entity);
	}
}
