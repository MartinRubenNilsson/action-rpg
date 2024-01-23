#include "stdafx.h"
#include "ecs_ai_behavior.h"
#include "ecs_ai_knowledge.h"
#include "ecs_ai_action.h"

namespace ecs
{
    extern entt::registry _registry;

    void update_ai_behaviors(float dt)
    {
        const AiWorld& world = get_ai_world();

        // IMPORTANT: knowledge must be const! don't modify it here!
        for (auto [entity, knowledge, behavior, action] :
			_registry.view<const ecs::AiKnowledge, ecs::AiBehavior, ecs::AiAction>().each())
		{
            switch (behavior.type) {
            case AiBehaviorType::Slime:
            {
                if (!_registry.valid(world.player.entity)) break;
                action.type = AiActionType::MoveToPosition;
                action.target_position = world.player.current_position;
                action.speed = knowledge.me.speed;
                break;
            }
            }
		}
    }
}
