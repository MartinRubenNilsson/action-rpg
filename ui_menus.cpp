#include "stdafx.h"
#include "ui_menus.h"
#include <RmlUi/Core.h>

namespace ui
{
	extern Rml::Context* _context;
	std::vector<MenuType> _menu_stack;

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

	MenuType get_current_menu() {
		return _menu_stack.empty() ? MenuType::Count : _menu_stack.back();
	}

	void _set_menu_visible(MenuType type, bool visible)
	{
		if (Rml::ElementDocument* doc = _get_menu_document(type))
			visible ? doc->Show() : doc->Hide();
	}

	void push_menu(MenuType type)
	{
		if (!_menu_stack.empty())
			_set_menu_visible(_menu_stack.back(), false);
		_menu_stack.push_back(type);
		_set_menu_visible(type, true);
	}

	void pop_menu()
	{
		if (_menu_stack.empty()) return;
		_set_menu_visible(_menu_stack.back(), false);
		_menu_stack.pop_back();
		if (_menu_stack.empty()) return;
		_set_menu_visible(_menu_stack.back(), true);
	}

	void pop_all_menus()
	{
		_menu_stack.clear();
		for (int i = 0; i < (int)MenuType::Count; ++i)
			_set_menu_visible((MenuType)i, false);
	}
}