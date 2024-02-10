#include "stdafx.h"
#include "ui_textbox_presets.h"
#include "ui.h"
#include "console.h"

namespace ui
{
	const std::string Textbox::SPRITE_SKULL = "icon-skull";
	const std::string Textbox::SPRITE_GOLDEN_KEY = "icon-golden-key";
	const std::string Textbox::OPENING_SOUND_ITEM_FANFARE = "snd_item_fanfare";
	const std::string Textbox::DEFAULT_TYPING_SOUND = "snd_txt1";
	const float Textbox::DEFAULT_TYPING_SPEED = 25.f;

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
				tb.sprite = Textbox::SPRITE_SKULL;
			}
			{
				Textbox& tb = tbs.emplace_back();
				tb.text = "Would you like to try again?";
				tb.options = { "Yes", "No" };
				tb.options_callback = [](const std::string& option) {
					if (option == "Yes") {
						bindings::on_click_restart();
					} else if (option == "No") {
						bindings::on_click_main_menu();
					}
				};
			}
		}
	}
}