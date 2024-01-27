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


        for (auto [entity, knowledge, ai_type, action] : _registry.view<const AiKnowledge, const AiType, AiAction>().each()) {
            switch (ai_type) {
            case AiType::Slime:
            {
                if (!_registry.valid(world.player.entity)) break;

                //const float distanceToPlayer = length(world.player.position - knowledge.me.position);
                //const float tooCloseDistance = 16.f; // Example value, adjust as needed.
                const float waitTime = 3.0f; // The time to wait when too close.
                const float acceptanceRadius = 16.f; // The distance to the target position at which the action is considered successful.

                // If the current action is to wait and it is still running, do nothing.
                if (action.type == AiActionType::Wait && action.status == AiActionStatus::Running) {
                    break;
                }

                // If the Slime is not too close to the player, make it move to the player's position.
                if (action.status == AiActionStatus::Succeeded && action.type == AiActionType::Wait) {
                    action.type = AiActionType::MoveToPosition;
                    action.target_position = world.player.position;
                    action.speed = knowledge.me.speed;
                    action.acceptance_radius = acceptanceRadius;
                    _registry.replace<AiAction>(entity, action); // Update the action in the registry.
                    break;
                }

                // If the Slime is too close to the player, make it wait.
                if ((action.status == AiActionStatus::Succeeded && (action.type == AiActionType::MoveToPosition || action.type == AiActionType::None))) {
                    console::log(std::to_string(static_cast<int>(action.status)));
                    console::log(std::to_string(static_cast<int>(action.type)));
                    ai_wait(entity, waitTime);
                    break;
                }
                break;

            }

            // ... handle other AI types ...
            }
        }

        //for (auto [entity, knowledge, ai_type, action] : _registry.view<const AiKnowledge, const AiType, AiAction>().each()) {
        //    switch (ai_type) {
        //    case AiType::Slime:
        //    {
        //        if (!_registry.valid(world.player.entity)) break;
        //        action.type = AiActionType::MoveToPosition;
        //        action.target_position = world.player.position;
        //        action.speed = knowledge.me.speed;
        //        break;
        //    }
        //    }
        //}
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
