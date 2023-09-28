#pragma once
#include <behaviortree_cpp/bt_factory.h>
#include <behaviortree_cpp/basic_types.h>
#include "console.h"
#include <SFML/System/Vector2.hpp>

namespace behavior
{
    // Behavior tree nodes which need to know the entity they are operating on
    // should inherit from this struct. It should NOT be used as a component.
    struct EntityNode { entt::handle handle; };

    void _register_nodes_console(BT::BehaviorTreeFactory& factory);
    void _reigster_nodes_ecs(BT::BehaviorTreeFactory& factory);
    void _register_nodes_player(BT::BehaviorTreeFactory& factory);
}

namespace BT
{
    template <> inline sf::Vector2f convertFromString(StringView str)
    {
        auto parts = splitString(str, ';');
        if (parts.size() != 2)
        {
            throw RuntimeError("invalid input");
        }
        else
        {
            sf::Vector2f output;
            output.x = convertFromString<float>(parts[0]);
            output.y = convertFromString<float>(parts[1]);
            return output;
        }
    }
} // end namespace BT