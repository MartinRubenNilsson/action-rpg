#pragma once
#include "timer.h"

namespace ecs
{
    struct Bomb
    {
        Timer explosion_timer = { 3.f };
        Vector2f explosion_center;
        float explosion_radius = 20.f;
        audio::EventHandle fuse_sound = audio::EventHandle::Invalid;
    };

    void update_bombs(float dt);

    entt::entity create_bomb(const Vector2f& position);
}