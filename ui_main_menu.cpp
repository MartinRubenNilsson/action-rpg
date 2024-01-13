#include "stdafx.h"
#include "ui_main_menu.h"
#include <RmlUi/Core.h>

namespace ui
{
	extern Rml::Context* _context;

	Rml::ElementDocument* get_main_menu_document() {
		return _context->GetDocument("main_menu");
	}

	bool is_main_menu_visible()
	{
		if (const Rml::ElementDocument* doc = get_main_menu_document())
			return doc->IsVisible();
		return false;
	}

	void set_main_menu_visible(bool visible)
	{
		if (Rml::ElementDocument* doc = get_main_menu_document())
			visible ? doc->Show() : doc->Hide();
	}
}