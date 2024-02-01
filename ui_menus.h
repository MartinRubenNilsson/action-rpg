#pragma once

namespace ui
{
	enum class MenuType
	{
		Main,
		Pause,
		Settings,
		Credits,
		Count
	};

	MenuType get_current_menu(); // Returns MenuType::Count if stack is empty.
	void push_menu(MenuType type);
	void pop_menu();
	void pop_all_menus();
}

