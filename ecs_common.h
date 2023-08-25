#pragma once

namespace ecs
{
	struct Name { std::string value; };
	struct Type { std::string value; }; // Same as "class" in the Tiled editor.
	struct LifeSpan { float value = 0.f; };

	void set_life_span(entt::entity entity, float life_span);
}