#include "stdafx.h"
#include "shaders.h"
#include "console.h"

namespace shaders
{
	extern bool log_errors = 
#ifdef _DEBUG
		true;
#else
		false;
#endif

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
		_copy_shaders_to_assets();
	}

	void reload_assets()
	{
		_copy_shaders_to_assets();
	}

	void unload_assets() {
	}
}