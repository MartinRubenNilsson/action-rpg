#pragma once

namespace ecs
{
    struct Arrow
    {
        int damage = 0;
        float lifetime = 0.f; // unused right now
    };

    entt::entity create_arrow(const sf::Vector2f& position, const sf::Vector2f& velocity);
}