#pragma once
#include "graphics_api_config.h"
#include <span>

#ifdef GRAPHICS_API_OPENGL
typedef void* (*GLADloadproc)(const char* name);
#endif

namespace window {

#ifdef GRAPHICS_API_OPENGL
	void make_opengl_context_current();
	GLADloadproc get_glad_load_proc();
	void present_swap_chain_back_buffer();
	void set_swap_chain_sync_interval(int sync_interval);
#endif

#ifdef GRAPHICS_API_VULKAN
	bool is_vulkan_supported();
	std::span<const char*> get_required_vulkan_instance_extensions();
#endif

}