#include "stdafx.h"
#include "ui_textbox.h"
#include "ui.h"
#include "console.h"

namespace ui
{
	const std::string Textbox::SPRITE_SKULL = "icon-skull";
	const std::string Textbox::SPRITE_GOLDEN_KEY = "icon-golden-key";
	const std::string Textbox::OPENING_SOUND_ITEM_FANFARE = "snd_item_fanfare";
	const std::string Textbox::DEFAULT_TYPING_SOUND = "snd_txt1";
	const float Textbox::DEFAULT_TYPING_SPEED = 25.f;

	std::vector<Textbox> _textbox_presets;

	std::span<const Textbox> find_textbox_presets(const std::string& id)
	{
		auto [first, last] = std::equal_range(_textbox_presets.begin(), _textbox_presets.end(), Textbox{ id },
			[](const Textbox& a, const Textbox& b) { return a.id < b.id; });
		return { first, last };
	}

	void register_textbox_presets()
	{
		{
			Textbox& tb = _textbox_presets.emplace_back();
			tb.id = "player_die";
			tb.text = "You are <span style='color: red'>deader than dead</span>!<br/>Oh, what a pity that your adventure should end here, and so soon...";
			tb.sprite = Textbox::SPRITE_SKULL;
		}
		{
			Textbox& tb = _textbox_presets.emplace_back();
			tb.id = "player_die";
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

		std::stable_sort(_textbox_presets.begin(), _textbox_presets.end(), [](const Textbox& a, const Textbox& b) {
			return a.id < b.id;
		});
	}
}