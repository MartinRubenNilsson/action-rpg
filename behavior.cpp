#include "behavior.h"

namespace behavior
{
	BT::BehaviorTreeFactory _factory;

	void load_behavior_trees()
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
