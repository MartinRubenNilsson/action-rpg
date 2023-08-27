#pragma once

namespace ecs
{
	// Behavior tree nodes which need to know the entity they are operating on
	// should inherit from this struct. It should NOT be used as a component.
	struct EntityNode { entt::entity entity = entt::null; };

	// Returns true if a behavior tree with the given name has been loaded.
	bool behavior_tree_exists(const std::string& tree_name);

	// "tree_name" is the name of the tree in the Groot2 editor, which is
	// the same as the attribute <BehaviorTree ID = "tree_name"> in the XML file.
	// Does not throw if the tree does not exist, but returns false.
	bool set_behavior_tree(
		entt::entity entity,
		const std::string& tree_name,
		BT::Blackboard::Ptr blackboard = BT::Blackboard::create());

	void update_behavior_trees();
}

