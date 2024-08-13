#pragma once

namespace ecs
{
	enum class VfxType
	{
		None,
		Explosion,
		Count,
	};

	struct Vfx
	{
		VfxType type = VfxType::None;
		Handle<graphics::Texture> texture;
		Vector2f position;
		unsigned int frame_rows = 0;
		unsigned int frame_cols = 0;
		float frame_duration = 0.f;
		float time = 0.f;
	};

	void update_vfx(float dt);
	void add_vfx_sprites_for_drawing(const Vector2f& camera_min, const Vector2f& camera_max);

	entt::entity create_vfx(VfxType type, const Vector2f& position);
}
