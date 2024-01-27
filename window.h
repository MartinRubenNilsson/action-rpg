#pragma once

namespace window
{
	void initialize(sf::RenderWindow& window);
	void close();
	void set_title(const std::string& title);
	bool set_icon(const std::string& filename);
	void set_cursor(sf::Cursor::Type type);
	bool has_focus();
	void set_scale(uint32_t scale);
	uint32_t get_scale();
}
