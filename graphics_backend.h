#pragma once
#include "graphics_types.h"

namespace graphics_backend
{
	using namespace graphics;

	extern const unsigned int MAX_VIEWPORTS;

	void initialize();
	void shutdown();

	void push_debug_group(std::string_view name);
	void pop_debug_group();

	uintptr_t create_buffer(const BufferDesc& desc);
	void destroy_buffer(uintptr_t buffer);
	void update_buffer(uintptr_t buffer, const void* data, unsigned int size, unsigned int offset);
	void bind_uniform_buffer(unsigned int binding, uintptr_t buffer);
	void bind_uniform_buffer_range(unsigned int binding, uintptr_t buffer, unsigned int size, unsigned int offset);

	uintptr_t create_texture(const TextureDesc& desc);
	void destroy_texture(uintptr_t texture);
	void copy_texture_region(
		uintptr_t dst_texture, unsigned int dst_level, unsigned int dst_x, unsigned int dst_y, unsigned int dst_z,
		uintptr_t src_texture, unsigned int src_level, unsigned int src_x, unsigned int src_y, unsigned int src_z,
		unsigned int src_width, unsigned int src_height, unsigned int src_depth);
	void bind_texture(unsigned int binding, uintptr_t texture);

	uintptr_t create_sampler(const SamplerDesc& desc);
	void destroy_sampler(uintptr_t sampler);
	void bind_sampler(unsigned int binding, uintptr_t sampler);

	void set_viewports(const Viewport* viewports, unsigned int count);
	void set_scissors(const Rect* scissors, unsigned int count);
	void set_scissor_test_enabled(bool enable);

	void draw(Primitives primitives, unsigned int vertex_count, unsigned int vertex_offset = 0);
	void draw_indexed(Primitives primitives, unsigned int index_count);
}