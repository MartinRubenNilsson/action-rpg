#pragma once

namespace platform {
	int system(const char* command);
	bool set_environment_variable(const char* name, const char* value);
	bool open(const char* path);
	bool is_debugger_present();
	void debug_break();
	void output_debug_string(const char* string);
}

#ifdef _DEBUG
#define _DEBUG_BREAK() platform::debug_break()
#define _OUTPUT_DEBUG_STRING(string) platform::output_debug_string(string)
#else
#define _DEBUG_BREAK()
#define _OUTPUT_DEBUG_STRING(string)
#endif