#pragma once

// graphics_api.h - Low-level graphics API

#include "graphics_config.h"
#include "graphics_types.h"
#include <span>

#ifdef GRAPHICS_API_D3D11
struct ID3D11Device;
struct ID3D11DeviceContext;
#endif

namespace graphics {
namespace api {

	using DebugMessageCallback = void(*)(std::string_view message);

	void set_debug_message_callback(DebugMessageCallback callback);

	struct InitializeOptions {
#ifdef GRAPHICS_API_OPENGL
		void* (*glad_load_proc)(const char* name) = nullptr;
#endif
#ifdef GRAPHICS_API_VULKAN
		const char* application_name = "Application";
		const char* engine_name = "Engine";
		std::span<const char*> vulkan_instance_extensions;
#endif
#ifdef GRAPHICS_API_D3D11
		void* hwnd = nullptr; // win32 window handle (HWND)
#endif
	};

	bool initialize(const InitializeOptions &options);
	void shutdown();

#ifdef GRAPHICS_API_D3D11
	ID3D11Device* get_d3d11_device();
	ID3D11DeviceContext* get_d3d11_device_context();
	void swap_buffers();
#endif

	void push_debug_group(std::string_view name);
	void pop_debug_group();

	struct VertexArrayHandle { uintptr_t object = 0; };

	VertexArrayHandle create_vertex_array(const VertexArrayDesc& desc);
	void destroy_vertex_array(VertexArrayHandle vertex_array);
	void bind_vertex_array(VertexArrayHandle vertex_array);

	struct ShaderHandle { uintptr_t object = 0; };

	ShaderHandle create_shader(const ShaderDesc& desc);
	void destroy_shader(ShaderHandle shader);
	void bind_shader(ShaderHandle shader);

	struct BufferHandle { uintptr_t object = 0; };

	BufferHandle create_buffer(const BufferDesc& desc);
	void destroy_buffer(BufferHandle buffer);
	void update_buffer(BufferHandle buffer, const void* data, unsigned int size, unsigned int offset);
	void bind_uniform_buffer(unsigned int binding, BufferHandle buffer);
	void bind_uniform_buffer_range(unsigned int binding, BufferHandle buffer, unsigned int size, unsigned int offset);
	void bind_vertex_buffer(VertexArrayHandle vertex_array, unsigned int binding, BufferHandle buffer, unsigned int stride, unsigned int offset);
	void bind_index_buffer(VertexArrayHandle vertex_array, BufferHandle buffer);

	struct TextureHandle { uintptr_t object = 0; };

	TextureHandle create_texture(const TextureDesc& desc);
	void destroy_texture(TextureHandle texture);
	void update_texture(
		TextureHandle texture,
		unsigned int level,
		unsigned int x,
		unsigned int y,
		unsigned int width,
		unsigned int height,
		Format pixel_format,
		const void* pixels);
	void copy_texture(
		TextureHandle dst_texture,
		unsigned int dst_level,
		unsigned int dst_x,
		unsigned int dst_y,
		unsigned int dst_z,
		TextureHandle src_texture,
		unsigned int src_level,
		unsigned int src_x,
		unsigned int src_y,
		unsigned int src_z,
		unsigned int src_width,
		unsigned int src_height,
		unsigned int src_depth);
	void bind_texture(unsigned int binding, TextureHandle texture);

	struct SamplerHandle { uintptr_t object = 0; };

	SamplerHandle create_sampler(const SamplerDesc& desc);
	void destroy_sampler(SamplerHandle sampler);
	void bind_sampler(unsigned int binding, SamplerHandle sampler);

	struct FramebufferHandle { uintptr_t object = 0; };

	FramebufferHandle create_framebuffer(const FramebufferDesc& desc);
	void destroy_framebuffer(FramebufferHandle framebuffer);
	bool attach_framebuffer_texture(FramebufferHandle framebuffer, TextureHandle texture);
	void clear_framebuffer(FramebufferHandle framebuffer, const float color[4]);
	void bind_framebuffer(FramebufferHandle framebuffer);

	void set_viewports(const Viewport* viewports, unsigned int count);
	void set_scissors(const Rect* scissors, unsigned int count);
	void set_scissor_test_enabled(bool enable);

	void draw(Primitives primitives, unsigned int vertex_count, unsigned int vertex_offset = 0);
	void draw_indexed(Primitives primitives, unsigned int index_count);

} // namespace api
} // namespace graphics