#pragma once

namespace window
{
	// The view defines the area of the game world that is visible to the player.
	// Its size is in pixels.
	extern const sf::Vector2u VIEW_SIZE;

	// The window size is limited to a multiple (scale) of the view size.
	// This ensures that pixels are not stretched or squashed.
	extern const uint32_t MIN_SCALE;
	extern const uint32_t MAX_SCALE;

	void initialize(sf::RenderWindow& window);
	void close();
	void set_title(const std::string& title);
	bool set_icon(const std::string& filename);
	void set_cursor_visible(bool visible);
	void set_cursor(sf::Cursor::Type type);
	bool has_focus();
	void set_scale(uint32_t scale);
	uint32_t get_scale();
}
