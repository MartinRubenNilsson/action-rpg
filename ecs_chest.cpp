#include "stdafx.h"
#include "ecs_chest.h"
#include "ecs_tile.h"
#include "console.h"

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

    void open_chest(entt::entity entity)
    {
        Chest* chest = get_chest(entity);
        if (!chest) return;
        if (chest->opened) return;
        chest->opened = true;

        if (ecs::Tile* tile = ecs::get_tile(entity)) {
            Vector2u coords = tile->get_coords();
            if (coords.y % 2 == 0) {
                tile->set_tile(coords.x, coords.y + 1);
            }
        }
    }
}