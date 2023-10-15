#include "ecs_behaviors.h"
#include "behavior.h"
#include "behavior_nodes.h"
#include "ecs_common.h"
#include "console.h"

namespace ecs
{
	extern entt::registry _registry;

	bool behavior_tree_exists(const std::string& tree_name)
	{
		for (const auto& loaded_tree : behavior::get_tree_names())
			if (loaded_tree == tree_name)
				return true;
		return false;
	}

	void _set_entity(entt::entity entity, BT::TreeNode* node)
	{
		if (auto entity_node = dynamic_cast<behavior::EntityNode*>(node))
			entity_node->handle = entt::handle(_registry, entity);
	}

	bool emplace_behavior_tree(entt::entity entity, const std::string& tree_name)
	{
		if (!_registry.valid(entity)) return false;
		try
		{
			BT::Tree tree = behavior::create_tree(tree_name); // throws std::runtime_error
			tree.applyVisitor(std::bind_front(_set_entity, entity));
			_registry.emplace<BT::Tree>(entity, std::move(tree));
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
		// Write ECS properties to blackboard properties
		for (auto [entity, tree, properties]
			: _registry.view<BT::Tree, Properties>().each())
		{
			auto blackboard = tree.rootBlackboard();
			for (const auto& [name, value] : properties.properties)
			{
				switch (value.index())
				{
				case 0: // bool
					blackboard->set(name, std::get<bool>(value));
					break;
				case 1: // int
					blackboard->set(name, std::get<int>(value));
					break;
				case 2: // float
					blackboard->set(name, std::get<float>(value));
					break;
				case 3: // std::string
					blackboard->set(name, std::get<std::string>(value));
					break;
				case 4: // entt::entity
					blackboard->set(name, (uint32_t)std::get<entt::entity>(value));
					break;
				}
			}
		}

		for (auto [entity, tree] : _registry.view<BT::Tree>().each())
			tree.tickOnce();

		// TODO
		// 
		//// Write blackboard properties to ECS properties
		//for (auto [entity, tree, properties]
		//	: _registry.view<BT::Tree, Properties>().each())
		//{
		//	auto blackboard = tree.rootBlackboard();
		//	for (const auto& key : blackboard->getKeys())
		//	{
		//		for (const auto& [name, value] : properties.properties)
		//		{
		//			if (key == name)
		//			{
		//				blackboard->set(key, value);
		//				break;
		//			}
		//		}
		//	}
		//}
	}
}
