#include "ecs_bomb.h"
#include <entt/entt.hpp>
#include "ecs_tile.h"
#include "ecs_common.h"
#include "audio.h"
#include "ecs_camera.h"
#include "ecs_physics.h"
#include "ecs_vfx.h"
#include "postprocess.h"


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
                    explode_bomb(entity, bomb.blast_radius, tile.position);

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

    void explode_bomb(entt::entity bomb_entity, float blast_radius, const sf::Vector2f& position)
    {
        // Explosion logic goes here
        // You can use the blast_radius to determine the affected area
        // ...
        sf::Vector2f box_center = position;
        sf::Vector2f box_min = box_center - sf::Vector2f(12.f, 12.f);
        sf::Vector2f box_max = box_center + sf::Vector2f(12.f, 12.f);

        for (const BoxHit& hit : boxcast(box_min, box_max)) {
            std::string class_ = get_class(hit.entity);
            if (class_ == "slime") {
                destroy_at_end_of_frame(hit.entity);
            }
        }
        postprocess::shockwaves.push_back({ position, 0.1f, 0.1f, 0.1f });
        ecs::add_trauma_to_active_camera(0.8f);
        create_vfx(VfxType::Explosion, position);
        audio::play("event:/snd_glass_smash");
        destroy_at_end_of_frame(bomb_entity);
    }
}
