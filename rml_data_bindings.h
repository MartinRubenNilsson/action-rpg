#pragma once

namespace rml
{
	bool is_variable_dirty(const std::string& name);
	void dirty_variable(const std::string& name);

	extern std::string test_string;
}

