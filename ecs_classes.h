#pragma once

namespace ecs
{
	enum class Class // aka "type" or "tag"
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

	bool string_to_class(std::string_view string, Class& class_);
	std::string_view class_to_string(Class class_);
}