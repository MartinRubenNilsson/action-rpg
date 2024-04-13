#pragma once

namespace map
{
	extern bool debug;

	// Calling open(), close(), or reset() will not immediately change the map.
	// Instead, the change will be queued and will take effect when update() is called.

	void open(const std::string& map_name, bool reset_if_open = false);
	void close();
	void reset();
	void update();

	bool is_open();
	std::string get_name();
}
