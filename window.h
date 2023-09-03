#pragma once

namespace window
{
	sf::RenderWindow& create();
	void close();
	void set_title(const std::string& title);
	bool has_focus();
}
