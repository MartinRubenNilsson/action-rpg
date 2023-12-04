#include "stdafx.h"
#include "ui_textbox_presets.h"
#include "console.h"

namespace ui
{
	const std::string TEXTBOX_SPRITE_SKULL = "icon-skull";
	const std::string TEXTBOX_SPRITE_GOLDEN_KEY = "icon-golden-key";
	const std::string TEXTBOX_OPENING_SOUND_ITEM_FANFARE = "snd_item_fanfare";
	const std::string TEXTBOX_TYPING_SOUND_DEFAULT = "snd_txt1";
	extern const float TEXTBOX_TYPING_SPEED_DEFAULT = 25.f;

	std::unordered_map<std::string, Textbox> _textbox_presets;

	bool get_textbox_preset(const std::string& name, Textbox& textbox)
	{
		auto it = _textbox_presets.find(name);
		if (it == _textbox_presets.end()) {
			console::log_error("Failed to find textbox preset: " + name);
			return false;
		}
		textbox = it->second;
		return true;
	}

	void create_textbox_presets()
	{
		// PLAYER DIE
		{
			Textbox& textbox = _textbox_presets["player_die_01"];
			textbox.text = "You are <span style='color: red'>deader than dead</span>!<br/>Oh, what a pity that your adventure should end here, and so soon...";
			textbox.sprite = TEXTBOX_SPRITE_SKULL;
			textbox.next_textbox = "player_die_02";
		}
		{
			Textbox& textbox = _textbox_presets["player_die_02"];
			textbox.text = "But wait! There is still hope! Your <span style='color: red'>soul</span> is not yet ready to leave this world...";
			textbox.next_textbox = "player_die_03";
		}
		{
			Textbox& textbox = _textbox_presets["player_die_03"];
			textbox.text = "Would you like to try again?";
		}

		// EXAMINE BLUE GRATE
		{
			Textbox& textbox = _textbox_presets["examine_blue_grate_01"];
			textbox.text = "You stick your hands inside the blue grate. Feeling around with your hands, you find...";
			textbox.next_textbox = "examine_blue_grate_02";
		}
		{
			Textbox& textbox = _textbox_presets["examine_blue_grate_02"];
			textbox.text = "...an expensive-looking <span style='color: #ffd700'>golden key</span>!<br/>Score!";
			textbox.sprite = TEXTBOX_SPRITE_GOLDEN_KEY;
			textbox.opening_sound = TEXTBOX_OPENING_SOUND_ITEM_FANFARE;
		}
	}
}