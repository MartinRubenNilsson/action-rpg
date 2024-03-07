#pragma once

namespace steam
{
	bool restart_app_if_necessary();
	bool initialize();
	void shutdown();
	void run_message_loop();

	bool is_overlay_active();
}

