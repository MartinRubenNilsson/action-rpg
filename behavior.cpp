#include "behavior.h"
#include <behaviortree_cpp/xml_parsing.h> // writeTreeNodesModelXML()

namespace behavior
{
	BT::BehaviorTreeFactory _factory;

	BT::NodeStatus do_stuff(BT::TreeNode& node)
	{
		std::cout << "Doing stuff..." << std::endl;
		return BT::NodeStatus::SUCCESS;
	}

	// Writes "models" of all registered tree nodes to a file,
	// which can be imported into Groot2 to make it aware of the nodes.
	void _write_tree_node_models()
	{
		std::string models_xml = BT::writeTreeNodesModelXML(_factory);
		models_xml.insert(5, " BTCPP_format = \"4\""); // add format version so Groot2 doesn't complain
		std::ofstream file("assets/behaviors/models/models.xml");
		file << models_xml;
	}

	void initialize()
	{
		_factory.registerSimpleCondition("do_stuff", do_stuff);
		_factory.registerSimpleCondition("do_stuff2", do_stuff);
		_write_tree_node_models();
	}

	void load_trees()
	{
		for (const auto& entry : std::filesystem::directory_iterator("assets/behaviors"))
		{
			if (entry.path().extension() == ".xml")
				_factory.registerBehaviorTreeFromFile(entry.path().string());
		}
	}

	BT::Tree create_tree(const std::string& tree_id)
	{
		return _factory.createTree(tree_id);
	}
}
