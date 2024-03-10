#pragma once

namespace steam
{
	bool server_is_initialized();
	bool server_initialize();
	void server_shutdown();

	void server_log_on_anonymous();
	void server_log_off();
	bool server_logged_on();
}

