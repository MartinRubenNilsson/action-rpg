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

	struct ShaderAsset
	{
		std::filesystem::path path;
		std::shared_ptr<sf::Shader> shader;
	};

	std::vector<ShaderAsset> _assets;

	void _copy_shaders_to_assets()
	{
#ifdef _DEBUG
		// HACK: We should be using a post-build event to copy the shaders,
		// but then it doesn't run when only debugging and not recompiling,
		// which is annoying when you've changed a shader but not the code,
		// because then the new shader doesn't get copied.
		system("copy /Y ..\\*.frag .\\assets\\shaders\\");
#endif
	}

	void load_assets(const std::filesystem::path& dir)
	{
		if (!sf::Shader::isAvailable()) {
			if (log_errors)
				console::log_error("Shaders are not supported on this system");
			return;
		}

		_copy_shaders_to_assets();

		for (const std::filesystem::directory_entry& entry
			: std::filesystem::directory_iterator(dir)) {
			if (!entry.is_regular_file()) continue;
			if (entry.path().extension() != ".frag") continue;
			std::shared_ptr<sf::Shader> shader = std::make_shared<sf::Shader>();
			if (!shader->loadFromFile(entry.path().string(), sf::Shader::Fragment)) {
				if (log_errors)
					console::log_error("Failed to load shader: " + entry.path().string());
				continue;
			}
			ShaderAsset& asset = _assets.emplace_back();
			asset.path = entry.path().lexically_normal();
			asset.shader = shader;
		}
	}

	void reload_assets()
	{
		_copy_shaders_to_assets();
		for (ShaderAsset& asset : _assets) {
			if (!asset.shader->loadFromFile(asset.path.string(), sf::Shader::Fragment)) {
				if (log_errors)
					console::log_error("Failed to reload shader: " + asset.path.string());
			}
		}
	}

	void unload_assets() {
		_assets.clear();
	}

	std::shared_ptr<sf::Shader> get(const std::string& name)
	{
		for (const ShaderAsset& asset : _assets)
			if (asset.path.stem().string() == name)
				return asset.shader;
		if (log_errors)
			console::log_error("Failed to find shader: " + name);
		return nullptr;
	}
}