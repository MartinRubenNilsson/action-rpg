#pragma once

namespace ui
{
	namespace bindings
	{
		extern int hud_player_health;
		extern int hud_arrow_ammo;
		extern int hud_bomb_ammo;
		extern int hud_rupee_amount;
	}

	bool is_hud_visible();
	void set_hud_visible(bool visible);
}
