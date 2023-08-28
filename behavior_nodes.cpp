#include "console.h"
#include "ecs.h"
#include "ecs_common.h"
#include "ecs_behaviors.h"

#define CHECK_INPUT(name) \
	if (!name) \
	{ \
		console::log_error(__FUNCTION__ "(): missing input \"" #name "\""); \
		return BT::NodeStatus::FAILURE; \
	}

namespace behavior
{
	struct DestroySelfNode : BT::SyncActionNode, ecs::EntityNode
	{
		DestroySelfNode(const std::string& name, const BT::NodeConfig& config)
			: BT::SyncActionNode(name, config)
		{}

		static BT::PortsList providedPorts() { return {}; }

		BT::NodeStatus tick() override
		{
			ecs::mark_for_destruction(entity);
			return BT::NodeStatus::SUCCESS;
		}
	};

	struct SetLifeSpanNode : BT::SyncActionNode, ecs::EntityNode
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
			CHECK_INPUT(life_span)
			ecs::set_life_span(entity, life_span.value());
			return BT::NodeStatus::SUCCESS;
		}
	};

	void _register_nodes(BT::BehaviorTreeFactory& factory)
	{
		factory.registerNodeType<DestroySelfNode>("DestroySelf");
		factory.registerNodeType<SetLifeSpanNode>("SetLifeSpan");
	}
}