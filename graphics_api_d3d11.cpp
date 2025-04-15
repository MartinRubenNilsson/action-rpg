#include "graphics_api.h"
#ifdef GRAPHICS_API_D3D11
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <dxgi.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
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

	ID3D11Device* _device = nullptr;
	ID3D11DeviceContext1* _device_context = nullptr;
#ifdef GRAPHICS_API_DEBUG
	ID3DUserDefinedAnnotation* _user_defined_annotation = nullptr;
#endif
	IDXGISwapChain* _swap_chain = nullptr;
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
			Microsoft::WRL::ComPtr<ID3D11DeviceContext> device_context{};
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
				&device_context
			);
			if (FAILED(result)) {
				_output_debug_message("Failed to create D3D11 device and device context");
				return false;
			}
			result = device_context->QueryInterface(IID_PPV_ARGS(&_device_context));
			if (FAILED(result)) {
				_output_debug_message("Failed to query D3D11 device context 1");
				return false;
			}
#ifdef GRAPHICS_API_DEBUG
			result = device_context->QueryInterface(IID_PPV_ARGS(&_user_defined_annotation));
			if (FAILED(result)) {
				_output_debug_message("Failed to query D3D11 user defined annotation");
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
				_output_debug_message("Failed to create DXGI swap chain");
				return false;
			}
		}
		{
			Microsoft::WRL::ComPtr<ID3D11Texture2D> back_buffer;
			result = _swap_chain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));
			if (FAILED(result)) {
				_output_debug_message("Failed to get DXGI swap chain back buffer");
				return false;
			}
			_set_debug_name(back_buffer.Get(), "BackBuffer");
			result = _device->CreateRenderTargetView(back_buffer.Get(), nullptr, &_swap_chain_back_buffer_rtv);
			if (FAILED(result)) {
				_output_debug_message("Failed to create DXGI swap chain back buffer RTV");
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
		_safe_release(_user_defined_annotation);
#endif
		_safe_release(_device_context);
		_safe_release(_device);
	}

	bool is_spirv_supported() {
		return false;
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
		if (!_user_defined_annotation) return;
		wchar_t wide_name[256] = {};
		MultiByteToWideChar(CP_UTF8, 0, name.data(), (int)name.size(), wide_name, std::size(wide_name) - 1);
		_user_defined_annotation->BeginEvent(wide_name);
#endif
	}

	void pop_debug_group() {
#ifdef GRAPHICS_API_DEBUG
		if (!_user_defined_annotation) return;
		_user_defined_annotation->EndEvent();
#endif
	}

	bool _compile_shader(const ShaderDesc& desc, const char* target, ID3DBlob** shader_blob) {
		if (desc.code.empty()) {
			_output_debug_message("Shader code is empty: " + std::string(desc.debug_name));
			return false;
		}
		UINT flags = 0;
#ifdef GRAPHICS_API_DEBUG
		flags |= D3DCOMPILE_DEBUG;
		flags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
		Microsoft::WRL::ComPtr<ID3DBlob> error_blob{};
		HRESULT result = D3DCompile(
			desc.code.data(),
			desc.code.size(),
			nullptr,
			nullptr,
			D3D_COMPILE_STANDARD_FILE_INCLUDE,
			"main",
			target,
			flags,
			0,
			shader_blob,
			&error_blob
		);
		if (FAILED(result)) {
			if (error_blob) {
				_output_debug_message("Failed to compile shader: " + std::string(desc.debug_name));
				_output_debug_message((char*)error_blob->GetBufferPointer());
			}
			return false;
		}
		return true;
	}

#if 0
	bool _is_dxil_signed(std::span<const unsigned char> buffer) {
		//https://www.wihlidal.com/blog/pipeline/2018-09-16-dxil-signing-post-compile/
		struct DxilMinimalHeader {
			UINT32 four_cc;
			UINT32 hash_digest[4];
		};
		if (buffer.size() < sizeof(DxilMinimalHeader)) {
			return false;
		}
		const DxilMinimalHeader* header = (const DxilMinimalHeader*)buffer.data();
		bool has_digest = false;
		has_digest |= header->hash_digest[0] != 0x0;
		has_digest |= header->hash_digest[1] != 0x0;
		has_digest |= header->hash_digest[2] != 0x0;
		has_digest |= header->hash_digest[3] != 0x0;
		return has_digest;
	}
#endif

#define SHADER_MODEL "5_1"

	VertexShaderHandle create_vertex_shader(const ShaderDesc& desc) {
		if (desc.code.empty()) {
			_output_debug_message("Vertex shader code is empty: " + std::string(desc.debug_name));
			return VertexShaderHandle();
		}
		HRESULT result = S_OK;
		ID3D11VertexShader* shader = nullptr;
		if (desc.binary) { // DXBC
			result = _device->CreateVertexShader(
				desc.code.data(),
				desc.code.size(),
				nullptr,
				&shader
			);
		} else { // HLSL
			Microsoft::WRL::ComPtr<ID3DBlob> shader_blob{};
			if (!_compile_shader(desc, "vs_" SHADER_MODEL, &shader_blob)) {
				return VertexShaderHandle();
			}
			result = _device->CreateVertexShader(
				shader_blob->GetBufferPointer(),
				shader_blob->GetBufferSize(),
				nullptr,
				&shader
			);
		}
		if (FAILED(result)) {
			_output_debug_message("Failed to create vertex shader: " + std::string(desc.debug_name));
			return VertexShaderHandle();
		}
		_set_debug_name(shader, desc.debug_name);
		return VertexShaderHandle{ .object = (uintptr_t)shader };
	}

	void destroy_vertex_shader(VertexShaderHandle shader) {
		if (!shader.object) return;
		ID3D11VertexShader* d3d11_shader = (ID3D11VertexShader*)shader.object;
		d3d11_shader->Release();
	}

	void bind_vertex_shader(VertexShaderHandle shader) {
		ID3D11VertexShader* d3d11_shader = (ID3D11VertexShader*)shader.object;
		_device_context->VSSetShader(d3d11_shader, nullptr, 0);
	}

	FragmentShaderHandle create_fragment_shader(const ShaderDesc& desc) {
		if (desc.code.empty()) {
			_output_debug_message("Fragment shader code is empty: " + std::string(desc.debug_name));
			return FragmentShaderHandle();
		}
		HRESULT result = S_OK;
		ID3D11PixelShader* shader = nullptr;
		if (desc.binary) { // DXBC
			result = _device->CreatePixelShader(
				desc.code.data(),
				desc.code.size(),
				nullptr,
				&shader
			);
		} else { // HLSL
			Microsoft::WRL::ComPtr<ID3DBlob> shader_blob{};
			if (!_compile_shader(desc, "ps_" SHADER_MODEL, &shader_blob)) {
				return FragmentShaderHandle();
			}
			result = _device->CreatePixelShader(
				shader_blob->GetBufferPointer(),
				shader_blob->GetBufferSize(),
				nullptr,
				&shader
			);
		}
		if (FAILED(result)) {
			_output_debug_message("Failed to create fragment shader: " + std::string(desc.debug_name));
			return FragmentShaderHandle();
		}
		_set_debug_name(shader, desc.debug_name);
		return FragmentShaderHandle{ .object = (uintptr_t)shader };
	}

	void destroy_fragment_shader(FragmentShaderHandle shader) {
		if (!shader.object) return;
		ID3D11PixelShader* d3d11_shader = (ID3D11PixelShader*)shader.object;
		d3d11_shader->Release();
	}

	void bind_fragment_shader(FragmentShaderHandle shader) {
		ID3D11PixelShader* d3d11_shader = (ID3D11PixelShader*)shader.object;
		_device_context->PSSetShader(d3d11_shader, nullptr, 0);
	}

	DXGI_FORMAT _format_to_dxgi_format(Format format) {
		switch (format) {
		case Format::R8_UNORM:     return DXGI_FORMAT_R8_UNORM;
		case Format::RG8_UNORM:    return DXGI_FORMAT_R8G8_UNORM;
		case Format::RGB8_UNORM:   return DXGI_FORMAT_R8G8B8A8_UNORM;
		case Format::RGBA8_UNORM:  return DXGI_FORMAT_R8G8B8A8_UNORM;
		case Format::R32_FLOAT:    return DXGI_FORMAT_R32_FLOAT;
		case Format::RG32_FLOAT:   return DXGI_FORMAT_R32G32_FLOAT;
		case Format::RGB32_FLOAT:  return DXGI_FORMAT_R32G32B32_FLOAT;
		case Format::RGBA32_FLOAT: return DXGI_FORMAT_R32G32B32A32_FLOAT;
		default:                   return DXGI_FORMAT_UNKNOWN;
		}
	}

	VertexInputHandle create_vertex_input(const VertexInputDesc& desc) {
		if (desc.attributes.empty()) {
			_output_debug_message("Vertex input attributes are empty: " + std::string(desc.debug_name));
			return VertexInputHandle();
		}
		if (desc.attributes.size() > D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT) {
			_output_debug_message("Too many vertex input attributes: " + std::string(desc.debug_name));
			return VertexInputHandle();
		}
		if (desc.bytecode.empty()) {
			_output_debug_message("Vertex input bytecode is empty: " + std::string(desc.debug_name));
			return VertexInputHandle();
		}
		D3D11_INPUT_ELEMENT_DESC d3d11_input_elements[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
		const UINT d3d11_input_element_count = (UINT)desc.attributes.size();
		for (UINT i = 0; i < d3d11_input_element_count; ++i) {
			const VertexInputAttribDesc& attrib = desc.attributes[i];
			// These are the HLSL semantics outputted by SPIRV-Cross, which maps a GLSL
			// vertex input with location i to a HLSL vertex input with semantic TEXCOORDi.
			d3d11_input_elements[i].SemanticName = "TEXCOORD";
			d3d11_input_elements[i].SemanticIndex = i;
			d3d11_input_elements[i].Format = _format_to_dxgi_format(attrib.format);
			d3d11_input_elements[i].InputSlot = attrib.binding;
			d3d11_input_elements[i].AlignedByteOffset = attrib.offset;
			d3d11_input_elements[i].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			d3d11_input_elements[i].InstanceDataStepRate = 0;
		}
		ID3D11InputLayout* d3d11_input_layout = nullptr;
		HRESULT result = _device->CreateInputLayout(
			d3d11_input_elements,
			d3d11_input_element_count,
			desc.bytecode.data(),
			desc.bytecode.size(),
			&d3d11_input_layout
		);
		if (FAILED(result)) {
			_output_debug_message("Failed to create vertex input layout: " + std::string(desc.debug_name));
			return VertexInputHandle();
		}
		_set_debug_name(d3d11_input_layout, desc.debug_name);
		return VertexInputHandle{ .object = (uintptr_t)d3d11_input_layout };
	}

	void destroy_vertex_input(VertexInputHandle vertex_input) {
		if (!vertex_input.object) return;
		ID3D11InputLayout* d3d11_input_layout = (ID3D11InputLayout*)vertex_input.object;
		d3d11_input_layout->Release();
	}

	void bind_vertex_input(VertexInputHandle vertex_input) {
		ID3D11InputLayout* d3d11_input_layout = (ID3D11InputLayout*)vertex_input.object;
		_device_context->IASetInputLayout(d3d11_input_layout);
	}

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

	void bind_uniform_buffer(unsigned int binding, BufferHandle buffer) {
		ID3D11Buffer* d3d11_buffer = (ID3D11Buffer*)buffer.object;
		_device_context->VSSetConstantBuffers(binding, 1, &d3d11_buffer);
		_device_context->PSSetConstantBuffers(binding, 1, &d3d11_buffer);
	}

	void bind_uniform_buffer_range(unsigned int binding, BufferHandle buffer, unsigned int size, unsigned int offset) {
		ID3D11Buffer* d3d11_buffer = (ID3D11Buffer*)buffer.object;
		_device_context->VSSetConstantBuffers1(binding, 1, &d3d11_buffer, &offset, &size);
		_device_context->PSSetConstantBuffers1(binding, 1, &d3d11_buffer, &offset, &size);
	}

	void bind_vertex_buffer(unsigned int binding, BufferHandle buffer, unsigned int stride, unsigned int offset) {
		ID3D11Buffer* d3d11_buffer = (ID3D11Buffer*)buffer.object;
		const UINT strides[] = { stride };
		const UINT offsets[] = { offset };
		_device_context->IASetVertexBuffers(binding, 1, &d3d11_buffer, strides, offsets);
	}
	
	void bind_index_buffer(BufferHandle buffer, unsigned int offset) {
		ID3D11Buffer* d3d11_buffer = (ID3D11Buffer*)buffer.object;
		_device_context->IASetIndexBuffer(d3d11_buffer, DXGI_FORMAT_R32_UINT, offset);
	}

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

	D3D11_FILTER _filter_to_d3d11_filter(Filter filter) {
		switch (filter) {
		case Filter::Nearest:   return D3D11_FILTER_MIN_MAG_MIP_POINT;
		case Filter::Linear:    return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		//case Filter::Bilinear:  return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		//case Filter::Trilinear: return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		default:                return D3D11_FILTER_MIN_MAG_MIP_POINT;
		}
	}

	D3D11_TEXTURE_ADDRESS_MODE _wrap_to_d3d11_texture_address_mode(Wrap wrap) {
		switch (wrap) {
		case Wrap::Repeat:             return D3D11_TEXTURE_ADDRESS_WRAP;
		case Wrap::MirroredRepeat:     return D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
		case Wrap::ClampToEdge:        return D3D11_TEXTURE_ADDRESS_CLAMP;
		case Wrap::ClampToBorder:      return D3D11_TEXTURE_ADDRESS_BORDER;
		case Wrap::MirrorClampToEdge:  return D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
		default:                       return D3D11_TEXTURE_ADDRESS_WRAP;
		}
	}

	SamplerHandle create_sampler(const SamplerDesc& desc) {
		D3D11_SAMPLER_DESC sampler_desc{};
		sampler_desc.Filter = _filter_to_d3d11_filter(desc.filter);
		sampler_desc.AddressU = _wrap_to_d3d11_texture_address_mode(desc.wrap);
		sampler_desc.AddressV = _wrap_to_d3d11_texture_address_mode(desc.wrap);
		sampler_desc.AddressW = _wrap_to_d3d11_texture_address_mode(desc.wrap);
		sampler_desc.MinLOD = 0.f;
		sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;
		sampler_desc.MipLODBias = 0.f;
		sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		ID3D11SamplerState* sampler_state = nullptr;
		HRESULT result = _device->CreateSamplerState(&sampler_desc, &sampler_state);
		if (FAILED(result)) {
			_output_debug_message("Failed to create sampler state");
			return SamplerHandle();
		}
		_set_debug_name(sampler_state, desc.debug_name);
		return SamplerHandle{ .object = (uintptr_t)sampler_state };
	}

	void destroy_sampler(SamplerHandle sampler) {
		if (!sampler.object) return;
		ID3D11SamplerState* d3d11_sampler_state = (ID3D11SamplerState*)sampler.object;
		d3d11_sampler_state->Release();
	}

	void bind_sampler(unsigned int binding, SamplerHandle sampler) {
		ID3D11SamplerState* d3d11_sampler_state = (ID3D11SamplerState*)sampler.object;
		_device_context->PSSetSamplers(binding, 1, &d3d11_sampler_state);
		_device_context->VSSetSamplers(binding, 1, &d3d11_sampler_state);
	}

	FramebufferHandle get_default_framebuffer() {
		return FramebufferHandle{ .object = (uintptr_t)_swap_chain_back_buffer_rtv };
	}

	FramebufferHandle create_framebuffer(const FramebufferDesc& desc) { return FramebufferHandle(); }
	void destroy_framebuffer(FramebufferHandle framebuffer) {}
	bool attach_framebuffer_texture(FramebufferHandle framebuffer, TextureHandle texture) { return true; }

	void clear_framebuffer(FramebufferHandle framebuffer, const float color[4]) {
		if (!framebuffer.object) return;
		ID3D11RenderTargetView* d3d11_rtv = (ID3D11RenderTargetView*)framebuffer.object;
		_device_context->ClearRenderTargetView(d3d11_rtv, color);
	}

	void bind_framebuffer(FramebufferHandle framebuffer) {
		ID3D11RenderTargetView* d3d11_rtv = (ID3D11RenderTargetView*)framebuffer.object;
		_device_context->OMSetRenderTargets(1, &d3d11_rtv, nullptr);
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