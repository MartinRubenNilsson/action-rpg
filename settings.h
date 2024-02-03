#pragma once

struct Settings
{
	bool fullscreen = false;
	bool vsync = false;
	float volume_master = 1.f;
	float volume_music = 1.f;
	float volume_sound = 1.f;

	void set() const;
	void get();
	void write(std::ostream& os) const;
	void read(std::istream& is);
	bool save(const std::filesystem::path& filename = "settings.txt") const;
	bool load(const std::filesystem::path& filename = "settings.txt");
};

