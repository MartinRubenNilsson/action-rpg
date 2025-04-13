#include "stdafx.h"
#ifdef PLATFORM_WINDOWS
#include "platform.h"
#include <Windows.h>

namespace platform {
	int system(const char* command) {
		return ::system(command);
	}

	bool set_environment_variable(const char* name, const char* value) {
		return ::SetEnvironmentVariableA(name, value);
	}
}

#endif // PLATFORM_WINDOWS