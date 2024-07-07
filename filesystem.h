#pragma once

namespace filesystem
{
	//These are not used for anything rn
	enum class FileType
	{
		Unknown,
		VertexShader,
		FragmentShader,
		FmodStudioBank,
		TiledMap,
		TiledTileset,
		TiledTemplate,
		TrueTypeFont,
	};

	std::string get_normalized_path(const std::string& path);
	std::string get_parent_path(const std::string& path);
	std::string get_filename(const std::string& path);
	std::string get_stem(const std::string& path);
	std::string get_extension(const std::string& path);
}