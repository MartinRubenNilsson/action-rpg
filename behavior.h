#pragma once

namespace behavior
{
	// Nodes which need to know the entity they are operating on
	// should inherit from this struct.
	struct EntityNode { entt::entity entity = entt::null; };

	// Registers all custom behavior tree nodes.
	// This function should only be called once.
	void register_nodes();

	// Writes models of all registered nodes to a special XML file,
	// which can be loaded by the Groot2 editor to make them available for use.
	void write_node_models(const std::string& path);

	// Loads all behavior tree XML files in the assets/behaviors directory.
	// This function should only be called once.
	void load_trees();

	// "name" is the name of the tree in the Groot2 editor, which is
	// also the attribute <BehaviorTree ID = "name"> in the XML file.
	// Throws std::runtime_error if the tree is not found.
	BT::Tree create_tree(const std::string& name, BT::Blackboard::Ptr blackboard = BT::Blackboard::create());

	// Sets the entity for all nodes in the tree which inherit from EntityNode.
	void set_entity(BT::Tree& tree, entt::entity entity);
}

