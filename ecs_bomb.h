#pragma once
#include "timer.h"

namespace ecs
{
    struct Bomb
    {
        Timer timer = { 3.f };  // The timer for the bomb's explosion
        float damage{ 3.f };  // The damage the bomb deals
        float blast_radius{ 16.f };  // The effect radius of the bomb's explosion
        bool is_active = true;  // Is the bomb currently active (has it been dropped/placed in the game world)
        bool should_blink = false;  // Should the bomb start blinking to indicate it's about to explode
        sf::Vector2f position{};  // The position of the bomb
    };

    void update_bombs(float dt);

    entt::entity create_bomb(const sf::Vector2f& position);
    void explode_bomb(entt::entity bomb_entity, float blast_radius);
}