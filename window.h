#pragma once

namespace window
{
	extern const sf::Vector2u BASE_SIZE;
	extern const sf::View BASE_VIEW;

	struct Desc
	{
		std::string title = "Action RPG";
		std::string icon_filename = "assets/window/swordsman.png";
		bool cursor_visible = true;
		bool fullscreen = false;
		uint32_t scale = 5; // Only used if fullscreen is false.
		bool vsync = false;
	};

	// Does not call sf::RenderWindow::create()
	void initialize(sf::RenderWindow& window);
	void create_or_update(const Desc& desc = Desc());
	const Desc& get_desc();

	// Having our own function like this allows us to inject custom events.
	bool poll_event(sf::Event& ev);
	void set_cursor(sf::Cursor::Type type);
	bool has_focus();
}
