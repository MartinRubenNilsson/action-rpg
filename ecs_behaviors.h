#pragma once

namespace ecs
{
	bool behavior_tree_exists(const std::string& tree_name);

	// "tree_name" is the name of the tree in the Groot2 editor, which is
	// the same as the attribute <BehaviorTree ID = "tree_name"> in the XML file.
	// Returns false if the tree is not found.
	bool emplace_behavior_tree(entt::entity entity, const std::string& tree_name);

	void update_behavior_trees();
}

