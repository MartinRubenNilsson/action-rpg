#pragma once
#include <RmlUi/Core/Input.h>
#include <RmlUi/Core/SystemInterface.h>
#include <RmlUi/Core/Types.h>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

class SystemInterface_SFML : public Rml::SystemInterface
{
public:
	SystemInterface_SFML();

	void SetWindow(sf::RenderWindow* window);
	double GetElapsedTime() override;
	void SetMouseCursor(const Rml::String& cursor_name) override;
	void SetClipboardText(const Rml::String& text) override;
	void GetClipboardText(Rml::String& text) override;

private:
	sf::Clock timer;
	sf::RenderWindow* window = nullptr;

	bool cursors_valid = false;
	sf::Cursor cursor_default;
	sf::Cursor cursor_move;
	sf::Cursor cursor_pointer;
	sf::Cursor cursor_resize;
	sf::Cursor cursor_cross;
	sf::Cursor cursor_text;
	sf::Cursor cursor_unavailable;
};
