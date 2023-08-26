#pragma once

namespace behavior
{
	// Registers all custom behavior tree nodes.
	// This function should only be called once.
	void register_nodes();

	// Writes so-called "models" of all registered nodes to a special XML file,
	// which can be loaded by the Groot2 editor to make them available for use.
	void write_node_models(const std::string& path);

	// Loads all behavior tree XML files in the assets/behaviors directory.
	// This function should only be called once.
	void load_trees();

	// "name" is the name of the tree in the Groot2 editor, which is
	// the same as the attribute <BehaviorTree ID = "name"> in the XML file.
	// Throws std::runtime_error if the tree is not found.
	BT::Tree create_tree(const std::string& name,
		BT::Blackboard::Ptr blackboard = BT::Blackboard::create());
}

