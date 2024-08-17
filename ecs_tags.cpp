#include "stdafx.h"
#include "ecs_tags.h"

namespace ecs
{
    bool string_to_tag(std::string_view string, Tag& tag)
    {
		char sanitized_string[64] = {}; // lowercase alphabetic characters only
		for (size_t i = 0, j = 0; i < string.size() && j < std::size(sanitized_string); ++i) {
			if (!isalpha(string[i])) continue;
			sanitized_string[j++] = tolower(string[i]);
		}
		auto value = magic_enum::enum_cast<Tag>(sanitized_string, magic_enum::case_insensitive);
		if (!value.has_value()) return false;
		tag = value.value();
		return true;
    }

	std::string_view tag_to_string(Tag tag)
	{
		return magic_enum::enum_name(tag);
	}
}
