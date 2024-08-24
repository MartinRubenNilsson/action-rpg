#pragma once

namespace ecs
{
	enum class VfxType
	{
		Explosion,
		Count,
	};

	entt::entity create_vfx(VfxType type, const Vector2f& position);
}
