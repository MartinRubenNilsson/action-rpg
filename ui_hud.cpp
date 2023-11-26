#include "stdafx.h"
#include "ui_hud.h"
#include <RmlUi/Core.h>

namespace ui
{
	extern Rml::Context* _context;
	int hud_player_health = 0; // int instead of uint32_t to avoid underflow problems

	bool is_hud_visible()
	{
		if (const Rml::ElementDocument* doc = _context->GetDocument("hud"))
			return doc->IsVisible();
		return false;
	}

	void set_hud_visible(bool visible)
	{
		if (Rml::ElementDocument* doc = _context->GetDocument("hud"))
			visible ? doc->Show() : doc->Hide();
	}
}