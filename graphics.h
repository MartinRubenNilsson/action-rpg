#pragma once
#include "graphics_types.h"
#include "handle.h"
#include <string_view>

// graphics.h - High-level graphics API

namespace graphics {

	struct VertexShader;
	struct FragmentShader;
	struct VertexInput;
	struct Buffer;
	struct Texture;
	struct Framebuffer;
	struct Sampler;
	struct RasterizerState;
	struct BlendState;

	bool initialize();
	void shutdown();

	bool is_spirv_supported();

	bool resize_swap_chain_framebuffer(unsigned int new_width, unsigned int new_height);
	void set_swap_chain_sync_interval(unsigned int sync_interval);
	void present_swap_chain_back_buffer();

	void push_debug_group(std::string_view name);
	void pop_debug_group();

	struct ScopedDebugGroup {
		ScopedDebugGroup(std::string_view name) { push_debug_group(name); }
		~ScopedDebugGroup() { pop_debug_group(); }
	};

	Handle<VertexShader> create_vertex_shader(ShaderDesc&& desc);
	// Pass an empty handle to unbind any currently bound vertex shader.
	void bind_vertex_shader(Handle<VertexShader> handle);

	Handle<FragmentShader> create_fragment_shader(ShaderDesc&& desc);
	// Pass an empty handle to unbind any currently bound fragment shader.
	void bind_fragment_shader(Handle<FragmentShader> handle);

	Handle<VertexInput> create_vertex_input(VertexInputDesc&& desc);
	void bind_vertex_input(Handle<VertexInput> handle);

	Handle<Buffer> create_buffer(BufferDesc&& desc);
	void recreate_buffer(Handle<Buffer> handle, unsigned int size, const void* initial_data = nullptr);
	void destroy_buffer(Handle<Buffer> handle);
	// Fails if the buffer is not dynamic, or if offset + size exceeds the buffer size.
	void update_buffer(Handle<Buffer> handle, const void* data, unsigned int size, unsigned int offset = 0);
	size_t get_buffer_size(Handle<Buffer> handle);
	// Pass an empty handle to unbind any currently bound buffer.
	void bind_vertex_buffer(unsigned int binding, Handle<Buffer> handle, unsigned int stride, unsigned int offset = 0);
	// Pass an empty handle to unbind any currently bound buffer.
	void bind_index_buffer(Handle<Buffer> handle);
	// Pass an empty handle to unbind any currently bound buffer.
	void bind_uniform_buffer(unsigned int binding, Handle<Buffer> handle);
	// Fails if offset + size exceeds the buffer size, or if offset
	// is not a multiple of get_uniform_buffer_offset_alignment().
	void bind_uniform_buffer_range(unsigned int binding, Handle<Buffer> handle, unsigned int size, unsigned offset = 0);

	Handle<Texture> create_texture(TextureDesc&& desc);
	Handle<Texture> load_texture(const std::string& path);
	Handle<Texture> copy_texture(Handle<Texture> src);
	void destroy_texture(Handle<Texture> handle);
	// Pass an empty handle to unbind any currently bound texture.
	void bind_texture(unsigned int binding, Handle<Texture> handle);
	void update_texture(Handle<Texture> handle, const unsigned char* data);
	void copy_texture(Handle<Texture> dest, Handle<Texture> src);
	void get_texture_size(Handle<Texture> handle, unsigned int& width, unsigned int& height);

	Handle<Sampler> create_sampler(SamplerDesc&& desc);
	void destroy_sampler(Handle<Sampler> handle);
	// Pass an empty handle to unbind any currently bound sampler.
	void bind_sampler(unsigned int binding, Handle<Sampler> handle);

	Handle<Framebuffer> get_swap_chain_back_buffer();
	Handle<Framebuffer> create_framebuffer(FramebufferDesc&& desc);
	void attach_framebuffer_texture(Handle<Framebuffer> framebuffer_handle, Handle<Texture> texture_handle);
	Handle<Texture> get_framebuffer_texture(Handle<Framebuffer> handle);
	// Resizes all textures attached to the framebuffer.
	void resize_framebuffer(Handle<Framebuffer> framebuffer_handle, unsigned int width, unsigned int height);
	void bind_framebuffer(Handle<Framebuffer> handle);
	void clear_framebuffer(Handle<Framebuffer> handle, const float color[4]);

	Handle<RasterizerState> create_rasterizer_state(RasterizerDesc&& desc);
	void bind_rasterizer_state(Handle<RasterizerState> handle);

	Handle<BlendState> create_blend_state(BlendDesc&& desc);
	void bind_blend_state(Handle<BlendState> handle);

	void set_primitives(Primitives primitives);
	void set_viewport(const Viewport& viewport);
	void get_viewport(Viewport& viewport);
	void set_scissor(const Rect& scissor);
	void get_scissor(Rect& scissor);
	void set_scissor_test_enabled(bool enable);
	bool get_scissor_test_enabled();

	void draw(unsigned int vertex_count, unsigned int vertex_offset = 0);
	void draw_indexed(unsigned int index_count);

	void show_texture_debug_window();
}