#pragma once

namespace fonts
{
	extern bool log_errors;

	void load_assets(const std::filesystem::path& dir);
	void unload_assets();

	std::shared_ptr<sf::Font> get(const std::string& name);
}

