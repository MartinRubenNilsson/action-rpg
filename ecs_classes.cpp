#include "stdafx.h"
#include "ecs_classes.h"

namespace ecs
{
    bool string_to_class(std::string_view string, Class& class_)
    {
		char sanitized_string[64] = {}; // lowercase alphabetic characters only
		for (size_t i = 0, j = 0; i < string.size() && j < std::size(sanitized_string); ++i) {
			if (!isalpha(string[i])) continue;
			sanitized_string[j++] = tolower(string[i]);
		}
		auto value = magic_enum::enum_cast<Class>(sanitized_string, magic_enum::case_insensitive);
		if (!value.has_value()) return false;
		class_ = value.value();
		return true;
    }

	std::string_view class_to_string(Class class_)
	{
		return magic_enum::enum_name(class_);
	}
}
