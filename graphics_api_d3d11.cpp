#include "graphics_api.h"
#ifdef GRAPHICS_API_D3D11
#include <dxgi.h>
#include <d3d11_1.h>
#include <wrl/client.h> // for Microsoft::WRL::ComPtr

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

namespace graphics {
namespace api {

	DebugMessageCallback _debug_message_callback = nullptr;

	void set_debug_message_callback(DebugMessageCallback callback) {
		_debug_message_callback = callback;
	}

	void _output_debug_message(std::string_view message) {
		if (_debug_message_callback) {
			_debug_message_callback(message);
		}
	}

	ID3D11Device* _device = nullptr;
	ID3D11DeviceContext* _device_context = nullptr;
#ifdef GRAPHICS_API_DEBUG
	ID3DUserDefinedAnnotation* _annotation = nullptr;
#endif
	IDXGISwapChain* _swap_chain = nullptr;

	template <class T>
	void _set_debug_name(T* object, std::string_view name) {
#ifdef GRAPHICS_API_DEBUG
		if (!object) return;
		if (name.empty()) return;
		object->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)name.size(), name.data());
#endif
	}

	bool initialize(const InitializeOptions& options) {
		UINT create_device_flags = 0;
#ifdef GRAPHICS_API_DEBUG
		create_device_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		D3D_FEATURE_LEVEL feature_levels[] = { D3D_FEATURE_LEVEL_11_0 };
		HRESULT result = D3D11CreateDevice(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			create_device_flags,
			feature_levels,
			std::size(feature_levels),
			D3D11_SDK_VERSION,
			&_device,
			nullptr,
			&_device_context
		);
		if (FAILED(result)) {
			_output_debug_message("Failed to create D3D11 device");
			return false;
		}
#ifdef GRAPHICS_API_DEBUG
		result = _device_context->QueryInterface(IID_PPV_ARGS(&_annotation));
		if (FAILED(result)) {
			_output_debug_message("Failed to create D3D11 user defined annotation");
			return false;
		}
#endif
		Microsoft::WRL::ComPtr<IDXGIFactory> factory{};
		result = CreateDXGIFactory(IID_PPV_ARGS(&factory));
		if (FAILED(result)) {
			_output_debug_message("Failed to create DXGI factory");
			return false;
		}
		DXGI_SWAP_CHAIN_DESC swap_chain_desc = {};
		swap_chain_desc.BufferCount = 2;
		swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swap_chain_desc.OutputWindow = (HWND)options.hwnd;
		swap_chain_desc.SampleDesc.Count = 1;
		swap_chain_desc.Windowed = TRUE;
		result = factory->CreateSwapChain(_device, &swap_chain_desc, &_swap_chain);
		if (FAILED(result)) {
			_output_debug_message("Failed to create swap chain");
			return false;
		}
		return true;
	}

	template <class T>
	void _safe_release(T*& ptr) {
		if (!ptr) return;
		ptr->Release();
		ptr = nullptr;
	}

	void shutdown() {
		_safe_release(_swap_chain);
#ifdef GRAPHICS_API_DEBUG
		_safe_release(_annotation);
#endif
		_safe_release(_device_context);
		_safe_release(_device);
	}

	ID3D11Device* get_d3d11_device() {
		return _device;
	}
	ID3D11DeviceContext* get_d3d11_device_context() {
		return _device_context;
	}

	void swap_buffers() {
		if (!_swap_chain) return;
		_swap_chain->Present(0, 0);
	}

	void push_debug_group(std::string_view name) {
#ifdef GRAPHICS_API_DEBUG
		if (!_annotation) return;
		wchar_t wide_name[256] = {};
		MultiByteToWideChar(CP_UTF8, 0, name.data(), (int)name.size(), wide_name, std::size(wide_name) - 1);
		_annotation->BeginEvent(wide_name);
#endif
	}

	void pop_debug_group() {
#ifdef GRAPHICS_API_DEBUG
		if (!_annotation) return;
		_annotation->EndEvent();
#endif
	}

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