#include "stdafx.h"
#include "ui_pause_menu.h"
#include <RmlUi/Core.h>

namespace ui
{
	extern Rml::Context* _context;

	Rml::ElementDocument* _get_pause_menu_document() {
		return _context->GetDocument("pause_menu");
	}

	bool is_pause_menu_visible()
	{
		if (const Rml::ElementDocument* doc = _get_pause_menu_document())
			return doc->IsVisible();
		return false;
	}

	void set_pause_menu_visible(bool visible)
	{
		if (Rml::ElementDocument* doc = _get_pause_menu_document())
			visible ? doc->Show() : doc->Hide();
	}

	void toggle_pause_menu_visible() {
		set_pause_menu_visible(!is_pause_menu_visible());
	}
}