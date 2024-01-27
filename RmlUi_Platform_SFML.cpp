#include "RmlUi_Platform_SFML.h"
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Input.h>
#include <RmlUi/Core/StringUtilities.h>
#include <RmlUi/Core/SystemInterface.h>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

SystemInterface_SFML::SystemInterface_SFML()
{
	cursors_valid = true;
	cursors_valid &= cursor_default.loadFromSystem(sf::Cursor::Arrow);
	cursors_valid &= cursor_move.loadFromSystem(sf::Cursor::SizeAll);
	cursors_valid &= cursor_pointer.loadFromSystem(sf::Cursor::Hand);
	cursors_valid &= cursor_resize.loadFromSystem(sf::Cursor::SizeTopLeftBottomRight)
		|| cursor_resize.loadFromSystem(sf::Cursor::SizeAll);
	cursors_valid &= cursor_cross.loadFromSystem(sf::Cursor::Cross);
	cursors_valid &= cursor_text.loadFromSystem(sf::Cursor::Text);
	cursors_valid &= cursor_unavailable.loadFromSystem(sf::Cursor::NotAllowed);
}

void SystemInterface_SFML::SetWindow(sf::RenderWindow* in_window) {
	window = in_window;
}

double SystemInterface_SFML::GetElapsedTime() {
	return static_cast<double>(timer.getElapsedTime().asMicroseconds()) / 1'000'000.0;
}

void SystemInterface_SFML::SetMouseCursor(const Rml::String& cursor_name)
{
	if (!cursors_valid || !window) return;

	sf::Cursor* cursor = nullptr;
	if (cursor_name.empty() || cursor_name == "arrow")
		cursor = &cursor_default;
	else if (cursor_name == "move")
		cursor = &cursor_move;
	else if (cursor_name == "pointer")
		cursor = &cursor_pointer;
	else if (cursor_name == "resize")
		cursor = &cursor_resize;
	else if (cursor_name == "cross")
		cursor = &cursor_cross;
	else if (cursor_name == "text")
		cursor = &cursor_text;
	else if (cursor_name == "unavailable")
		cursor = &cursor_unavailable;
	else if (Rml::StringUtilities::StartsWith(cursor_name, "rmlui-scroll"))
		cursor = &cursor_move;

	if (cursor)
		window->setMouseCursor(*cursor);
}

void SystemInterface_SFML::SetClipboardText(const Rml::String& text_utf8) {
	sf::Clipboard::setString(text_utf8);
}

void SystemInterface_SFML::GetClipboardText(Rml::String& text) {
	text = sf::Clipboard::getString();
}

