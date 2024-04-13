#pragma once

namespace map
{
	enum class RequestType
	{
		Open, // Open a new map.
		Close, // Close the current map.
		Reset, // Reset the current map.
	};

	struct Request
	{
		RequestType type = RequestType::Open;
		std::string map_name;
		bool reset_if_already_open = false;
	};

	extern bool debug;

	void update(float dt);

	bool request(const Request& request);
	bool open(const std::string& map_name);
	bool close();
	bool reset();
}
