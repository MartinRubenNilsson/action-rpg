#include "stdafx.h"
#include "filesystem.h"
#include <filesystem>
#include <fstream>

namespace filesystem
{
	std::vector<File> _files;

	FileFormat _get_file_format_from_extension(std::string_view path)
	{
		if (path.ends_with(".txt"))  return FileFormat::Text;
		if (path.ends_with(".png"))  return FileFormat::PngImage;
		if (path.ends_with(".ktx2")) return FileFormat::KhronosTexture;
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
		return { first, last };
	}

	bool file_exists(const std::string& path)
	{
		return std::ranges::binary_search(_files, path, {}, &File::path);
	}

	bool read_text(const std::string& path, std::string& text)
	{
		std::ifstream file(path, std::ios::ate);
		if (!file) return false;
		text.resize(file.tellg());
		file.seekg(0);
		file.read(text.data(), text.size());
		while (!text.empty() && text.back() == '\0') {
			text.pop_back();
		}
		return true;
	}

	bool write_text(const std::string& path, const std::string& text)
	{
		std::ofstream file(path);
		if (!file) return false;
		file.write(text.data(), text.size());
		return true;
	}

	bool read_binary(const std::string& path, std::vector<unsigned char>& data)
	{
		std::ifstream file(path, std::ios::ate | std::ios::binary);
		if (!file) return false;
		data.resize(file.tellg());
		file.seekg(0);
		file.read(reinterpret_cast<char*>(data.data()), data.size());
		return true;
	}

	bool write_binary(const std::string& path, const std::vector<unsigned char>& data)
	{
		std::ofstream file(path, std::ios::binary);
		if (!file) return false;
		file.write(reinterpret_cast<const char*>(data.data()), data.size());
		return true;
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

	std::string replace_extension(const std::string& path, const std::string& new_extension)
	{
		std::filesystem::path p = path;
		p.replace_extension(new_extension);
		return p.string();
	}
}