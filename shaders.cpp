#include "stdafx.h"
#include "shaders.h"
#include "console.h"

namespace shaders
{
	extern bool log_errors = 
#if defined(_DEBUG)
		true;
#else
		false;
#endif

	std::unordered_map<std::string, std::shared_ptr<sf::Shader>> _shaders;

	void load_shaders(const std::filesystem::path& dir)
	{
		for (const std::filesystem::directory_entry& entry
			: std::filesystem::directory_iterator(dir)) {
			if (!entry.is_regular_file()) continue;
			if (entry.path().extension() != ".frag") continue;
			std::string filename = entry.path().filename().string();
			std::string shader_name = filename.substr(0, filename.find_last_of('.'));
			std::shared_ptr<sf::Shader> shader = std::make_shared<sf::Shader>();
			if (!shader->loadFromFile(entry.path().string(), sf::Shader::Fragment)) {
				if (log_errors)
					console::log_error("Failed to load shader: " + entry.path().string());
				continue;
			}
			_shaders[shader_name] = std::move(shader);
		}
	}

	void unload_shaders() {
		_shaders.clear();
	}

	std::shared_ptr<sf::Shader> get(const std::string& name)
	{
		auto it = _shaders.find(name);
		if (it != _shaders.end()) return it->second;
		if (log_errors)
			console::log_error("Shader not found: " + name);
		return nullptr;
	}
}