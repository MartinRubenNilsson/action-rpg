#pragma once

template <typename T>
struct Handle {
	uint16_t index = 0;
	uint16_t generation = 0; // Valid generations start at 1.

	auto operator<=>(const Handle&) const = default;
};