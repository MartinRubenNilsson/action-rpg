#include "behavior_nodes.h"
#include "console.h"
#include "physics_helpers.h"
#include "ecs_common.h"
#include "ecs_player.h"

namespace behavior
{
	// CONSOLE

	struct ConsoleLogNode : BT::SyncActionNode
	{
		ConsoleLogNode(const std::string& name, const BT::NodeConfig& config)
			: BT::SyncActionNode(name, config)
		{}

		static BT::PortsList providedPorts() {
			return { BT::InputPort<std::string>("message", "", "The message to log") };
		}

		BT::NodeStatus tick() override
		{
			auto message = getInput<std::string>("message");
			if (!message) return BT::NodeStatus::FAILURE;
			console::log(message.value());
			return BT::NodeStatus::SUCCESS;
		}
	};

	struct ConsoleLogErrorNode : BT::SyncActionNode
	{
		ConsoleLogErrorNode(const std::string& name, const BT::NodeConfig& config)
			: BT::SyncActionNode(name, config)
		{}

		static BT::PortsList providedPorts() {
			return { BT::InputPort<std::string>("message", "", "The message to log") };
		}

		BT::NodeStatus tick() override
		{
			auto message = getInput<std::string>("message");
			if (!message) return BT::NodeStatus::FAILURE;
			console::log_error(message.value());
			return BT::NodeStatus::SUCCESS;
		}
	};

	struct ConsoleExecuteNode : BT::SyncActionNode
	{
		ConsoleExecuteNode(const std::string& name, const BT::NodeConfig& config)
			: BT::SyncActionNode(name, config)
		{}

		static BT::PortsList providedPorts() {
			return { BT::InputPort<std::string>("command", "", "The command line to execute") };
		}

		BT::NodeStatus tick() override
		{
			auto command = getInput<std::string>("command");
			if (!command) return BT::NodeStatus::FAILURE;
			console::execute(command.value());
			return BT::NodeStatus::SUCCESS;
		}
	};

	// ECS

	struct DestroySelfNode : BT::SyncActionNode, EntityNode
	{
		DestroySelfNode(const std::string& name, const BT::NodeConfig& config)
			: BT::SyncActionNode(name, config)
		{}

		static BT::PortsList providedPorts() { return {}; }

		BT::NodeStatus tick() override
		{
			ecs::mark_for_destruction(handle.entity());
			return BT::NodeStatus::SUCCESS;
		}
	};

	struct SetLifeSpanNode : BT::SyncActionNode, EntityNode
	{
		SetLifeSpanNode(const std::string& name, const BT::NodeConfig& config)
			: BT::SyncActionNode(name, config)
		{}

		static BT::PortsList providedPorts() {
			return { BT::InputPort<float>("life_span", 0.f, "The remaining life span") };
		}

		BT::NodeStatus tick() override
		{
			auto life_span = getInput<float>("life_span");
			if (!life_span) return BT::NodeStatus::FAILURE;
			ecs::set_life_span(handle.entity(), life_span.value());
			return BT::NodeStatus::SUCCESS;
		}
	};

	struct StopMovingNode : BT::SyncActionNode, EntityNode
	{
		StopMovingNode(const std::string& name, const BT::NodeConfig& config)
			: BT::SyncActionNode(name, config)
		{}

		static BT::PortsList providedPorts() {
			return {};
		}

		BT::NodeStatus tick() override
		{
			if (!handle.all_of<b2Body*>()) return BT::NodeStatus::FAILURE;
			set_linear_velocity(handle.get<b2Body*>(), sf::Vector2f());
			return BT::NodeStatus::SUCCESS;
		}
	};

	// PLAYER

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
			auto body = handle.get<b2Body*>();

			auto radius = getInput<float>("radius");
			if (!radius) return BT::NodeStatus::FAILURE;

			sf::Vector2f center = get_world_center(body);
			sf::Vector2f player_center = ecs::get_player_center();
			float distance = length(player_center - center);

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
			auto body = handle.get<b2Body*>();

			auto speed = getInput<float>("speed");
			if (!speed) return BT::NodeStatus::FAILURE;

			sf::Vector2f position = get_world_center(body);
			sf::Vector2f player_position = ecs::get_player_center();
			sf::Vector2f direction = normalize(player_position - position);
			sf::Vector2f velocity = direction * speed.value();
			set_linear_velocity(body, velocity);

			return BT::NodeStatus::SUCCESS;
		}
	};

	void _register_nodes(BT::BehaviorTreeFactory& factory)
	{
		// CONSOLE
		factory.registerNodeType<ConsoleLogNode>("ConsoleLog");
		factory.registerNodeType<ConsoleLogErrorNode>("ConsoleLogError");
		factory.registerNodeType<ConsoleExecuteNode>("ConsoleExecute");

		// ECS
		factory.registerNodeType<DestroySelfNode>("DestroySelf");
		factory.registerNodeType<SetLifeSpanNode>("SetLifeSpan");
		factory.registerNodeType<StopMovingNode>("StopMoving");

		// PLAYER
		factory.registerNodeType<IsPlayerInRange>("IsPlayerInRange");
		factory.registerNodeType<ApproachPlayer>("ApproachPlayer");
	}
}