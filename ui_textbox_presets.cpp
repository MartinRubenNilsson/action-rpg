#include "stdafx.h"
#include "ui_textbox_presets.h"
#include "ui.h"
#include "console.h"

namespace ui
{
	const std::string TEXTBOX_SPRITE_SKULL = "icon-skull";
	const std::string TEXTBOX_SPRITE_GOLDEN_KEY = "icon-golden-key";
	const std::string TEXTBOX_OPENING_SOUND_ITEM_FANFARE = "snd_item_fanfare";
	const std::string TEXTBOX_DEFAULT_TYPING_SOUND = "snd_txt1";
	extern const float TEXTBOX_DEFAULT_TYPING_SPEED = 25.f;

	std::unordered_map<std::string, std::vector<Textbox>> _textbox_presets;

	bool get_textbox_presets(const std::string& name, std::vector<Textbox>& textboxes)
	{
		auto it = _textbox_presets.find(name);
		if (it == _textbox_presets.end()) {
			console::log_error("Failed to find textbox preset: " + name);
			return false;
		}
		textboxes = it->second;
		return true;
	}

	void initialize_textbox_presets()
	{
		// PLAYER DIE
		{
			auto& tbs = _textbox_presets["player_die"];
			{
				Textbox& tb = tbs.emplace_back();
				tb.text = "You are <span style='color: red'>deader than dead</span>!<br/>Oh, what a pity that your adventure should end here, and so soon...";
				tb.sprite = TEXTBOX_SPRITE_SKULL;
			}
			{
				Textbox& tb = tbs.emplace_back();
				tb.text = "But wait! There is still hope! Your <span style='color: red'>soul</span> is not yet ready to leave this world...";
			}
			{
				Textbox& tb = tbs.emplace_back();
				tb.text = "Would you like to try again?";
				tb.select_options = { "Yes", "No" };
				tb.select_callback = [](const std::string& option) {
					if (option == "Yes") {
						push_event({ Event::RestartMap });
					} else if (option == "No") {
						push_event({ Event::GoToMainMenu });
					}
				};
			}
		}

		/* // EXAMINE BLUE GRATE
		{
			Textbox& textbox = _textbox_presets["examine_blue_grate"];
			textbox.text = "You stick your hands inside the blue grate. Feeling around with your hands, you find...";
		}
		{
			Textbox& textbox = _textbox_presets["examine_blue_grate"];
			textbox.text = "...an expensive-looking <span style='color: #ffd700'>golden key</span>!<br/>Score!";
			textbox.sprite = TEXTBOX_SPRITE_GOLDEN_KEY;
			textbox.opening_sound = TEXTBOX_OPENING_SOUND_ITEM_FANFARE;
		}*/
	}
}