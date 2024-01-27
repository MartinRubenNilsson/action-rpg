#include "stdafx.h"
#include "ecs_ai.h"
#include "ecs_ai_knowledge.h"
#include "ecs_ai_action.h"
#include "console.h"

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

                // The distance to the target position at which the MoveTo action is considered successful.
                const float ACCEPTANCE_RADIUS = 20.f;
                // The duration of the wait action.
                const float WAIT_DURATION = 2.f;

                if (action.type == AiActionType::Wait && action.status == AiActionStatus::Running) {
                    // If the slime is waiting, do nothing.
                } else if (action.type == AiActionType::MoveTo && action.status == AiActionStatus::Running) {
                    // If the slime is moving towards the player, keep doing it.
					ai_move_to(entity, world.player.position, knowledge.me.speed, ACCEPTANCE_RADIUS);
				} else if (action.type == AiActionType::MoveTo && action.status == AiActionStatus::Succeeded) {
                    // If the slime reached the player, wait for a while.
					ai_wait(entity, WAIT_DURATION);
				} else {
                    // Else, start moving towards the player.
					ai_move_to(entity, world.player.position, knowledge.me.speed, ACCEPTANCE_RADIUS);
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

    void emplace_ai(entt::entity entity, AiType type)
    {
        _registry.emplace_or_replace<AiKnowledge>(entity);
        _registry.emplace_or_replace<AiType>(entity, type);
		_registry.emplace_or_replace<AiAction>(entity);
    }
}
