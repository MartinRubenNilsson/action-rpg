#include "ecs_bomb.h"
#include <entt/entt.hpp>
#include "ecs_tile.h"
#include "ecs_common.h"

namespace ecs
{
    extern entt::registry _registry;

    float _bomb_elapsed_time = 0.f;

    void update_bombs(float dt)
    {
        _bomb_elapsed_time += dt;
        auto view = _registry.view<Bomb, Tile>();
        for (auto entity : view) {
            auto& bomb = view.get<Bomb>(entity);
            auto& tile = view.get<Tile>(entity);

            if (bomb.is_active) {
                // Update the timer
                if (bomb.timer.update(dt)) {
                    // The timer finished counting down
                    explode_bomb(entity, bomb.blast_radius);
                }
                else {
                    // Check if the bomb should start blinking
                    if (bomb.timer.get_progress() > 0.5f) {  // Start blinking at >50% progress
                        // Blinking effect
                        constexpr float BLINK_SPEED = 20.f;
                        float blink_fraction = 0.75f + 0.25f * std::sin(_bomb_elapsed_time * BLINK_SPEED);
                        tile.color.a = (sf::Uint8)(255 * blink_fraction);

                        bomb.should_blink = true;
                    }
                }
            }
        }
    }

    entt::entity create_bomb(const sf::Vector2f& position)
    {
        entt::entity bomb_entity = _registry.create();
        // Add a Bomb component
        Bomb bomb = {};
        bomb.timer.start();
        _registry.emplace<Bomb>(bomb_entity, bomb);
        // ... setup other necessary components like physics, tile, etc.

        Tile& tile = emplace_tile(bomb_entity);
        tile.set("bomb", "items1");
        tile.position = position;
        tile.pivot = sf::Vector2f(6.f, 6.f);

        return bomb_entity;
    }

    void explode_bomb(entt::entity bomb_entity, float blast_radius)
    {
        // Explosion logic goes here
        // You can use the blast_radius to determine the affected area
        // ...
        destroy_at_end_of_frame(bomb_entity);
    }
}
