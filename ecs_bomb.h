#pragma once

#include <entt/entt.hpp>

namespace ecs {

    struct Bomb {
        float damage;  // The damage the bomb deals
        float blast_radius;  // The effect radius of the bomb's explosion
        bool is_active;  // Is the bomb currently active (has it been dropped/placed in the game world)
        bool should_blink;  // Should the bomb start blinking to indicate it's about to explode
    };

    entt::entity create_bomb(const sf::Vector2f position);
}