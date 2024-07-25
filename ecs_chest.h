#pragma once

namespace ecs
{
	struct Chest
	{
		bool opened = false;
	};

	Chest& emplace_chest(entt::entity entity, const Chest& chest);
	Chest* get_chest(entt::entity entity);

	void open_chest(entt::entity entity);
}