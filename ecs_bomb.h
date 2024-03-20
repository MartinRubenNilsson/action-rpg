#pragma once
#include "timer.h"

namespace ecs
{
    struct Bomb
    {
        Timer explosion_timer = { 3.f };
        sf::Vector2f explosion_center;
        float explosion_radius = 16.f;
    };

    void update_bombs(float dt);

    entt::entity create_bomb(const sf::Vector2f& position);
}