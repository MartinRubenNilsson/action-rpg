#include "stdafx.h"
#ifdef PLATFORM_WINDOWS
#include "platform.h"
#include <Windows.h>
#include <debugapi.h>

namespace platform {
	int system(const char* command) {
		return ::system(command);
	}

	bool set_environment_variable(const char* name, const char* value) {
		return SetEnvironmentVariableA(name, value);
	}

	bool is_debugger_present() {
		return IsDebuggerPresent();
	}

	void debug_break() {
		DebugBreak();
	}

	void output_debug_string(const char* string) {
		OutputDebugStringA(string);
	}
}

#endif // PLATFORM_WINDOWS