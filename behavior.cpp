#include "stdafx.h"
#include "behavior.h"
#include "console.h"

namespace behavior
{
	// COMPOSITES

	struct CompositeNode : Node
	{
		std::vector<NodePtr> children;
	};

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

	// DECORATORS

	struct DecoratorNode : Node
	{
		NodePtr child;
	};

	struct SucceederNode : DecoratorNode
	{
		Status update(float dt) override
		{
			child->update(dt);
			return SUCCESS;
		}
	};

	struct InverterNode : DecoratorNode
	{
		Status update(float dt) override
		{
			switch (child->update(dt))
			{
			case SUCCESS:
				return FAILURE;
			case FAILURE:
				return SUCCESS;
			default:
				return RUNNING;
			}
		}
	};

	struct CooldownNode : DecoratorNode
	{
		float cooldown_time = 0.f;
		float time_left = 0.f;

		Status update(float dt) override
		{
			if (time_left > 0.f)
			{
				time_left -= dt;
				return FAILURE;
			}
			Status child_status = child->update(dt);
			if (child_status != RUNNING)
				time_left = cooldown_time;
			return child_status;
		}
	};

	// LEAVES

	struct WaitNode : Node
	{
		float wait_time = 0.f;
		float time_elapsed = 0.f;

		Status update(float dt) override
		{
			time_elapsed += dt;
			if (time_elapsed >= wait_time)
			{
				time_elapsed = 0.0f;
				return SUCCESS;
			}
			return RUNNING;
		}
	};

	struct ConsoleLogNode : Node
	{
		std::string message;

		Status update(float dt) override
		{
			console::log(message);
			return SUCCESS;
		}
	};

	struct ConsoleExecuteNode : Node
	{
		std::string command_line;

		Status update(float dt) override
		{
			console::execute(command_line);
			return SUCCESS;
		}
	};

	// FACTORY FUNCTIONS

	NodePtr create_selector_node(const std::vector<NodePtr>& children)
	{
		auto node = std::make_shared<SelectorNode>();
		node->children = children;
		return node;
	}

	NodePtr create_sequence_node(const std::vector<NodePtr>& children)
	{
		auto node = std::make_shared<SequenceNode>();
		node->children = children;
		return node;
    }

	NodePtr create_succeeder_node(NodePtr child)
	{
		auto node = std::make_shared<SucceederNode>();
		node->child = child;
		return node;
	}

	NodePtr create_inverter_node(NodePtr child)
	{
		auto node = std::make_shared<InverterNode>();
		node->child = child;
		return node;
	}

	NodePtr create_cooldown_node(float cooldown_time, NodePtr child)
	{
		auto node = std::make_shared<CooldownNode>();
		node->cooldown_time = cooldown_time;
		node->child = child;
		return node;
	}

	NodePtr create_wait_node(float wait_time)
	{
		auto node = std::make_shared<WaitNode>();
		node->wait_time = wait_time;
		return node;
	}

	NodePtr create_console_log_node(const std::string& message)
	{
		auto node = std::make_shared<ConsoleLogNode>();
		node->message = message;
		return node;
	}

	NodePtr create_console_execute_node(const std::string& command_line)
	{
		auto node = std::make_shared<ConsoleExecuteNode>();
		node->command_line = command_line;
		return node;
	}
}
