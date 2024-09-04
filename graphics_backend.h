#pragma once
#include "graphics_types.h"

// graphics_backend.h - Low-level graphics API

namespace graphics_backend
{
	using namespace graphics;
	using DebugMessageCallback = void(*)(std::string_view message);

	extern const unsigned int MAX_VIEWPORTS;

	void set_debug_message_callback(DebugMessageCallback callback);

	void initialize();
	void shutdown();

	void push_debug_group(std::string_view name);
	void pop_debug_group();

	uintptr_t create_shader(const ShaderDesc& desc);
	void destroy_shader(uintptr_t shader);
	void bind_shader(uintptr_t shader);

	uintptr_t create_buffer(const BufferDesc& desc);
	void destroy_buffer(uintptr_t buffer);
	void update_buffer(uintptr_t buffer, const void* data, unsigned int size, unsigned int offset);
	void bind_uniform_buffer(unsigned int binding, uintptr_t buffer);
	void bind_uniform_buffer_range(unsigned int binding, uintptr_t buffer, unsigned int size, unsigned int offset);
	void bind_vertex_buffer(unsigned int binding, uintptr_t buffer, unsigned int stride, unsigned int offset);
	void bind_index_buffer(uintptr_t buffer);

	uintptr_t create_texture(const TextureDesc& desc);
	void destroy_texture(uintptr_t texture);
	void update_texture(uintptr_t texture, unsigned int level, unsigned int x, unsigned int y,
		unsigned int width, unsigned int height, Format pixel_format, const void* pixels);
	void copy_texture(
		uintptr_t dst_texture, unsigned int dst_level, unsigned int dst_x, unsigned int dst_y, unsigned int dst_z,
		uintptr_t src_texture, unsigned int src_level, unsigned int src_x, unsigned int src_y, unsigned int src_z,
		unsigned int src_width, unsigned int src_height, unsigned int src_depth);
	void bind_texture(unsigned int binding, uintptr_t texture);

	uintptr_t create_sampler(const SamplerDesc& desc);
	void destroy_sampler(uintptr_t sampler);
	void bind_sampler(unsigned int binding, uintptr_t sampler);

	uintptr_t create_framebuffer(const FramebufferDesc& desc);
	void destroy_framebuffer(uintptr_t framebuffer);
	bool attach_framebuffer_texture(uintptr_t framebuffer, uintptr_t texture);
	void clear_framebuffer(uintptr_t framebuffer, const float color[4]);
	void bind_framebuffer(uintptr_t framebuffer);

	void set_viewports(const Viewport* viewports, unsigned int count);
	void set_scissors(const Rect* scissors, unsigned int count);
	void set_scissor_test_enabled(bool enable);

	void draw(Primitives primitives, unsigned int vertex_count, unsigned int vertex_offset = 0);
	void draw_indexed(Primitives primitives, unsigned int index_count);
}