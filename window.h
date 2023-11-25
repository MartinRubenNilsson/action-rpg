#pragma once

namespace window
{
	void initialize(sf::RenderWindow& window);
	void close();
	void set_title(const std::string& title);
	bool has_focus();
	void set_scale(uint32_t scale);
	uint32_t get_scale();
}
