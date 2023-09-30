#include "behavior.h"
#include "behavior_internal.h"
#include <behaviortree_cpp/xml_parsing.h> // writeTreeNodesModelXML()
#include "console.h"

namespace behavior
{
	BT::BehaviorTreeFactory _factory;

	void register_nodes()
	{
		_register_nodes_console(_factory);
		_reigster_nodes_ecs(_factory);
		_register_nodes_player(_factory);
	}

	void write_node_models_file(const std::string& path)
	{
		std::string models_xml = BT::writeTreeNodesModelXML(_factory);
		models_xml.insert(5, " BTCPP_format = \"4\""); // add missing format version so Groot2 doesn't complain
		std::ofstream file(path);
		file << models_xml;
	}

	void load_trees()
	{
		for (const auto& entry : std::filesystem::directory_iterator("assets/behaviors"))
		{
			if (entry.path().extension() != ".xml")
				continue;

			try
			{
				_factory.registerBehaviorTreeFromFile(entry.path().string());
			}
			catch (const BT::RuntimeError& error)
			{
				console::log_error("Failed to load behavior tree from file: " + entry.path().generic_string());
				console::log_error(error.what());
			}
		}
	}

	std::vector<std::string> get_tree_names() {
		return _factory.registeredBehaviorTrees();
	}

	BT::Tree create_tree(const std::string& name) {
		return _factory.createTree(name);
	}
}
