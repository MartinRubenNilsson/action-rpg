#include "stdafx.h"
#ifdef GRAPHICS_BACKEND_VULKAN
#include "graphics_backend.h"
#include <vulkan/vulkan.h>

namespace graphics_backend
{
	void initialize() {}
	void shutdown() {}

	uintptr_t create_buffer(const BufferDesc& desc) { return 0; }
	void destroy_buffer(uintptr_t buffer) {}

	uintptr_t create_texture(const TextureDesc& desc) { return 0; }
	void destroy_texture(uintptr_t texture) {}
	void bind_texture(unsigned int binding, uintptr_t texture) {}

	uintptr_t create_sampler(const SamplerDesc& desc) { return 0; }
	void destroy_sampler(uintptr_t sampler) {}
	void bind_sampler(unsigned int binding, uintptr_t sampler) {}

	void set_viewports(const Viewport* viewports, unsigned int count) {}
	void set_scissors(const Rect* scissors, unsigned int count) {}
	void set_scissor_test_enabled(bool enable) {}

	void push_debug_group(std::string_view name) {}
	void pop_debug_group() {}
}
#endif // GRAPHICS_BACKEND_VULKAN