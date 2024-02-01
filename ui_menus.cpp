#include "stdafx.h"
#include "ui_menus.h"
#include <RmlUi/Core.h>

namespace ui
{
	extern Rml::Context* _context;

	Rml::ElementDocument* _get_menu_document(MenuType type)
	{
		switch (type) {
		case MenuType::Main:
			return _context->GetDocument("main_menu");
		case MenuType::Pause:
			return _context->GetDocument("pause_menu");
		case MenuType::Settings:
			return _context->GetDocument("settings_menu");
		case MenuType::Credits:
			return _context->GetDocument("credits_menu");
		default:
			return nullptr;
		}
	}

	bool is_menu_visible(MenuType type)
	{
		Rml::ElementDocument* doc = _get_menu_document(type);
		return doc && doc->IsVisible();
	}

	bool is_any_menu_visible()
	{
		for (int i = 0; i < (int)MenuType::Count; ++i)
			if (is_menu_visible((MenuType)i))
				return true;
		return false;
	}

	void set_menu_visible(MenuType type, bool visible)
	{
		if (Rml::ElementDocument* doc = _get_menu_document(type))
			visible ? doc->Show() : doc->Hide();
	}

	void show_one_menu_and_hide_rest(MenuType type)
	{
		for (int i = 0; i < (int)MenuType::Count; ++i)
			set_menu_visible((MenuType)i, (MenuType)i == type);
	}

	void hide_all_menus()
	{
		for (int i = 0; i < (int)MenuType::Count; ++i)
			set_menu_visible((MenuType)i, false);
	}
}