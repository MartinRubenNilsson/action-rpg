#include "ecs_bomb.h"
#include <entt/entt.hpp>

namespace ecs {

    extern entt::registry _registry;  // Assuming you have a global EnTT registry

    void update_bomb_system(float dt) {
        auto view = _registry.view<Bomb>();
        for (auto entity : view) {
            auto& bomb = view.get<Bomb>(entity);

            if (bomb.is_active) {
                // Update the countdown timer

                // Check if the bomb should start blinking

                // Explosion logic
            }
        }
    }

    void explode_bomb(entt::entity bomb_entity, float blast_radius) {
        // Explosion logic goes here
        // You can use the blast_radius to determine the affected area
        // ...
    }

    entt::entity create_bomb(const sf::Vector2f position)
    {
        entt::entity bomb_entity = _registry.create();
        // Add a Bomb component
        Bomb bomb = { 5.0f, 20.0f, 10, false }; // example values
        _registry.emplace<Bomb>(bomb_entity, bomb);
        // ... setup other necessary components like physics, tile, etc.

        return bomb_entity;
    }
}
