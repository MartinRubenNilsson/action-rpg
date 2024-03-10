#pragma once

namespace steam
{
	bool server_initialize();
	void server_shutdown();
	/*void server_log_on_anonymous();
	void server_log_off();
	bool server_logged_on();*/
	std::string server_get_public_ip();
}

