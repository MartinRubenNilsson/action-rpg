#include "behavior_nodes.h"
#include "console.h"

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
			CHECK_INPUT(message)
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
			CHECK_INPUT(message)
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
			CHECK_INPUT(command)
			console::execute(command.value());
			return BT::NodeStatus::SUCCESS;
		}
	};

	void _register_console_nodes(BT::BehaviorTreeFactory& factory)
	{
		factory.registerNodeType<ConsoleLogNode>("ConsoleLog");
		factory.registerNodeType<ConsoleLogErrorNode>("ConsoleLogError");
		factory.registerNodeType<ConsoleExecuteNode>("ConsoleExecute");
	}
}