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

		using Ptr = std::shared_ptr<Node>;

		virtual ~Node() = default;
		virtual Status update(float dt) = 0;
	};

	struct CompositeNode : public Node
	{
		using Ptr = std::shared_ptr<CompositeNode>;

		std::vector<Node::Ptr> children;
	};

	CompositeNode::Ptr create_selector_node();
	CompositeNode::Ptr create_sequence_node();

	Node::Ptr create_console_log_node(const std::string& message);
	Node::Ptr create_console_execute_node(const std::string& command_line);
}

