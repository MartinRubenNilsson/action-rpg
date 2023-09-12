#pragma once

namespace ecs
{
	// Returns true if a behavior tree with the given name has been loaded.
	bool behavior_tree_exists(const std::string& tree_name);

	// "tree_name" is the name of the tree in the Groot2 editor, which is
	// the same as the attribute <BehaviorTree ID = "tree_name"> in the XML file.
	// Does not throw if the tree does not exist, but returns false.
	bool emplace_behavior_tree(
		entt::entity entity,
		const std::string& tree_name,
		BT::Blackboard::Ptr blackboard = BT::Blackboard::create());

	void update_behavior_trees();
}

