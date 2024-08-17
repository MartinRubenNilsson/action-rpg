#pragma once

namespace ecs
{
	enum class Tag
	{
		None,
		Player,
		Slime,
		PushableBlock,
		Bomb,
		Arrow,
		Pickup,
		Grass,
		Portal,
		BladeTrap,
		Chest,
		Camera,
		AudioSource,
	};

	bool string_to_tag(std::string_view string, Tag& tag);
	std::string_view tag_to_string(Tag tag);
}