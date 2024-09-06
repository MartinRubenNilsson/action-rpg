#pragma once

namespace platform
{
	// Sets the contents of the specified environment variable for the current process.
	bool set_environment_variable(const char* name, const char* value);
}