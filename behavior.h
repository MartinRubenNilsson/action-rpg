#pragma once

namespace behavior
{
	struct Node
	{
		enum Status
		{
			RUNNING,
			SUCCESS,
			FAILURE,
		};

		virtual ~Node() = default;
		virtual Status update(float dt) = 0;
	};

	using NodePtr = std::shared_ptr<Node>;

	struct CompositeNode : Node {
		std::vector<NodePtr> children;
	};

	struct DecoratorNode : Node {
		NodePtr child;
	};

	// COMPOSITES
	NodePtr create_selector_node(const std::vector<NodePtr>& children);
	NodePtr create_sequence_node(const std::vector<NodePtr>& children);

	// DECORATORS
	NodePtr create_succeeder_node(NodePtr child);
	NodePtr create_inverter_node(NodePtr child);
	NodePtr create_cooldown_node(float cooldown_time, NodePtr child);
	NodePtr create_time_limit_node(float time_limit, NodePtr child);

	// LEAVES
	NodePtr create_wait_node(float wait_time);
	NodePtr create_console_log_node(const std::string& message);
	NodePtr create_console_execute_node(const std::string& command_line);
}

