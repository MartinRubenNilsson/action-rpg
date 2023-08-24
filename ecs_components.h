#pragma once

namespace ecs
{
	struct LifeSpan { float value = 0.f; };

	void set_life_span(entt::entity entity, float life_span);
}