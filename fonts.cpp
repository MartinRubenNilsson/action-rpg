#include "stdafx.h"
#include "fonts.h"
#include "console.h"

namespace fonts
{
	extern bool log_errors =
#if defined(_DEBUG)
		true;
#else
		false;
#endif

	std::unordered_map<std::string, std::shared_ptr<sf::Font>> _fonts;

	void load_assets(const std::filesystem::path& dir)
	{
		for (const std::filesystem::directory_entry& entry
			: std::filesystem::directory_iterator(dir)) {
			if (!entry.is_regular_file()) continue;
			if (entry.path().extension() != ".ttf") continue;
			std::string filename = entry.path().filename().string();
			std::string font_name = filename.substr(0, filename.find_last_of('.'));
			std::shared_ptr<sf::Font> font = std::make_shared<sf::Font>();
			if (!font->loadFromFile(entry.path().string())) {
				if (log_errors)
					console::log_error("Failed to load font: " + entry.path().string());
				continue;
			}
			_fonts[font_name] = std::move(font);
		}
	}

	void unload_assets() {
		_fonts.clear();
	}

	std::shared_ptr<sf::Font> get(const std::string& name)
	{
		auto it = _fonts.find(name);
		if (it != _fonts.end()) return it->second;
		if (log_errors)
			console::log_error("Font not found: " + name);
		return nullptr;
	}
}

