#pragma once
#include "timer.h"

namespace ecs
{
    struct Bomb
    {
        Timer explosion_timer = { 3.f };
        Vector2f explosion_center;
        float explosion_radius = 20.f;
        int fuse_sounds_event_id = -1;
    };

    void update_bombs(float dt);

    entt::entity create_bomb(const Vector2f& position);
}