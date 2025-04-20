#include "stdafx.h"
#include "ecs_chest.h"
#include "ecs_animations.h"
#include "ecs_bomb.h"
#include "ecs_physics.h"
#include "ui_textbox.h"
#include "map.h"
#include "audio.h"

namespace ecs {
    extern entt::registry _registry;

    Chest& emplace_chest(entt::entity entity, const Chest& chest) {
        return _registry.emplace_or_replace<Chest>(entity, chest);
    }

    Chest* get_chest(entt::entity entity) {
        return _registry.try_get<Chest>(entity);
    }

    void open_chest(entt::entity entity, bool ignore_contents) {
        Chest* chest = get_chest(entity);
        if (!chest) return;
        if (chest->opened) return;
        chest->opened = true;

        if (TileAnimation* animation = get_tile_animation(entity)) {
            // At the time I'm writing this code, the treasure chest tileset has 6 rows and 5 columns. 
            // Each chest's closed sprite is on an even row and its corresponding open sprite is right below it.
            constexpr unsigned int COLUMNS = 5;
            unsigned int x = animation->tile_id / COLUMNS;
            unsigned int y = animation->tile_id % COLUMNS;
            if (x % 2 == 0) {
                // If the chest is closed, we open it by setting the tile to the one right below it.
                animation->tile_id = (x + 1) * COLUMNS + y;
            }
        }

        if (ignore_contents) return;

        switch (chest->type) {
        case ChestType::Normal: {
            ui::open_textbox({ .text = "You open the chest and find... nothing!" });
            // By marking the chest as opened, we ensure that the chest
            // stays open when the player re-enters the map.
            map::mark_chest_as_opened(entity);
        } break;
        case ChestType::Bomb: {
            emplace_bomb(entity);
            ignite_bomb(entity);
            // By marking the entity as destroyed, we ensure that the chest
            // stays removed from the map when the player re-enters the map.
            map::mark_entity_as_destroyed(entity);
        } break;
        }

        audio::create_event({ .path = "event:/props/chest/open" });
    }

    void interact_with_chest(entt::entity entity) {
        open_chest(entity);
    }
}