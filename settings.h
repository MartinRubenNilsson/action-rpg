#pragma once

namespace settings {

	struct AppSettings {
		bool fullscreen = false;
		unsigned int window_scale = 5; // Window size relative to WINDOW_MIN_WIDTH/HEIGHT
		bool vsync = false;
		float volume_master = 1.f;
		float volume_music = 1.f;
		float volume_sound = 1.f;
	};

	extern AppSettings app_settings;

	void apply(const AppSettings &settings);

	void save_to_stream(std::ostream& os, const AppSettings& settings);
	void load_from_stream(std::istream& is, AppSettings& settings);

	extern const std::string_view APP_SETTINGS_PATH;

	bool save_to_file(std::string_view path, const AppSettings &settings);
	bool load_from_file(std::string_view path, AppSettings& settings);
}
