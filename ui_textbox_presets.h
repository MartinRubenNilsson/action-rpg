#pragma once

namespace ui
{
	struct Textbox
	{
		static const std::string SPRITE_SKULL;
		static const std::string SPRITE_GOLDEN_KEY;
		static const std::string OPENING_SOUND_ITEM_FANFARE;
		static const std::string DEFAULT_TYPING_SOUND;
		static const float DEFAULT_TYPING_SPEED;

		std::string text; // RML
		std::string sprite;
		std::string opening_sound; // name of sound event
		std::string typing_sound = DEFAULT_TYPING_SOUND;
		float typing_speed = DEFAULT_TYPING_SPEED; // in chars per second, 0 = instant
		std::vector<std::string> options;
		void (*options_callback)(const std::string& option) = nullptr;
	};

	bool get_textbox_presets(const std::string& name, std::vector<Textbox>& textboxes);
	void initialize_textbox_presets();
}

