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
		uint32_t frame_rows = 1;
		uint32_t frame_cols = 1;
		float frame_duration = 0.0f;
		float elapsed_time = 0.0f;
		sf::Sprite sprite;
	};

	void update_vfx(float dt);

	entt::entity create_vfx(VfxType type, const sf::Vector2f& position);
}
