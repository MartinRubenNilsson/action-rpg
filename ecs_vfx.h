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
		std::shared_ptr<sf::Texture> texture;
		sf::Vector2f position;
		uint32_t frame_rows = 0;
		uint32_t frame_cols = 0;
		float frame_duration = 0.f;
		float time = 0.f;
	};

	void update_vfx(float dt);
	void draw_vfx(const sf::Vector2f& camera_min, const sf::Vector2f& camera_max);

	entt::entity create_vfx(VfxType type, const sf::Vector2f& position);
}
