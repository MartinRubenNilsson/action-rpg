#include "behavior.h"
#include <behaviortree_cpp/xml_parsing.h> // writeTreeNodesModelXML()
#include "console.h"

namespace behavior
{
	struct ConsoleLogNode : BT::SyncActionNode
	{
		ConsoleLogNode(const std::string& name, const BT::NodeConfig& config)
			: BT::SyncActionNode(name, config)
		{}

		static BT::PortsList providedPorts() {
			//return { BT::InputPort<std::string>("message") };
			return {};
		}

		BT::NodeStatus tick() override
		{
			//auto message = getInput<std::string>("message");
			//if (!message)
			//	throw BT::RuntimeError("missing required input [message]: ", message.error());
			//console::log(message.value());
			static int i = 0;
			console::log("Hello, world: " + std::to_string(i++));
			return BT::NodeStatus::SUCCESS;
		}
	};

	BT::BehaviorTreeFactory _factory;

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
		_factory.registerNodeType<ConsoleLogNode>("ConsoleLog");
		_write_tree_node_models();
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

	BT::Tree create_tree(const std::string& tree_id)
	{
		return _factory.createTree(tree_id);
	}
}
