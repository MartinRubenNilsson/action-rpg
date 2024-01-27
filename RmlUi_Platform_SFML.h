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

namespace RmlSFML
{
	// Applies input on the context based on the given SFML event.
	// @return True if the event is still propagating, false if it was handled by the context.
	bool InputHandler(Rml::Context* context, const sf::Event& ev);
	Rml::Input::KeyIdentifier sfml_key_to_rml_key(sf::Keyboard::Key key);
	Rml::Input::KeyModifier get_key_modifier_state();
}
