#pragma once

namespace steam
{
	bool restart_app_if_necessary();
	bool initialize();
	void shutdown();
	void process_events();

	bool is_overlay_active();
}

