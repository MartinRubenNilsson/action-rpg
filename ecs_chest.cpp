#include "stdafx.h"
#include "ecs_chest.h"
#include "ecs_tile.h"
#include "map.h"
#include "ui_textbox.h"
#include "ecs_bomb.h"
#include "ecs_physics.h"

namespace ecs
{
    extern entt::registry _registry;

    Chest& emplace_chest(entt::entity entity, const Chest& chest)
    {
        return _registry.emplace_or_replace<Chest>(entity, chest);
    }

    Chest* get_chest(entt::entity entity)
    {
        return _registry.try_get<Chest>(entity);
    }

    void open_chest(entt::entity entity, bool ignore_contents)
    {
        Chest* chest = get_chest(entity);
        if (!chest) return;
        if (chest->opened) return;
        chest->opened = true;

        if (Tile* tile = get_tile(entity)) {
            // We assume that the closed chest tile is on an even row,
            // and the corresponding open chest tile is on the next row.
            Vector2u coords = tile->get_coords();
            if (coords.y % 2 == 0) {
                tile->set_tile(coords.x, coords.y + 1);
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
    }

    void interact_with_chest(entt::entity entity)
    {
        open_chest(entity);
    }
}