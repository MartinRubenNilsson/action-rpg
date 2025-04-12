#include "graphics_api.h"
#ifdef GRAPHICS_API_D3D11
#define NOMINMAX
#include <dxgi.h>
#include <d3d11_1.h>
#ifdef GRAPHICS_API_DEBUG
#include <d3dcommon.h> // for WKPDID_D3DDebugObjectName
#endif
#include <wrl/client.h> // for Microsoft::WRL::ComPtr
#include <string>

#pragma comment(lib, "dxgi")
#pragma comment(lib, "d3d11")
#ifdef GRAPHICS_API_DEBUG
#pragma comment(lib, "dxguid") // for WKPDID_D3DDebugObjectName
#endif

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

	IDXGISwapChain* _swap_chain = nullptr;
	ID3D11Device* _device = nullptr;
	ID3D11DeviceContext* _device_context = nullptr;
#ifdef GRAPHICS_API_DEBUG
	ID3DUserDefinedAnnotation* _annotation = nullptr;
#endif
	ID3D11RenderTargetView* _swap_chain_back_buffer_rtv = nullptr;

	template <class T>
	void _set_debug_name(T* object, std::string_view name) {
#ifdef GRAPHICS_API_DEBUG
		if (!object) return;
		if (name.empty()) return;
		object->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)name.size(), name.data());
#endif
	}

	bool initialize(const InitializeOptions& options) {
		HRESULT result = S_OK;
		{
			UINT create_device_flags = 0;
#ifdef GRAPHICS_API_DEBUG
			create_device_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
			D3D_FEATURE_LEVEL feature_levels[] = { D3D_FEATURE_LEVEL_11_0 };
			result = D3D11CreateDevice(
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
		}
		{
			Microsoft::WRL::ComPtr<IDXGIFactory> factory{};
			result = CreateDXGIFactory(IID_PPV_ARGS(&factory));
			if (FAILED(result)) {
				_output_debug_message("Failed to create DXGI factory");
				return false;
			}
			DXGI_SWAP_CHAIN_DESC swap_chain_desc{};
			swap_chain_desc.BufferCount = 2; // one front buffer, one back buffer
			swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
			swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swap_chain_desc.OutputWindow = (HWND)options.hwnd;
			swap_chain_desc.SampleDesc.Count = 1;
			swap_chain_desc.Windowed = TRUE;
			swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
			result = factory->CreateSwapChain(_device, &swap_chain_desc, &_swap_chain);
			if (FAILED(result)) {
				_output_debug_message("Failed to create swap chain");
				return false;
			}
		}
		{
			Microsoft::WRL::ComPtr<ID3D11Texture2D> back_buffer;
			result = _swap_chain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));
			if (FAILED(result)) {
				_output_debug_message("Failed to get swap chain back buffer");
				return false;
			}
			_set_debug_name(back_buffer.Get(), "BackBuffer");
			result = _device->CreateRenderTargetView(back_buffer.Get(), nullptr, &_swap_chain_back_buffer_rtv);
			if (FAILED(result)) {
				_output_debug_message("Failed to create swap chain back buffer RTV");
				return false;
			}
			_set_debug_name(_swap_chain_back_buffer_rtv, "BackBufferRTV");
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
		_safe_release(_swap_chain_back_buffer_rtv);
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

	VertexInputHandle create_vertex_input(const VertexInputDesc& desc) {
		D3D11_INPUT_ELEMENT_DESC input_element_descs[16] = {};
		//TODO
		return VertexInputHandle();
	}

	void destroy_vertex_input(VertexInputHandle vertex_input) {}
	void bind_vertex_input(VertexInputHandle vertex_input) {}

	ShaderHandle create_shader(const ShaderDesc& desc) { return ShaderHandle(); }
	void destroy_shader(ShaderHandle shader) {}
	void bind_shader(ShaderHandle shader) {}

	BufferHandle create_buffer(const BufferDesc& desc) {
		D3D11_BUFFER_DESC buffer_desc{};
		buffer_desc.ByteWidth = desc.size;
		buffer_desc.Usage = desc.dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE;
		if (desc.type == BufferType::VertexBuffer) {
			buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		} else if (desc.type == BufferType::IndexBuffer) {
			buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		} else if (desc.type == BufferType::UniformBuffer) {
			buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		}
		if (desc.dynamic) {
			buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		}
		HRESULT result = S_OK;
		ID3D11Buffer* buffer = nullptr;
		if (desc.initial_data) {
			D3D11_SUBRESOURCE_DATA initial_data{};
			initial_data.pSysMem = desc.initial_data;
			result = _device->CreateBuffer(&buffer_desc, &initial_data, &buffer);
		} else {
			result = _device->CreateBuffer(&buffer_desc, nullptr, &buffer);
		}
		if (FAILED(result)) {
			_output_debug_message("Failed to create buffer");
			return BufferHandle();
		}
		_set_debug_name(buffer, desc.debug_name);
		return BufferHandle{ .object = (uintptr_t)buffer };
	}

	void destroy_buffer(BufferHandle buffer) {
		if (!buffer.object) return;
		ID3D11Buffer* d3d11_buffer = (ID3D11Buffer*)buffer.object;
		d3d11_buffer->Release();
	}

	void update_buffer(BufferHandle buffer, const void* data, unsigned int size, unsigned int offset) {
		if (!buffer.object) return;
		ID3D11Buffer* d3d11_buffer = (ID3D11Buffer*)buffer.object;
		D3D11_MAPPED_SUBRESOURCE mapped_resource{};
		HRESULT result = _device_context->Map(d3d11_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
		if (FAILED(result)) {
			_output_debug_message("Failed to map buffer");
			return;
		}
		memcpy((void*)((uintptr_t)mapped_resource.pData + offset), data, size);
		_device_context->Unmap(d3d11_buffer, 0);
	}

	void bind_uniform_buffer(unsigned int binding, BufferHandle buffer) {}
	void bind_uniform_buffer_range(unsigned int binding, BufferHandle buffer, unsigned int size, unsigned int offset) {}
	void bind_vertex_buffer(VertexInputHandle vertex_input, unsigned int binding, BufferHandle buffer, unsigned int stride, unsigned int offset) {}
	void bind_index_buffer(VertexInputHandle vertex_input, BufferHandle buffer) {}

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

	FramebufferHandle get_default_framebuffer() {
		return FramebufferHandle{ .object = (uintptr_t)_swap_chain_back_buffer_rtv };
	}

	FramebufferHandle create_framebuffer(const FramebufferDesc& desc) { return FramebufferHandle(); }
	void destroy_framebuffer(FramebufferHandle framebuffer) {}
	bool attach_framebuffer_texture(FramebufferHandle framebuffer, TextureHandle texture) { return true; }

	void clear_framebuffer(FramebufferHandle framebuffer, const float color[4]) {
		if (!framebuffer.object) return;
		_device_context->ClearRenderTargetView((ID3D11RenderTargetView*)framebuffer.object, color);
	}

	void bind_framebuffer(FramebufferHandle framebuffer) {
		_device_context->OMSetRenderTargets(1, &_swap_chain_back_buffer_rtv, nullptr);
	}

	void set_viewports(const Viewport* viewports, unsigned int count) {
		if (count > D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT) {
			count = D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT;
		}
		D3D11_VIEWPORT d3d11_viewports[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
		for (unsigned int i = 0; i < count; ++i) {
			d3d11_viewports[i].TopLeftX = viewports[i].x;
			d3d11_viewports[i].TopLeftY = viewports[i].y;
			d3d11_viewports[i].Width = viewports[i].width;
			d3d11_viewports[i].Height = viewports[i].height;
			d3d11_viewports[i].MinDepth = 0.f;
			d3d11_viewports[i].MaxDepth = 1.f;
		}
		_device_context->RSSetViewports(count, d3d11_viewports);
	}

	D3D_PRIMITIVE_TOPOLOGY _primitives_to_d3d11_primitive_topology(Primitives primitives) {
		switch (primitives) {
		case Primitives::PointList:     return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
		case Primitives::LineList:      return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
		case Primitives::LineStrip:     return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
		case Primitives::TriangleList:  return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		case Primitives::TriangleStrip: return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		default:                        return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
		}
	}

	void set_primitives(Primitives primitives) {
		_device_context->IASetPrimitiveTopology(_primitives_to_d3d11_primitive_topology(primitives));
	}

	void set_scissors(const Rect* scissors, unsigned int count) {}
	void set_scissor_test_enabled(bool enable) {}

	void draw(unsigned int vertex_count, unsigned int vertex_offset) {
		//_device_context->Draw(vertex_count, vertex_offset);
	}

	void draw_indexed(unsigned int index_count) {
		//_device_context->DrawIndexed(index_count, 0, 0);
	}

} // namespace api
} // namespace graphics

#endif // GRAPHICS_API_D3D11