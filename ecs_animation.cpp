#include "stdafx.h"
#include "ecs_animation.h"
#include "tiled.h"
#include "graphics.h"
#include "sprites.h"

namespace ecs
{
	extern entt::registry _registry;

#if 0
	bool Animation::set_tileset(Handle<tiled::Tileset> handle)
	{
		const tiled::Tileset* tileset = tiled::get_tileset(handle);
		if (!tileset) return false;
		this->tileset = handle;
		this->tile_id = 0;
		return true;
	}

	bool Animation::set_tileset(const std::string& tileset_name)
	{
		return set_tileset(tiled::find_tileset_by_name(tileset_name));
	}

	void Animation::set_flag(unsigned int flag, bool value)
	{
		if (value) {
			flags |= flag;
		} else {
			flags &= ~flag;
		}
	}

	bool Animation::get_flag(unsigned int flag) const
	{
		return (flags & flag) != 0;
	}

	bool _get_nth_bit(int value, int n)
	{
		return (value & (1 << n)) >> n;
	}

	void Animation::set_rotation(int clockwise_quarter_turns)
	{
		bool bit_0 = _get_nth_bit(clockwise_quarter_turns, 0);
		bool bit_1 = _get_nth_bit(clockwise_quarter_turns, 1);
		set_flag(TILE_FLIP_X, bit_0 != bit_1); // XOR
		set_flag(TILE_FLIP_Y, bit_1);
		set_flag(TILE_FLIP_DIAGONAL, bit_0);
	}
#endif

	void update_animations(float dt)
	{
		for (auto [entity, animation] : _registry.view<Animation>().each()) {

			animation.frame_changed = false;
			animation.looped = false;

			const tiled::Tileset* tileset = tiled::get_tileset(animation.tileset);
			if (!tileset) continue;
			if (animation.tile_id >= tileset->tiles.size()) continue;
			const tiled::Tile& tile = tileset->tiles[animation.tile_id];

			if (tile.animation.empty()) continue;

			unsigned int duration_ms = 0; // in milliseconds
			for (const tiled::Frame& frame : tile.animation) {
				duration_ms += frame.duration_ms;
			}

			if (!duration_ms) continue;

			constexpr float MILLISECONDS_PER_SECOND = 1000.f;
			// TODO: support for negative speed
			const float delta_progress = std::max(animation.speed, 0.f) * dt * MILLISECONDS_PER_SECOND / duration_ms;

			animation.progress += delta_progress;
			if (animation.progress >= 1.f) {
				if (animation.loop) {
					animation.looped = true;
					animation.progress = fmodf(animation.progress, 1.f);
				} else {
					animation.progress = 1.f;
				}
			}

			unsigned int time_ms = (unsigned int)(animation.progress * duration_ms);

			for (unsigned int frame_id = 0; frame_id < tile.animation.size(); ++frame_id) {
				const tiled::Frame& frame = tile.animation[frame_id];
				if (time_ms >= frame.duration_ms) {
					time_ms -= frame.duration_ms;
					continue;
				}
				if (frame_id != animation.frame_id) {
					animation.animated_tile_id = frame.tile_id;
					animation.frame_id = frame_id;
					animation.frame_changed = true;
				}
				break;
			}
		}
	}

	void update_animated_sprites(float dt)
	{
		for (auto [entity, sprite, animation] : _registry.view<sprites::Sprite, const Animation>().each()) {

			const tiled::Tileset* tileset = tiled::get_tileset(animation.tileset);
			if (!tileset) continue;
			if (animation.animated_tile_id >= tileset->tiles.size()) continue;

			const tiled::TextureRect tex_rect = tileset->get_texture_rect(animation.animated_tile_id);
			sprite.tex_pos = { (float)tex_rect.x, (float)tex_rect.y };
			sprite.tex_size = { (float)tex_rect.w, (float)tex_rect.h };
			sprite.size = sprite.tex_size;

			Vector2u texture_size;
			graphics::get_texture_size(sprite.texture, texture_size.x, texture_size.y);
			sprite.tex_pos /= Vector2f(texture_size);
			sprite.tex_size /= Vector2f(texture_size);
		}
	}

	Animation& emplace_animation(entt::entity entity)
	{
		return _registry.emplace_or_replace<Animation>(entity);
	}

	Animation* get_animation(entt::entity entity)
	{
		return _registry.try_get<Animation>(entity);
	}

	bool remove_animation(entt::entity entity)
	{
		return _registry.remove<Animation>(entity);
	}
}
