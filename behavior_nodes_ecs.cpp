#include "behavior_internal.h"
#include "ecs_behaviors.h"
#include "ecs_common.h"
#include "utility_b2.h"

namespace behavior
{
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
			set_linear_velocity(*handle.get<b2Body*>(), sf::Vector2f());
			return BT::NodeStatus::SUCCESS;
		}
	};

	void _reigster_nodes_ecs(BT::BehaviorTreeFactory& factory)
	{
		factory.registerNodeType<DestroySelfNode>("DestroySelf");
		factory.registerNodeType<SetLifeSpanNode>("SetLifeSpan");
		factory.registerNodeType<StopMovingNode>("StopMoving");
	}
}