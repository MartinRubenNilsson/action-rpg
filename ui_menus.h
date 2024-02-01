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

	bool is_menu_visible(MenuType type);
	bool is_any_menu_visible();
	void set_menu_visible(MenuType type, bool visible);
	void show_one_menu_and_hide_rest(MenuType type);
	void hide_all_menus();
}

