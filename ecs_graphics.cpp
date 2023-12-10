#include "ecs_graphics.h"
#include "defines.h"
#include "physics_helpers.h"
#include "tiled.h"

namespace ecs
{
	extern entt::registry _registry;

	Animation::Animation(const tiled::Tile* tile)
		: _tile(tile)
		, _frame(tile)
		, _timer(tiled::get_animation_duration(tile->animation) / 1000.f)
	{
		_timer.start();
	}

	bool Animation::update(float dt)
	{
		_timer.update(speed * dt, loop);
		uint32_t time_in_ms = (uint32_t)(_timer.get_time() * 1000.f);
		bool changed = false;
		if (const tiled::Tile* next_frame = tiled::sample_animation(_tile->animation, time_in_ms)) {
			changed = (next_frame != _frame);
			_frame = next_frame;
		}
		return changed;
	}

	bool Animation::play(const std::string& tile_class)
	{
		if (tile_class.empty()) return false;
		if (tile_class == _frame->class_) return false;
		for (const tiled::Tile& tile : _tile->tileset->tiles) {
			if (tile.class_ == tile_class) {
				_tile = &tile;
				_frame = &tile;
				_timer = Timer(tiled::get_animation_duration(tile.animation) / 1000.f);
				_timer.start();
				return true;
			}
		}
		return false;
	}

	bool Animation::is_playing(const std::string& tile_class) const {
		return _tile->class_ == tile_class;
	}

	void update_graphics(float dt)
	{
		for (auto [entity, anim] : _registry.view<Animation>().each()) {
			anim.update(dt);
			_registry.emplace_or_replace<const tiled::Tile*>(entity, anim.get_frame());
		}

		for (auto [entity, sprite, tile] : _registry.view<Sprite, const tiled::Tile*>().each()) {
			if (!tile) continue;
			sprite.sprite.setTexture(*tile->sprite.getTexture());
			sprite.sprite.setTextureRect(tile->sprite.getTextureRect());
		}

		for (auto [entity, sprite, body] : _registry.view<Sprite, b2Body*>().each()) {
			sprite.sprite.setPosition(get_position(body) * PIXELS_PER_METER);
		}
	}

	void emplace_sprite(entt::entity entity, const Sprite& sprite) {
		_registry.emplace_or_replace<Sprite>(entity, sprite);
	}

	void emplace_animation(entt::entity entity, const Animation& anim) {
		_registry.emplace_or_replace<Animation>(entity, anim);
	}
}
