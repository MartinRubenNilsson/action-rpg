#pragma once

namespace ui
{
	extern const std::string TEXTBOX_SPRITE_SKULL;
	extern const std::string TEXTBOX_SPRITE_GOLDEN_KEY;
	extern const std::string TEXTBOX_OPENING_SOUND_ITEM_FANFARE;

	extern const std::string TEXTBOX_DEFAULT_TYPING_SOUND;
	extern const float TEXTBOX_DEFAULT_TYPING_SPEED;

	struct Textbox
	{
		std::string text; // RML
		std::string sprite;
		std::string opening_sound; // name of sound event
		std::string typing_sound = TEXTBOX_DEFAULT_TYPING_SOUND;
		float typing_speed = TEXTBOX_DEFAULT_TYPING_SPEED; // in chars per second, 0 = instant
		std::vector<std::string> select_options;
		void (*select_callback)(const std::string& option) = nullptr;
	};

	bool get_textbox_presets(const std::string& name, std::vector<Textbox>& textboxes);
	void initialize_textbox_presets();
}

