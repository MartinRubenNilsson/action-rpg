#include "stdafx.h"
#include "ui_rmlui_system_interface.h"
#include "window.h"

namespace ui
{
	double RmlUiSystemInterface::GetElapsedTime() {
		return window::get_elapsed_time();
	}

	void RmlUiSystemInterface::SetMouseCursor(const Rml::String& cursor_name)
	{
#if 0
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
#endif
	}

	void RmlUiSystemInterface::SetClipboardText(const Rml::String& text) {
		window::set_clipboard_string(text);
	}

	void RmlUiSystemInterface::GetClipboardText(Rml::String& text) {
		text = window::get_clipboard_string();
	}
}
