#include "behavior.h"
#include "console.h"

namespace behavior
{
	// COMPOSITES

	struct SelectorNode : CompositeNode
	{
		size_t last_child_index = 0;

		Status update(float dt) override
		{
			for (size_t child_index = last_child_index; child_index < children.size(); ++child_index)
			{
				switch (children[child_index]->update(dt))
				{
				case RUNNING:
					last_child_index = child_index;
					return RUNNING;
				case SUCCESS:
					last_child_index = 0;
					return SUCCESS;
				}
			}
			last_child_index = 0;
			return FAILURE;
		}
	};

    struct SequenceNode : CompositeNode
	{
		size_t last_child_index = 0;

		Status update(float dt) override
		{
			for (size_t child_index = last_child_index; child_index < children.size(); ++child_index)
			{
				switch (children[child_index]->update(dt))
				{
				case RUNNING:
					last_child_index = child_index;
					return RUNNING;
				case FAILURE:
					last_child_index = 0;
					return FAILURE;
				}
			}
			last_child_index = 0;
			return SUCCESS;
		}
	};

	// CONSOLE NODES

	struct ConsoleLogNode : Node
	{
		std::string message;

		ConsoleLogNode(const std::string& message) : message(message) {}

		Status update(float dt) override
		{
			console::log(message);
			return SUCCESS;
		}
	};

	struct ConsoleExecuteNode : Node
	{
		std::string command_line;

		ConsoleExecuteNode(const std::string& command_line) : command_line(command_line) {}

		Status update(float dt) override
		{
			console::execute(command_line);
			return SUCCESS;
		}
	};

	// FACTORY FUNCTIONS

	CompositeNode::Ptr create_selector_node() {
		return std::make_shared<SelectorNode>();
	}

	CompositeNode::Ptr create_sequence_node() {
		return std::make_shared<SequenceNode>();
    }

	Node::Ptr create_console_log_node(const std::string& message) {
		return std::make_shared<ConsoleLogNode>(message);
	}

	Node::Ptr create_console_execute_node(const std::string& command_line) {
		return std::make_shared<ConsoleExecuteNode>(command_line);
	}
}
