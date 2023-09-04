#pragma once

namespace window
{
	sf::RenderWindow& create();
	void close();
	void set_title(const std::string& title);
	bool has_focus();
	void set_scale(uint32_t scale);
	uint32_t get_scale();
}
