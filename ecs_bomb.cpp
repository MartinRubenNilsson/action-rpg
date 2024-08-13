#include "stdafx.h"
#include "ecs_bomb.h"
#include "ecs_sprite.h"
#include "ecs_animation.h"
#include "ecs_common.h"
#include "ecs_camera.h"
#include "ecs_physics.h"
#include "ecs_vfx.h"
#include "ecs_damage.h"
#include "audio.h"
#include "postprocessing.h"

namespace ecs
{
    extern entt::registry _registry;

    void update_bombs(float dt)
    {
        for (auto [entity, bomb, body] : _registry.view<Bomb, b2Body*>().each()) {

            if (!bomb.ignited) continue;
            bomb.explosion_timer.update(dt);

            const Vector2f center = body->GetPosition();
            audio::set_event_position(bomb.fuse_sound, center);

            if (!bomb.explosion_timer.finished()) continue;

            // Explode the bomb
            apply_damage_in_circle({ DamageType::Explosion, 2, entity }, center, bomb.explosion_radius);
            add_trauma_to_active_camera(0.8f);
            create_vfx(VfxType::Explosion, center);
            destroy_at_end_of_frame(entity);
            audio::create_event({ .path = "event:/snd_bomb_explosion", .position = center });
            audio::stop_event(bomb.fuse_sound);
            postprocessing::create_shockwave(center);
		}

        for (auto [entity, bomb, sprite] : _registry.view<Bomb, sprites::Sprite>().each()) {

            // Start blinking at >50% progress 
            if (bomb.explosion_timer.get_progress() <= 0.5f) continue;

            constexpr float BLINK_FREQUENCY = 6.f; // in Hz
            float blink_fraction = 0.75f + 0.25f * sin(bomb.explosion_timer.get_time_left() * BLINK_FREQUENCY * M_2PI);
            sprite.color.g = (unsigned char)(255 * blink_fraction);
            sprite.color.b = (unsigned char)(255 * blink_fraction);
        }
    }

    Bomb& emplace_bomb(entt::entity entity, const Bomb& bomb)
    {
        return _registry.emplace_or_replace<Bomb>(entity, bomb);
    }

    Bomb* get_bomb(entt::entity entity)
    {
        return _registry.try_get<Bomb>(entity);
    }

    entt::entity create_bomb(const Vector2f& position)
    {
        entt::entity entity = _registry.create();
        set_class(entity, "bomb");
        emplace_bomb(entity);
        ignite_bomb(entity);
        {
            b2BodyDef body_def{};
            body_def.type = b2_staticBody;
            body_def.position = position;
            body_def.fixedRotation = true;
            b2Body* body = emplace_body(entity, body_def);
            b2CircleShape shape{};
            shape.m_radius = 4.f;
            body->CreateFixture(&shape, 0.f);
        }
#if 0
        {
            Animation& sprite = emplace_animation(entity);
            sprite.set_tileset("items1");
            sprite.set_tile(TILE_ID_ITEM_POTION);
            sprite.position = position;
            sprite.pivot = Vector2f(8.f, 16.f);
            sprite.sorting_pivot = sprite.pivot;
        }
#endif
        return entity;
    }

    void ignite_bomb(entt::entity entity)
    {
        Bomb* bomb = get_bomb(entity);
		if (!bomb) return;
		if (bomb->ignited) return;
		bomb->ignited = true;
        bomb->explosion_timer.start();
		bomb->fuse_sound = audio::create_event({ .path = "event:/snd_bomb_fuse" });
    }

    bool apply_damage_to_bomb(entt::entity entity, const Damage& damage)
    {
        // DON'T call _explode_bomb() directly here, I got a stack overflow
        // when two bombs kept on exploding each other in an infinite loop!
        if (damage.amount <= 0) return false;
        Bomb* bomb = get_bomb(entity);
        if (!bomb) return false;
        bomb->explosion_timer.finish();
        return true;
    }
}
