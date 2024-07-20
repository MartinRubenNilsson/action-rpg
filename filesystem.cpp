#include "stdafx.h"
#include "filesystem.h"
#include <filesystem>

namespace filesystem
{
	std::vector<File> _files;

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
		return FileFormat::Unknown;
	}

	void initialize()
	{
		_files.clear();
		for (const auto& entry : std::filesystem::recursive_directory_iterator(".")) {
			if (!entry.is_regular_file()) continue;
			File file{};
			file.path = entry.path().lexically_normal().string();
			file.format = _get_file_format_from_extension(file.path);
			if (file.format == FileFormat::Unknown) continue;
			_files.push_back(std::move(file));
		}
		std::ranges::sort(_files, {}, &File::path);
	}

	size_t get_file_count()
	{
		return _files.size();
	}

	std::span<const File> get_files()
	{
		return _files;
	}

	std::span<const File> get_files_in_directory(const std::string& directory_path)
	{
		const std::string normalized_directory_path = get_normalized_path(directory_path);
		const size_t prefix_size = normalized_directory_path.size();
		auto [first, last] = std::ranges::equal_range(_files, File{ .path = normalized_directory_path },
			[prefix_size](const File& left, const File& right) {
			return strncmp(left.path.c_str(), right.path.c_str(), prefix_size) < 0;
		});
		return std::span<const File>{first, last};
	}

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