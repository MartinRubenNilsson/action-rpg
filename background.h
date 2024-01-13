#pragma once

namespace background
{
	enum class Type
	{
		None,
		MountainDusk,
	};

	extern Type type;

	void load_assets();
	void unload_assets();
	void update(float dt);
	void render(sf::RenderTarget& target);
}

