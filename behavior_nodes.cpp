#include "console.h"
#include "ecs_common.h"

namespace behavior
{
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
			assert(message);
			console::log(message.value());
			return BT::NodeStatus::SUCCESS;
		}
	};

	struct EntityNode
	{
		entt::entity entity;
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
			ecs::set_life_span(entity, getInput<float>("life_span").value());
			return BT::NodeStatus::SUCCESS;
		}
	};

	void _register_nodes(BT::BehaviorTreeFactory& factory)
	{
		factory.registerNodeType<ConsoleLogNode>("ConsoleLog");
		factory.registerNodeType<SetLifeSpanNode>("SetLifeSpan");
	}
}