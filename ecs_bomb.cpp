#include "stdafx.h"
#include "ecs_bomb.h"
#include "ecs_tile.h"
#include "ecs_common.h"
#include "ecs_camera.h"
#include "ecs_physics.h"
#include "ecs_vfx.h"
#include "ecs_damage.h"
#include "debug_draw.h"
#include "audio.h"
#include "postprocessing.h"

namespace ecs
{
    extern entt::registry _registry;

    float _bomb_elapsed_time = 0.f;

    void _explode_bomb(entt::entity entity, float blast_radius, const sf::Vector2f& position)
    {
        create_vfx(VfxType::Explosion, position);
        add_trauma_to_active_camera(0.8f);
        destroy_at_end_of_frame(entity);
        audio::play("event:/snd_glass_smash");
        postprocessing::create_shockwave(position);

        sf::Vector2f box_center = position;
        sf::Vector2f box_min = box_center - sf::Vector2f(12.f, 12.f);
        sf::Vector2f box_max = box_center + sf::Vector2f(12.f, 12.f);
        debug::draw_box(box_min, box_max, sf::Color::Red, 0.3f);

        Damage damage{};
        damage.type = DamageType::Explosion;
        damage.amount = 2;
        for (const BoxHit& hit : boxcast(box_min, box_max)) {
            apply_damage(hit.entity, damage);
        }
    }

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
                    _explode_bomb(entity, bomb.blast_radius, tile.position);

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
        entt::entity entity = _registry.create();
        {
            Bomb& bomb = _registry.emplace<Bomb>(entity);
            bomb.timer.start();
        }
        {
            Tile& tile = emplace_tile(entity);
            tile.set_sprite("bomb", "items1");
            tile.position = position;
            tile.pivot = sf::Vector2f(8.f, 16.f);
            tile.sorting_pivot = sf::Vector2f(8.f, 16.f);
        }
        return entity;
    }
}
