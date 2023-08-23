#pragma once

namespace behavior
{
	void initialize();

	// Loads all XML behavior trees in the assets/behaviors directory.
	// This function should only be called once.
	void load_trees();

	// tree_id is the attribute <BehaviorTree ID = "tree_id"> in the XML file.
	// Throws std::runtime_error if the tree is not found.
	BT::Tree create_tree(const std::string& tree_id);
}

