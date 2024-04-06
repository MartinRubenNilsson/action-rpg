#pragma once

namespace window
{
	extern const sf::Vector2u BASE_SIZE;
	extern const sf::View BASE_VIEW;

	struct State
	{
		std::string title = "Action RPG";
		std::string icon = "assets/window/swordsman.png"; // Path to icon file.
		bool cursor_visible = true;
		bool fullscreen = false;
		uint32_t scale = 5; // Only used if fullscreen is false.
		bool vsync = false;
	};

	// Does not call sf::RenderWindow::create()
	void initialize(sf::RenderWindow& window);
	void set_state(const State& state);
	const State& get_state();

	// Having our own function like this allows us to inject custom events.
	bool poll_event(sf::Event& ev);
	void set_cursor(sf::Cursor::Type type);
	bool has_focus();
}
