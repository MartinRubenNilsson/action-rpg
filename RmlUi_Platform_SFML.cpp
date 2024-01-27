#include "RmlUi_Platform_SFML.h"
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Input.h>
#include <RmlUi/Core/StringUtilities.h>
#include <RmlUi/Core/SystemInterface.h>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include "window.h"

double SystemInterface_SFML::GetElapsedTime() {
	return static_cast<double>(timer.getElapsedTime().asMicroseconds()) / 1'000'000.0;
}

void SystemInterface_SFML::SetMouseCursor(const Rml::String& cursor_name)
{
	if (cursor_name.empty() || cursor_name == "arrow")
		window::set_cursor(sf::Cursor::Arrow);
	else if (cursor_name == "move")
		window::set_cursor(sf::Cursor::SizeAll);
	else if (cursor_name == "pointer")
		window::set_cursor(sf::Cursor::Hand);
	else if (cursor_name == "resize")
		window::set_cursor(sf::Cursor::SizeTopLeftBottomRight);
	else if (cursor_name == "cross")
		window::set_cursor(sf::Cursor::Cross);
	else if (cursor_name == "text")
		window::set_cursor(sf::Cursor::Text);
	else if (cursor_name == "unavailable")
		window::set_cursor(sf::Cursor::NotAllowed);
	else if (cursor_name.starts_with("rmlui-scroll"))
		window::set_cursor(sf::Cursor::SizeAll);
}

void SystemInterface_SFML::SetClipboardText(const Rml::String& text_utf8) {
	sf::Clipboard::setString(text_utf8);
}

void SystemInterface_SFML::GetClipboardText(Rml::String& text) {
	text = sf::Clipboard::getString();
}

