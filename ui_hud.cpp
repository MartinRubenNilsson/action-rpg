#include "stdafx.h"
#include "ui_hud.h"

namespace ui
{
	extern Rml::Context* _context;

	Rml::ElementDocument* _get_hud_document()
	{
		return _context->GetDocument("hud");
	}

	bool get_hud_visible()
	{
		Rml::ElementDocument* doc = _get_hud_document();
		return doc && doc->IsVisible();
	}

	void set_hud_visible(bool visible)
	{
		if (Rml::ElementDocument* doc = _get_hud_document()) {
			visible ? doc->Show() : doc->Hide();
		}
	}
}