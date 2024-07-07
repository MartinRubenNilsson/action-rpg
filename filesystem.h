#pragma once

namespace filesystem
{
	std::string get_normalized_path(const std::string& path);
	std::string get_parent_path(const std::string& path);
	std::string get_stem(const std::string& path);
}