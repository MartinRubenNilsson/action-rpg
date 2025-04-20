#pragma once

namespace ecs {
	enum class ChestType {
		Normal,
		Bomb, // trap chest; explodes when opened
	};

	struct Chest {
		ChestType type = ChestType::Normal;
		bool opened = false;
	};

	Chest& emplace_chest(entt::entity entity, const Chest& chest);
	Chest* get_chest(entt::entity entity);

	void open_chest(entt::entity entity, bool ignore_contents = false);
	void interact_with_chest(entt::entity entity);
}