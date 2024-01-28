#include "stdafx.h"
#include "ecs_ai.h"
#include "ecs_ai_knowledge.h"
#include "ecs_ai_action.h"
#include "console.h"
#include "map.h"

namespace ecs
{
    extern entt::registry _registry;

    void _use_ai_knowledge_to_decide_next_ai_action(float dt)
    {
        // VERY IMPORTANT:
        // 
        // In this function, the AiWorld, AiKnowledge and AiType are READ-ONLY.
        // The AiAction component is the only one that can be modified.
        // Use ONLY the AiWorld and AiKnowledge to decide what the AiAction should be,
        // do not access the game world directly! If you need more info to decide
        // the AiAction, add this info to AiKnowledge and/or the AiWorld.
        // The AiAction will run independently and is allowed to modify the game world.

        const AiWorld& world = get_ai_world();

        for (auto [entity, knowledge, ai_type, action] :
            _registry.view<const AiKnowledge, const AiType, const AiAction>().each()) {

            switch (ai_type) {
            case AiType::Slime: {
                if (!_registry.valid(world.player.entity)) break;

                // The radius of the circle around the player that the slime will try to reach.
                const float PURSUE_RADIUS = 50.f;
                // The duration of the wait action.
                const float WAIT_DURATION = 1.f;
                // When player is closer than this, start fleeing
                const float FLEE_RADIUS = 30.f; 

                float distance_to_player = length(world.player.position - knowledge.me.position);

                if (action.type == AiActionType::Wait && action.status == AiActionStatus::Running) {
                    // If the slime is waiting, do nothing.
                } else if (action.type == AiActionType::Flee && action.status == AiActionStatus::Running) {
					// If the slime is fleeing, keep doing that.
				} else if (action.type == AiActionType::Flee && action.status == AiActionStatus::Succeeded) {
                    // If the slime has finished fleeing, wait for a while.
					ai_wait(entity, WAIT_DURATION);
                } else if (distance_to_player < FLEE_RADIUS) {
                    // Player is too close, start fleeing
                    ai_flee(entity, world.player.entity, knowledge.me.speed, PURSUE_RADIUS);
                } else if (distance_to_player > PURSUE_RADIUS) {
                    // Player is too far, start pursuing
					ai_pursue(entity, world.player.entity, knowledge.me.speed, PURSUE_RADIUS);
				} else {
                    // Else, start moving towards the player.
                    ai_pursue(entity, world.player.entity, knowledge.me.speed, PURSUE_RADIUS);
                }
                break;
            }
            }
        }
    }

    void update_ai(float dt)
    {
        update_ai_knowledge(dt);
        _use_ai_knowledge_to_decide_next_ai_action(dt);
        update_ai_actions(dt);
    }

    void debug_draw_ai(sf::RenderTarget& target)
    {
        const AiWorld& world = get_ai_world();

        for (const AiEntityInfo& ai : world.ais) {
            std::vector<sf::Vector2f> path = map::pathfind(ai.position, world.player.position);
            if (path.size() < 2) continue;
            std::vector<sf::Vertex> vertices(path.size());
            for (size_t i = 0; i < path.size(); ++i) {
				vertices[i].position = path[i];
				vertices[i].color = sf::Color::Red;
			}
            target.draw(vertices.data(), vertices.size(), sf::PrimitiveType::LineStrip);
        }
    }

    void emplace_ai(entt::entity entity, AiType type)
    {
        _registry.emplace_or_replace<AiKnowledge>(entity);
        _registry.emplace_or_replace<AiType>(entity, type);
		_registry.emplace_or_replace<AiAction>(entity);
    }
}
