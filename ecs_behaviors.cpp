#include "ecs_behaviors.h"
#include "behavior.h"
#include "behavior_internal.h"
#include "console.h"

namespace ecs
{
	extern entt::registry _registry;

	bool behavior_tree_exists(const std::string& tree_name)
	{
		for (const auto& loaded_tree : behavior::get_loaded_trees())
			if (loaded_tree == tree_name)
				return true;
		return false;
	}

	void _set_entity(entt::entity entity, BT::TreeNode* node)
	{
		if (auto entity_node = dynamic_cast<behavior::EntityNode*>(node))
			entity_node->handle = entt::handle(_registry, entity);
	}

	bool set_behavior_tree(
		entt::entity entity,
		const std::string& tree_name,
		BT::Blackboard::Ptr blackboard)
	{
		if (!_registry.valid(entity)) return false;
		try
		{
			BT::Tree tree = behavior::create_tree(tree_name, blackboard); // throws std::runtime_error
			tree.applyVisitor(std::bind_front(_set_entity, entity));
			_registry.emplace_or_replace<BT::Tree>(entity, std::move(tree));
			return true;
		}
		catch (const std::runtime_error& error)
		{
			console::log_error("Failed to create behavior tree: " + tree_name);
			console::log_error(error.what());
		}
		return false;
	}

	void update_behavior_trees()
	{
		for (auto [entity, tree] : _registry.view<BT::Tree>().each())
			tree.tickOnce();
	}
}
