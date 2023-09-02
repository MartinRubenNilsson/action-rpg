#include "behavior_internal.h"
#include "ecs_player.h"
#include "utility_b2.h"

namespace behavior
{
	struct IsPlayerInRange : BT::ConditionNode, EntityNode
	{
		IsPlayerInRange(const std::string& name, const BT::NodeConfig& config)
			: BT::ConditionNode(name, config)
		{}

		static BT::PortsList providedPorts() {
			return { BT::InputPort<float>("radius", 0.f, "The radius of the range") };
		}

		BT::NodeStatus tick() override
		{
			if (!ecs::player_exists()) return BT::NodeStatus::FAILURE;
			if (!handle.all_of<b2Body*>()) return BT::NodeStatus::FAILURE;
			b2Body& body = *handle.get<b2Body*>();

			auto radius = getInput<float>("radius");
			if (!radius) return BT::NodeStatus::FAILURE;

			sf::Vector2f position = b2::get_position(body);
			sf::Vector2f player_position = ecs::get_player_position();
			float distance = length(player_position - position);

			return (distance <= radius.value()) ?
				BT::NodeStatus::SUCCESS :
				BT::NodeStatus::FAILURE;
		}
	};

	struct ApproachPlayer : BT::SyncActionNode, EntityNode
	{
		ApproachPlayer(const std::string& name, const BT::NodeConfig& config)
			: BT::SyncActionNode(name, config)
		{}

		static BT::PortsList providedPorts() {
			return { BT::InputPort<float>("speed", 0.f, "The movement speed to use") };
		}

		BT::NodeStatus tick() override
		{
			if (!ecs::player_exists()) return BT::NodeStatus::FAILURE;
			if (!handle.all_of<b2Body*>()) return BT::NodeStatus::FAILURE;
			b2Body& body = *handle.get<b2Body*>();

			auto speed = getInput<float>("speed");
			if (!speed) return BT::NodeStatus::FAILURE;

			sf::Vector2f position = b2::get_position(body);
			sf::Vector2f player_position = ecs::get_player_position();
			sf::Vector2f direction = normalize(player_position - position);
			sf::Vector2f velocity = direction * speed.value();
			b2::set_linear_velocity(body, velocity);

			return BT::NodeStatus::SUCCESS;
		}
	};

	void _register_nodes_player(BT::BehaviorTreeFactory& factory)
	{
		factory.registerNodeType<IsPlayerInRange>("IsPlayerInRange");
		factory.registerNodeType<ApproachPlayer>("ApproachPlayer");
	}
}