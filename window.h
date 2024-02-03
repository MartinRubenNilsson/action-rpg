#pragma once

namespace window
{
	struct Desc
	{
		std::string title = "Action RPG";
		std::string icon_filename = "assets/window/swordsman.png";
		bool cursor_visible = true;
		bool fullscreen = false;
		bool vsync = false;
	};

	// The view defines the area of the game world that is visible to the player.
	extern const sf::Vector2u VIEW_SIZE;

	void initialize(sf::RenderWindow& window); // Does not create the window.
	void create_or_update(const Desc& desc = Desc());
	const Desc& get_desc();

	// Having our own function like this allows us to inject custom events.
	bool poll_event(sf::Event& ev);
	void set_cursor(sf::Cursor::Type type);
	bool has_focus();
}
