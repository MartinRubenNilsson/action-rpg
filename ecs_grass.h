#pragma once

namespace ecs {
	struct Grass {
		int dummy = 0;
	};

	struct GrassUniformBlock {
		Vector2f position;
		Vector2f tex_min;
		Vector2f tex_max;
	};

	Grass& emplace_grass(entt::entity entity);

	GrassUniformBlock& emplace_grass_uniform_block(entt::entity entity);

	struct Damage;

	bool apply_damage_to_grass(entt::entity entity, const Damage& damage);
}