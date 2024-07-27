#pragma once
#include "timer.h"

namespace ecs
{
    struct Bomb
    {
        Timer explosion_timer = { 3.f };
        Vector2f explosion_center;
        float explosion_radius = 20.f;
        Handle<audio::Event> fuse_sound;
    };

    void update_bombs(float dt);

    Bomb& emplace_bomb(entt::entity entity, const Bomb& bomb);
    Bomb* get_bomb(entt::entity entity);

    entt::entity create_bomb(const Vector2f& position);
}