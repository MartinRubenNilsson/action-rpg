#pragma once

namespace behavior
{
	struct Node
	{
		using Ptr = std::shared_ptr<Node>;

		enum Status
		{
			RUNNING,
			SUCCESS,
			FAILURE,
		};

		virtual ~Node() = default;
		virtual Status update(float dt) = 0;
	};

	struct CompositeNode : public Node
	{
		std::vector<Node::Ptr> children;
	};

	struct DecoratorNode : public Node
	{
		Node::Ptr child;
	};

	// COMPOSITES
	Node::Ptr create_selector_node(const std::vector<Node::Ptr>& children);
	Node::Ptr create_sequence_node(const std::vector<Node::Ptr>& children);

	// DECORATORS
	Node::Ptr create_succeeder_node(Node::Ptr child);
	Node::Ptr create_inverter_node(Node::Ptr child);
	Node::Ptr create_cooldown_node(Node::Ptr child, float cooldown_time);

	// LEAVES
	Node::Ptr create_wait_node(float wait_time);
	Node::Ptr create_console_log_node(const std::string& message);
	Node::Ptr create_console_execute_node(const std::string& command_line);
}

