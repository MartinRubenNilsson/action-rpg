#include "graphics_api.h"
#ifdef GRAPHICS_API_D3D11
#include <d3d11.h>

#pragma comment(lib, "d3d11.lib")

namespace graphics {
namespace api {

	void set_debug_message_callback(DebugMessageCallback callback) {}

	ID3D11Device* _device = nullptr;
	ID3D11DeviceContext* _device_context = nullptr;

	void initialize(const InitializeOptions& options) {
		// Initialize D3D11
		D3D_FEATURE_LEVEL feature_level = D3D_FEATURE_LEVEL_11_0;
		D3D11CreateDevice(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			0,
			&feature_level,
			1,
			D3D11_SDK_VERSION,
			&_device,
			nullptr,
			&_device_context
		);
	}

	template <class T>
	void _safe_release(T*& ptr) {
		if (ptr) {
			ptr->Release();
			ptr = nullptr;
		}
	}

	void shutdown() {
		_safe_release(_device_context);
		_safe_release(_device);
	}

#ifdef GRAPHICS_API_D3D11
	ID3D11Device* get_d3d11_device() {
		return _device;
	}
	ID3D11DeviceContext* get_d3d11_device_context() {
		return _device_context;
	}
#endif

	void push_debug_group(std::string_view name) {}
	void pop_debug_group() {}

	VertexArrayHandle create_vertex_array(const VertexArrayDesc& desc) { return VertexArrayHandle(); }
	void destroy_vertex_array(VertexArrayHandle vertex_array) {}
	void bind_vertex_array(VertexArrayHandle vertex_array) {}

	ShaderHandle create_shader(const ShaderDesc& desc) { return ShaderHandle(); }
	void destroy_shader(ShaderHandle shader) {}
	void bind_shader(ShaderHandle shader) {}

	BufferHandle create_buffer(const BufferDesc& desc) { return BufferHandle(); }
	void destroy_buffer(BufferHandle buffer) {}
	void update_buffer(BufferHandle buffer, const void* data, unsigned int size, unsigned int offset) {}
	void bind_uniform_buffer(unsigned int binding, BufferHandle buffer) {}
	void bind_uniform_buffer_range(unsigned int binding, BufferHandle buffer, unsigned int size, unsigned int offset) {}
	void bind_vertex_buffer(VertexArrayHandle vertex_array, unsigned int binding, BufferHandle buffer, unsigned int stride, unsigned int offset) {}
	void bind_index_buffer(VertexArrayHandle vertex_array, BufferHandle buffer) {}

	TextureHandle create_texture(const TextureDesc& desc) { return TextureHandle(); }
	void destroy_texture(TextureHandle texture) {}
	void update_texture(
		TextureHandle texture,
		unsigned int level,
		unsigned int x,
		unsigned int y,
		unsigned int width,
		unsigned int height,
		Format pixel_format,
		const void* pixels) {}
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
		unsigned int src_depth) {}
	void bind_texture(unsigned int binding, TextureHandle texture) {}

	SamplerHandle create_sampler(const SamplerDesc& desc) { return SamplerHandle(); }
	void destroy_sampler(SamplerHandle sampler) {}
	void bind_sampler(unsigned int binding, SamplerHandle sampler) {}

	FramebufferHandle create_framebuffer(const FramebufferDesc& desc) { return FramebufferHandle(); }
	void destroy_framebuffer(FramebufferHandle framebuffer) {}
	bool attach_framebuffer_texture(FramebufferHandle framebuffer, TextureHandle texture) { return true; }
	void clear_framebuffer(FramebufferHandle framebuffer, const float color[4]) {}
	void bind_framebuffer(FramebufferHandle framebuffer) {}

	void set_viewports(const Viewport* viewports, unsigned int count) {}
	void set_scissors(const Rect* scissors, unsigned int count) {}
	void set_scissor_test_enabled(bool enable) {}

	void draw(Primitives primitives, unsigned int vertex_count, unsigned int vertex_offset) {}
	void draw_indexed(Primitives primitives, unsigned int index_count) {}

} // namespace api
} // namespace graphics

#endif // GRAPHICS_API_D3D11