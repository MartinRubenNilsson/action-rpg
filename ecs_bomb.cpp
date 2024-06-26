#include "stdafx.h"
#include "ecs_bomb.h"
#include "ecs_tile.h"
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

    float _bomb_elapsed_time = 0.f;

    void _explode_bomb(entt::entity entity)
    {
        if (!_registry.all_of<Bomb>(entity)) return;
        Bomb& bomb = _registry.get<Bomb>(entity);
        apply_damage_in_circle({ DamageType::Explosion, 2, entity },
            bomb.explosion_center, bomb.explosion_radius);
        add_trauma_to_active_camera(0.8f);
        create_vfx(VfxType::Explosion, bomb.explosion_center);
        destroy_at_end_of_frame(entity);
        audio::play("event:/snd_bomb_explosion");
        audio::stop(bomb.fuse_sounds_event_id);
        postprocessing::create_shockwave(bomb.explosion_center);
    }

    void update_bombs(float dt)
    {
        _bomb_elapsed_time += dt;

        for (auto [entity, bomb, tile] : _registry.view<Bomb, Tile>().each()) {

            bomb.explosion_timer.update(dt);
            if (bomb.explosion_timer.finished()) {
                _explode_bomb(entity);
                continue;
            }

            // Start blinking at >50% progress
            if (bomb.explosion_timer.get_progress() < 0.5f) continue;  
            constexpr float BLINK_SPEED = 20.f;
            float blink_fraction = 0.75f + 0.25f * std::sin(_bomb_elapsed_time * BLINK_SPEED);
            tile.color.a = (sf::Uint8)(255 * blink_fraction);
        }
    }

    bool apply_damage_to_bomb(entt::entity entity, const Damage& damage)
    {
        // DON'T call _explode_bomb() directly here, I got a stack overflow
        // when two bombs kept on exploding each other in an infinite loop!
        if (damage.amount <= 0) return false;
        if (!_registry.all_of<Bomb>(entity)) return false;
        Bomb& bomb = _registry.get<Bomb>(entity);
        bomb.explosion_timer.finish();
        return true;
    }

    entt::entity create_bomb(const sf::Vector2f& position)
    {
        entt::entity entity = _registry.create();
        set_class(entity, "bomb");
        {
            Bomb& bomb = _registry.emplace<Bomb>(entity);
            bomb.explosion_timer.start();
            audio::EventOptions options{};
            options.position = position;
            bomb.fuse_sounds_event_id = audio::play("event:/snd_bomb_fuse", options);
            bomb.explosion_center = position;
        }
        {
            b2BodyDef body_def{};
            body_def.type = b2_staticBody;
            body_def.position.Set(position.x, position.y);
            body_def.fixedRotation = true;
            b2Body* body = emplace_body(entity, body_def);
            b2CircleShape shape{};
            shape.m_radius = 4.f;
            body->CreateFixture(&shape, 0.f);
        }
        {
            Tile& tile = emplace_tile(entity);
            tile.set_tile("bomb", "items1");
            tile.position = position;
            tile.pivot = sf::Vector2f(8.f, 16.f);
            tile.sorting_pivot = sf::Vector2f(8.f, 16.f);
        }
        return entity;
    }
}
