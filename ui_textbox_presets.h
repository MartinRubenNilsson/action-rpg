#pragma once

namespace ui
{
	extern const std::string TEXTBOX_SPRITE_SKULL;
	extern const std::string TEXTBOX_SPRITE_GOLDEN_KEY;
	extern const std::string TEXTBOX_OPENING_SOUND_ITEM_FANFARE;
	extern const std::string TEXTBOX_TYPING_SOUND_DEFAULT;
	extern const float TEXTBOX_TYPING_SPEED_DEFAULT;

	struct Textbox
	{
		std::string text; // RML
		std::string sprite;
		std::string opening_sound; // name of sound event
		// std::string closing_sound; // TODO: implement
		std::string typing_sound = TEXTBOX_TYPING_SOUND_DEFAULT;
		float typing_speed = TEXTBOX_TYPING_SPEED_DEFAULT; // in CPS, 0 = instant
		// bool auto_close = false; // TODO: implement
		std::string next_textbox; // name of textbox preset
		// bool sprite_to_left = false; // TODO: implement
	};

	bool get_textbox_preset(const std::string& name, Textbox& textbox);
	void create_textbox_presets();
}

