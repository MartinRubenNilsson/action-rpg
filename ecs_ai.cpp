#include "stdafx.h"
#include "ecs_ai.h"
#include "ecs_ai_knowledge.h"
#include "ecs_ai_action.h"

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
                action.type = AiActionType::MoveTo;
                action.target_position = world.player.position;
                action.speed = knowledge.me.speed;
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
