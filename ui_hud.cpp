#include "stdafx.h"
#include "ui_hud.h"
#include <RmlUi/Core.h>

namespace ui
{
	extern Rml::Context* _context;
	int hud_player_health = 0; // int instead of uint32_t to avoid underflow problems

	Rml::ElementDocument* get_hud_document() {
		return _context->GetDocument("hud");
	}

	bool is_hud_visible()
	{
		if (const Rml::ElementDocument* doc = get_hud_document())
			return doc->IsVisible();
		return false;
	}

	void set_hud_visible(bool visible)
	{
		if (Rml::ElementDocument* doc = get_hud_document())
			visible ? doc->Show() : doc->Hide();
	}
}