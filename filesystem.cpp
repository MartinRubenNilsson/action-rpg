#include "stdafx.h"
#include "filesystem.h"
#include <filesystem>

namespace filesystem
{
	std::vector<std::string> _file_paths;
	std::vector<FileFormat> _file_formats;

	FileFormat _get_file_format_from_extension(std::string_view path)
	{
		if (path.ends_with(".txt"))  return FileFormat::Text;
		if (path.ends_with(".png"))  return FileFormat::PNG;
		if (path.ends_with(".tmx"))  return FileFormat::TiledMap;
		if (path.ends_with(".tsx"))  return FileFormat::TiledTileset;
		if (path.ends_with(".tx"))   return FileFormat::TiledTemplate;
		if (path.ends_with(".ttf"))  return FileFormat::TrueTypeFont;
		if (path.ends_with(".vert")) return FileFormat::VertexShader;
		if (path.ends_with(".frag")) return FileFormat::FragmentShader;
		if (path.ends_with(".rml"))  return FileFormat::RmlUiDocument;
		if (path.ends_with(".rcss")) return FileFormat::RmlUiStyleSheet;
		if (path.ends_with(".bank")) return FileFormat::FmodStudioBank;
		if (path.ends_with(".dll"))  return FileFormat::DynamicLinkLibrary;
		if (path.ends_with(".exe"))  return FileFormat::Executable;
		return FileFormat::Unknown;
	}

	void initialize()
	{
		_file_paths.clear();
		for (const auto& entry : std::filesystem::recursive_directory_iterator(".")) {
			if (entry.is_regular_file()) {
				_file_paths.push_back(entry.path().lexically_normal().string());
			}
		}
		std::sort(_file_paths.begin(), _file_paths.end());

		_file_formats.clear();
		_file_formats.reserve(_file_paths.size());
		for (const std::string& path : _file_paths) {
			_file_formats.push_back(_get_file_format_from_extension(path));
		}

		return;
	}

	//std::span<const std::string> get_paths_in_directory(const std::string& directory)
	//{
	//
	//}

	std::string get_normalized_path(const std::string& path)
	{
		return std::filesystem::path(path).lexically_normal().string();
	}

	std::string get_parent_path(const std::string& path)
	{
		return std::filesystem::path(path).parent_path().string();
	}

	std::string get_filename(const std::string& path)
	{
		return std::filesystem::path(path).filename().string();
	}

	std::string get_stem(const std::string& path)
	{
		return std::filesystem::path(path).stem().string();
	}

	std::string get_extension(const std::string& path)
	{
		return std::filesystem::path(path).extension().string();
	}
}