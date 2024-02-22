#include "stdafx.h"
#include "ui_hud.h"

namespace ui
{
	namespace bindings
	{
		int hud_player_health = 0;
		int hud_arrow_ammo = 0;
		int hud_bomb_ammo = 0;
		int hud_rupee_amount = 0;
	}

	extern Rml::Context* _context;

	Rml::ElementDocument* _get_hud_document() {
		return _context->GetDocument("hud");
	}

	bool is_hud_visible()
	{
		Rml::ElementDocument* doc = _get_hud_document();
		return doc && doc->IsVisible();
	}

	void set_hud_visible(bool visible)
	{
		if (Rml::ElementDocument* doc = _get_hud_document())
			visible ? doc->Show() : doc->Hide();
	}
}