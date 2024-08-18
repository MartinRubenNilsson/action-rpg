#pragma once

namespace ecs
{
	struct Grass
	{
		int dummy = 0;
	};

	struct GrassUniformBlock
	{
		//TODO
	};

	Grass& emplace_grass(entt::entity entity);
}