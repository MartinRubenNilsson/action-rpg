#include "stdafx.h"
#include "filesystem.h"
#include <filesystem>

namespace filesystem
{
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