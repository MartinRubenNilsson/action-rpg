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

	struct FramebufferImpl {
		ID3D11RenderTargetView* rtvs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = {};
		ID3D11DepthStencilView* dsv = nullptr;
	};

	ID3D11Device* _device = nullptr;
	ID3D11DeviceContext1* _device_context = nullptr;
#ifdef GRAPHICS_API_DEBUG
	ID3D11Debug* _debug = nullptr;
	ID3D11InfoQueue* _info_queue = nullptr;
	ID3DUserDefinedAnnotation* _user_defined_annotation = nullptr;
#endif
	IDXGISwapChain* _swap_chain = nullptr;
	ID3D11RenderTargetView* _swap_chain_back_buffer_rtv = nullptr;
	FramebufferImpl _default_framebuffer_impl = {}; // rtvs[0] = _swap_chain_back_buffer_rtv

	template <class T>
	void _set_debug_name(T* object, std::string_view name) {
#ifdef GRAPHICS_API_DEBUG
		if (!object) return;
		if (name.empty()) return;
		object->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)name.size(), name.data());
#endif
	}

	template <class T>
	std::string _get_debug_name(T* object) {
#ifdef GRAPHICS_API_DEBUG
		std::string name;
		if (!object) return name;
		UINT size = 0;
		object->GetPrivateData(WKPDID_D3DDebugObjectName, &size, nullptr);
		name.resize(size);
		object->GetPrivateData(WKPDID_D3DDebugObjectName, &size, name.data());
		return name;
#endif
	}

	bool _initialize_swap_chain_back_buffer_rtv_and_default_framebuffer_impl() {
		Microsoft::WRL::ComPtr<ID3D11Texture2D> back_buffer_texture;
		HRESULT result = _swap_chain->GetBuffer(0, IID_PPV_ARGS(&back_buffer_texture));
		if (FAILED(result)) {
			_output_debug_message("Failed to get swap chain back buffer");
			return false;
		}
		_set_debug_name(back_buffer_texture.Get(), "swap chain back buffer texture");
		result = _device->CreateRenderTargetView(back_buffer_texture.Get(), nullptr, &_swap_chain_back_buffer_rtv);
		if (FAILED(result)) {
			_output_debug_message("Failed to create swap chain back buffer RTV");
			return false;
		}
		_set_debug_name(_swap_chain_back_buffer_rtv, "swap chain back buffer rtv");
		_default_framebuffer_impl.rtvs[0] = _swap_chain_back_buffer_rtv;
		return true;
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
			result = _device->QueryInterface(IID_PPV_ARGS(&_debug));
			if (FAILED(result)) {
				_output_debug_message("Failed to query D3D11 debug device");
				return false;
			}
			result = _debug->QueryInterface(IID_PPV_ARGS(&_info_queue));
			if (FAILED(result)) {
				_output_debug_message("Failed to query D3D11 info queue");
				return false;
			}
			_info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
			_info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
			_info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, TRUE);
			_info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_INFO, TRUE);
			_info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_MESSAGE, TRUE);
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
			swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swap_chain_desc.OutputWindow = (HWND)options.hwnd;
			swap_chain_desc.SampleDesc.Count = 1;
			swap_chain_desc.Windowed = TRUE;
			swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			result = factory->CreateSwapChain(_device, &swap_chain_desc, &_swap_chain);
			if (FAILED(result)) {
				_output_debug_message("Failed to create DXGI swap chain");
				return false;
			}
		}
		if (!_initialize_swap_chain_back_buffer_rtv_and_default_framebuffer_impl()) {
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
		_default_framebuffer_impl = {};
		_safe_release(_swap_chain_back_buffer_rtv);
		_safe_release(_swap_chain);
#ifdef GRAPHICS_API_DEBUG
		_safe_release(_user_defined_annotation);
#endif
		if (_device_context) {
			_device_context->ClearState();
			_device_context->Flush();
		}
		_safe_release(_device_context);
		_safe_release(_device);
#ifdef GRAPHICS_API_DEBUG
		if (_info_queue) {
			_info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, FALSE);
			_info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, FALSE);
			_info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, FALSE);
			_info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_INFO, FALSE);
			_info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_MESSAGE, FALSE);
		}
		_safe_release(_info_queue);
		if (_debug) {
			_debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL | D3D11_RLDO_IGNORE_INTERNAL);
		}
		_safe_release(_debug);
#endif
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

	bool resize_swap_chain_framebuffer(unsigned int new_width, unsigned int new_height) {
		if (new_width == 0 || new_height == 0) {
			_output_debug_message("Invalid swap chain width or height");
			return false;
		}
		// Need to release all outstanding references to the swap chain's back buffer(s) before resizing:
		// https://learn.microsoft.com/en-us/windows/win32/api/dxgi/nf-dxgi-idxgiswapchain-resizebuffers#remarks
		_default_framebuffer_impl = {};
		_safe_release(_swap_chain_back_buffer_rtv);
		_device_context->OMSetRenderTargets(0, nullptr, nullptr); // In case the back buffer is bound as RTV
		// Resize the swap chain buffers.
		HRESULT result = _swap_chain->ResizeBuffers(
			0, // keep the number of buffers as-is
			new_width,
			new_height,
			DXGI_FORMAT_UNKNOWN, // keep the format as-is
			0
		);
		if (FAILED(result)) {
			_output_debug_message("Failed to resize swap chain buffers");
			return false;
		}
		_initialize_swap_chain_back_buffer_rtv_and_default_framebuffer_impl();
		return true;
	}

	void present_swap_chain_back_buffer() {
		_swap_chain->Present(0, 0);
	}

	void push_debug_group(std::string_view name) {
#ifdef GRAPHICS_API_DEBUG
		wchar_t wide_name[256] = {};
		MultiByteToWideChar(CP_UTF8, 0, name.data(), (int)name.size(), wide_name, std::size(wide_name) - 1);
		_user_defined_annotation->BeginEvent(wide_name);
#endif
	}

	void pop_debug_group() {
#ifdef GRAPHICS_API_DEBUG
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
	bool _is_dxil_signed(std::span<const unsigned char> d3d11_buffer) {
		//https://www.wihlidal.com/blog/pipeline/2018-09-16-dxil-signing-post-compile/
		struct DxilMinimalHeader {
			UINT32 four_cc;
			UINT32 hash_digest[4];
		};
		if (d3d11_buffer.size() < sizeof(DxilMinimalHeader)) {
			return false;
		}
		const DxilMinimalHeader* header = (const DxilMinimalHeader*)d3d11_buffer.data();
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
		// SIC: Allow binding a null shader to unbind the current shader.
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
		// SIC: Allow binding a null shader to unbind the current shader.
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
		if (std::empty(desc.attributes)) {
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
			const VertexInputAttribDesc& attrib = *(desc.attributes.begin() + i);
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
		// SIC: Allow binding a null vertex input to unbind the current vertex input.
		ID3D11InputLayout* d3d11_input_layout = (ID3D11InputLayout*)vertex_input.object;
		_device_context->IASetInputLayout(d3d11_input_layout);
	}

	BufferHandle create_buffer(const BufferDesc& desc) {
		D3D11_BUFFER_DESC d3d11_buffer_desc{};
		d3d11_buffer_desc.ByteWidth = desc.size;
		d3d11_buffer_desc.Usage = desc.dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE;
		if (desc.type == BufferType::VertexBuffer) {
			d3d11_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		} else if (desc.type == BufferType::IndexBuffer) {
			d3d11_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		} else if (desc.type == BufferType::UniformBuffer) {
			d3d11_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		}
		if (desc.dynamic) {
			d3d11_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		}
		HRESULT result = S_OK;
		ID3D11Buffer* d3d11_buffer = nullptr;
		if (desc.initial_data) {
			D3D11_SUBRESOURCE_DATA d3d11_initial_data{};
			d3d11_initial_data.pSysMem = desc.initial_data;
			result = _device->CreateBuffer(&d3d11_buffer_desc, &d3d11_initial_data, &d3d11_buffer);
		} else {
			result = _device->CreateBuffer(&d3d11_buffer_desc, nullptr, &d3d11_buffer);
		}
		if (FAILED(result)) {
			_output_debug_message("Failed to create buffer");
			return BufferHandle();
		}
		_set_debug_name(d3d11_buffer, desc.debug_name);
		return BufferHandle{ .object = (uintptr_t)d3d11_buffer };
	}

	void destroy_buffer(BufferHandle buffer) {
		if (!buffer.object) return;
		ID3D11Buffer* d3d11_buffer = (ID3D11Buffer*)buffer.object;
		d3d11_buffer->Release();
	}

	void update_buffer(BufferHandle buffer, const void* data, unsigned int size, unsigned int offset) {
		if (!buffer.object) return;
		ID3D11Buffer* d3d11_buffer = (ID3D11Buffer*)buffer.object;
		D3D11_MAPPED_SUBRESOURCE d3d11_mapped_subresource{};
		HRESULT result = _device_context->Map(d3d11_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3d11_mapped_subresource);
		if (FAILED(result)) {
			_output_debug_message("Failed to map buffer");
			return;
		}
		memcpy((void*)((uintptr_t)d3d11_mapped_subresource.pData + offset), data, size);
		_device_context->Unmap(d3d11_buffer, 0);
	}

	void bind_uniform_buffer(unsigned int binding, BufferHandle buffer) {
		// SIC: Allow binding a null buffer to unbind the current buffer.
		ID3D11Buffer* d3d11_buffer = (ID3D11Buffer*)buffer.object;
		_device_context->VSSetConstantBuffers(binding, 1, &d3d11_buffer);
		_device_context->PSSetConstantBuffers(binding, 1, &d3d11_buffer);
	}

	void bind_uniform_buffer_range(unsigned int binding, BufferHandle buffer, unsigned int size, unsigned int offset) {
		// SIC: Allow binding a null buffer to unbind the current buffer.
		ID3D11Buffer* d3d11_buffer = (ID3D11Buffer*)buffer.object;
		// PITFALL: In D3D11, the XSSetConstantBuffers1() family of functions expects
		// the offsets and sizes to be counted in "shader constants", which are 16 bytes
		// (4*32-bit components), e.g. float4s. This is different from OpenGL, where
		// the offsets and sizes are counted in bytes. Compare:
		// https://learn.microsoft.com/en-us/windows/win32/api/d3d11_1/nf-d3d11_1-id3d11devicecontext1-vssetconstantbuffers1
		// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glBindBufferRange.xhtml
		size /= 16; // 16 bytes per float4
		offset /= 16; // 16 bytes per float4
		_device_context->VSSetConstantBuffers1(binding, 1, &d3d11_buffer, &offset, &size);
		_device_context->PSSetConstantBuffers1(binding, 1, &d3d11_buffer, &offset, &size);
	}

	void bind_vertex_buffer(unsigned int binding, BufferHandle buffer, unsigned int stride, unsigned int offset) {
		// SIC: Allow binding a null buffer to unbind the current buffer.
		ID3D11Buffer* d3d11_buffer = (ID3D11Buffer*)buffer.object;
		const UINT strides[] = { stride };
		const UINT offsets[] = { offset };
		_device_context->IASetVertexBuffers(binding, 1, &d3d11_buffer, strides, offsets);
	}
	
	void bind_index_buffer(BufferHandle buffer, unsigned int offset) {
		// SIC: Allow binding a null buffer to unbind the current buffer.
		ID3D11Buffer* d3d11_buffer = (ID3D11Buffer*)buffer.object;
		_device_context->IASetIndexBuffer(d3d11_buffer, DXGI_FORMAT_R32_UINT, offset);
	}

	UINT _format_to_byte_width(Format format) {
		switch (format) {
		case Format::R8_UNORM:     return 1;
		case Format::RG8_UNORM:    return 2;
		case Format::RGB8_UNORM:   return 3;
		case Format::RGBA8_UNORM:  return 4;
		case Format::R32_FLOAT:    return 4;
		case Format::RG32_FLOAT:   return 8;
		case Format::RGB32_FLOAT:  return 12;
		case Format::RGBA32_FLOAT: return 16;
		default:                   return 0;
		}
	}

	TextureHandle create_texture(const TextureDesc& desc) {
		D3D11_TEXTURE2D_DESC d3d11_texture2d_desc{};
		d3d11_texture2d_desc.Width = desc.width;
		d3d11_texture2d_desc.Height = desc.height;
		d3d11_texture2d_desc.MipLevels = 1;
		d3d11_texture2d_desc.ArraySize = 1;
		d3d11_texture2d_desc.Format = _format_to_dxgi_format(desc.format);
		d3d11_texture2d_desc.SampleDesc.Count = 1;
		d3d11_texture2d_desc.Usage = D3D11_USAGE_DEFAULT; // D3D11_USAGE_IMMUTABLE?
		d3d11_texture2d_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		if (desc.framebuffer_color) {
			d3d11_texture2d_desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
		}
		HRESULT result = S_OK;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> d3d11_texture2d{};
		if (desc.initial_data) {
			D3D11_SUBRESOURCE_DATA d3d11_initial_data{};
			d3d11_initial_data.pSysMem = desc.initial_data;
			d3d11_initial_data.SysMemPitch = desc.width * _format_to_byte_width(desc.format);
			result = _device->CreateTexture2D(&d3d11_texture2d_desc, &d3d11_initial_data, &d3d11_texture2d);
		} else {
			result = _device->CreateTexture2D(&d3d11_texture2d_desc, nullptr, &d3d11_texture2d);
		}
		if (FAILED(result)) {
			_output_debug_message("Failed to create texture: " + std::string(desc.debug_name));
			return TextureHandle();
		}
		_set_debug_name(d3d11_texture2d.Get(), desc.debug_name);
		D3D11_SHADER_RESOURCE_VIEW_DESC d3d11_srv_desc{};
		d3d11_srv_desc.Format = d3d11_texture2d_desc.Format;
		d3d11_srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		d3d11_srv_desc.Texture2D.MostDetailedMip = 0;
		d3d11_srv_desc.Texture2D.MipLevels = 1;
		ID3D11ShaderResourceView* d3d11_srv = nullptr;
		result = _device->CreateShaderResourceView(d3d11_texture2d.Get(), &d3d11_srv_desc, &d3d11_srv);
		if (FAILED(result)) {
			_output_debug_message("Failed to create shader resource view: " + std::string(desc.debug_name));
			return TextureHandle();
		}
		_set_debug_name(d3d11_srv, desc.debug_name);
		return TextureHandle{ .object = (uintptr_t)d3d11_srv };
	}

	void destroy_texture(TextureHandle texture) {
		if (!texture.object) return;
		ID3D11ShaderResourceView* d3d11_srv = (ID3D11ShaderResourceView*)texture.object;
		d3d11_srv->Release();
	}

	void update_texture(
		TextureHandle texture,
		unsigned int level,
		unsigned int x,
		unsigned int y,
		unsigned int width,
		unsigned int height,
		Format pixel_format,
		const void* pixels
	) {
		if (!texture.object) return;
		ID3D11ShaderResourceView* d3d11_srv = (ID3D11ShaderResourceView*)texture.object;
		Microsoft::WRL::ComPtr<ID3D11Resource> d3d11_resource{};
		d3d11_srv->GetResource(&d3d11_resource);
		D3D11_BOX d3d11_box{};
		d3d11_box.left = x;
		d3d11_box.top = y;
		d3d11_box.front = 0;
		d3d11_box.right = x + width;
		d3d11_box.bottom = y + height;
		d3d11_box.back = 1;
		_device_context->UpdateSubresource(
			d3d11_resource.Get(),
			level,
			&d3d11_box,
			pixels,
			width * _format_to_byte_width(pixel_format),
			0
		);
	}

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
		unsigned int src_depth
	) {
		if (!dst_texture.object) return;
		if (!src_texture.object) return;
		ID3D11ShaderResourceView* d3d11_dst_srv = (ID3D11ShaderResourceView*)dst_texture.object;
		ID3D11ShaderResourceView* d3d11_src_srv = (ID3D11ShaderResourceView*)src_texture.object;
		Microsoft::WRL::ComPtr<ID3D11Resource> d3d11_dst_resource;
		Microsoft::WRL::ComPtr<ID3D11Resource> d3d11_src_resource;
		d3d11_dst_srv->GetResource(&d3d11_dst_resource);
		d3d11_src_srv->GetResource(&d3d11_src_resource);
		D3D11_BOX d3d11_src_box{};
		d3d11_src_box.left = src_x;
		d3d11_src_box.top = src_y;
		d3d11_src_box.front = src_z;
		d3d11_src_box.right = src_x + src_width;
		d3d11_src_box.bottom = src_y + src_height;
		d3d11_src_box.back = src_z + src_depth;
		_device_context->CopySubresourceRegion(
			d3d11_dst_resource.Get(),
			dst_level,
			dst_x,
			dst_y,
			dst_z,
			d3d11_src_resource.Get(),
			src_level,
			&d3d11_src_box
		);
	}

	void bind_texture(unsigned int binding, TextureHandle texture) {
		// SIC: Allow binding a null texture to unbind the current texture.
		ID3D11ShaderResourceView* d3d11_srv = (ID3D11ShaderResourceView*)texture.object;
		_device_context->PSSetShaderResources(binding, 1, &d3d11_srv);
	}

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
		case Wrap::MirroredRepeat:     return D3D11_TEXTURE_ADDRESS_MIRROR;
		case Wrap::ClampToEdge:        return D3D11_TEXTURE_ADDRESS_CLAMP;
		case Wrap::ClampToBorder:      return D3D11_TEXTURE_ADDRESS_BORDER;
		case Wrap::MirrorClampToEdge:  return D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
		default:                       return D3D11_TEXTURE_ADDRESS_WRAP;
		}
	}

	SamplerHandle create_sampler(const SamplerDesc& desc) {
		D3D11_SAMPLER_DESC d3d11_sampler_desc{};
		d3d11_sampler_desc.Filter = _filter_to_d3d11_filter(desc.filter);
		d3d11_sampler_desc.AddressU = _wrap_to_d3d11_texture_address_mode(desc.wrap);
		d3d11_sampler_desc.AddressV = _wrap_to_d3d11_texture_address_mode(desc.wrap);
		d3d11_sampler_desc.AddressW = _wrap_to_d3d11_texture_address_mode(desc.wrap);
		d3d11_sampler_desc.MinLOD = 0.f;
		d3d11_sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;
		d3d11_sampler_desc.MipLODBias = 0.f;
		d3d11_sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		ID3D11SamplerState* d3d11_sampler_state = nullptr;
		HRESULT result = _device->CreateSamplerState(&d3d11_sampler_desc, &d3d11_sampler_state);
		if (FAILED(result)) {
			_output_debug_message("Failed to create sampler state");
			return SamplerHandle();
		}
		_set_debug_name(d3d11_sampler_state, desc.debug_name);
		return SamplerHandle{ .object = (uintptr_t)d3d11_sampler_state };
	}

	void destroy_sampler(SamplerHandle sampler) {
		if (!sampler.object) return;
		ID3D11SamplerState* d3d11_sampler_state = (ID3D11SamplerState*)sampler.object;
		d3d11_sampler_state->Release();
	}

	void bind_sampler(unsigned int binding, SamplerHandle sampler) {
		// SIC: Allow binding a null sampler to unbind the current sampler.
		ID3D11SamplerState* d3d11_sampler_state = (ID3D11SamplerState*)sampler.object;
		_device_context->PSSetSamplers(binding, 1, &d3d11_sampler_state);
		_device_context->VSSetSamplers(binding, 1, &d3d11_sampler_state);
	}

	FramebufferHandle get_swap_chain_back_buffer() {
		return FramebufferHandle{ .object = (uintptr_t)&_default_framebuffer_impl };
	}

	FramebufferHandle create_framebuffer(const FramebufferDesc& desc) {
		return FramebufferHandle{ .object = (uintptr_t)new FramebufferImpl() };
	}

	void destroy_framebuffer(FramebufferHandle framebuffer) {
		if (!framebuffer.object) return;
		FramebufferImpl* framebuffer_impl = (FramebufferImpl*)framebuffer.object;
		if (framebuffer_impl == &_default_framebuffer_impl) {
			_output_debug_message("Cannot destroy default framebuffer");
			return;
		}
		for (unsigned int i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i) {
			_safe_release(framebuffer_impl->rtvs[i]);
		}
		_safe_release(framebuffer_impl->dsv);
		delete framebuffer_impl;
	}

	bool attach_framebuffer_color_texture(FramebufferHandle framebuffer, unsigned int buffer, TextureHandle texture) {
		if (!framebuffer.object) return false;
		if (buffer >= D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT) return false;
		if (!texture.object) return false;
		FramebufferImpl* framebuffer_impl = (FramebufferImpl*)framebuffer.object;
		if (framebuffer_impl == &_default_framebuffer_impl) {
			_output_debug_message("Cannot attach color texture to default framebuffer");
			return false;
		}
		ID3D11ShaderResourceView* d3d11_srv = (ID3D11ShaderResourceView*)texture.object;
		Microsoft::WRL::ComPtr<ID3D11Resource> d3d11_resource{};
		d3d11_srv->GetResource(&d3d11_resource);
		Microsoft::WRL::ComPtr<ID3D11Texture2D> d3d11_texture_2d{};
		if (FAILED(d3d11_resource->QueryInterface(IID_PPV_ARGS(&d3d11_texture_2d)))) {
			_output_debug_message("Failed to query texture from shader resource view");
			return false;
		}
		D3D11_TEXTURE2D_DESC d3d11_texture2d_desc{};
		d3d11_texture_2d->GetDesc(&d3d11_texture2d_desc);
		if (!(d3d11_texture2d_desc.BindFlags & D3D11_BIND_RENDER_TARGET)) {
			_output_debug_message("Texture cannot be used as render target");
			return false;
		}
		D3D11_RENDER_TARGET_VIEW_DESC d3d11_rtv_desc{};
		d3d11_rtv_desc.Format = d3d11_texture2d_desc.Format;
		d3d11_rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		d3d11_rtv_desc.Texture2D.MipSlice = 0;
		ID3D11RenderTargetView* d3d11_rtv = nullptr;
		HRESULT result = _device->CreateRenderTargetView(d3d11_texture_2d.Get(), &d3d11_rtv_desc, &d3d11_rtv);
		if (FAILED(result)) {
			_output_debug_message("Failed to create render target view");
			return false;
		}
		_set_debug_name(d3d11_rtv, _get_debug_name(d3d11_texture_2d.Get()) + " rtv");
		_safe_release(framebuffer_impl->rtvs[buffer]);
		framebuffer_impl->rtvs[buffer] = d3d11_rtv;
		return true;
	}

	void clear_framebuffer_color(FramebufferHandle framebuffer, unsigned int buffer, const float color[4]) {
		if (!framebuffer.object) return;
		if (buffer >= D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT) return;
		FramebufferImpl* framebuffer_impl = (FramebufferImpl*)framebuffer.object;
		ID3D11RenderTargetView* d3d11_rtv = framebuffer_impl->rtvs[buffer];
		if (!d3d11_rtv) return;
		_device_context->ClearRenderTargetView(d3d11_rtv, color);
	}

	void bind_framebuffer(FramebufferHandle framebuffer) {
		if (!framebuffer.object) {
			_device_context->OMSetRenderTargets(0, nullptr, nullptr);
			return;
		}
		FramebufferImpl* framebuffer_impl = (FramebufferImpl*)framebuffer.object;
		_device_context->OMSetRenderTargets(
			D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT,
			framebuffer_impl->rtvs,
			framebuffer_impl->dsv);
	}

	D3D11_FILL_MODE _polygon_mode_to_d3d11_fill_mode(PolygonMode polygon_mode) {
		switch (polygon_mode) {
		case PolygonMode::Fill: return D3D11_FILL_SOLID;
		case PolygonMode::Line: return D3D11_FILL_WIREFRAME;
		default:                return D3D11_FILL_SOLID;
		}
	}

	D3D11_CULL_MODE _cull_mode_to_d3d11_cull_mode(CullMode cull_mode) {
		switch (cull_mode) {
		case CullMode::None:  return D3D11_CULL_NONE;
		case CullMode::Front: return D3D11_CULL_FRONT;
		case CullMode::Back:  return D3D11_CULL_BACK;
		default:              return D3D11_CULL_NONE;
		}
	}

	RasterizerStateHandle create_rasterizer_state(const RasterizerDesc& desc) {
		D3D11_RASTERIZER_DESC d3d11_rasterizer_desc = CD3D11_RASTERIZER_DESC(D3D11_DEFAULT);
		d3d11_rasterizer_desc.FillMode = _polygon_mode_to_d3d11_fill_mode(desc.polygon_mode);
		d3d11_rasterizer_desc.CullMode = _cull_mode_to_d3d11_cull_mode(desc.cull_mode);
		d3d11_rasterizer_desc.FrontCounterClockwise = desc.front_face_ccw ? TRUE : FALSE;
		ID3D11RasterizerState* d3d11_rasterizer_state = nullptr;
		HRESULT result = _device->CreateRasterizerState(&d3d11_rasterizer_desc, &d3d11_rasterizer_state);
		if (FAILED(result)) {
			_output_debug_message("Failed to create rasterizer state: " + std::string(desc.debug_name));
			return RasterizerStateHandle();
		}
		_set_debug_name(d3d11_rasterizer_state, desc.debug_name);
		return RasterizerStateHandle{ .object = (uintptr_t)d3d11_rasterizer_state };
	}

	void destroy_rasterizer_state(RasterizerStateHandle state) {
		if (!state.object) return;
		ID3D11RasterizerState* d3d11_rasterizer_state = (ID3D11RasterizerState*)state.object;
		d3d11_rasterizer_state->Release();
	}

	void bind_rasterizer_state(RasterizerStateHandle state) {
		// SIC: Allow binding a null state to unbind the current state.
		ID3D11RasterizerState* d3d11_rasterizer_state = (ID3D11RasterizerState*)state.object;
		_device_context->RSSetState(d3d11_rasterizer_state);
	}

	D3D11_BLEND _blend_factor_to_d3d11_blend(BlendFactor blend_factor) {
		switch (blend_factor) {
		case BlendFactor::Zero:             return D3D11_BLEND_ZERO;
		case BlendFactor::One:              return D3D11_BLEND_ONE;
		case BlendFactor::SrcColor:         return D3D11_BLEND_SRC_COLOR;
		case BlendFactor::OneMinusSrcColor: return D3D11_BLEND_INV_SRC_COLOR;
		case BlendFactor::SrcAlpha:         return D3D11_BLEND_SRC_ALPHA;
		case BlendFactor::OneMinusSrcAlpha: return D3D11_BLEND_INV_SRC_ALPHA;
		case BlendFactor::DstColor:         return D3D11_BLEND_DEST_COLOR;
		case BlendFactor::OneMinusDstColor: return D3D11_BLEND_INV_DEST_COLOR;
		case BlendFactor::DstAlpha:         return D3D11_BLEND_DEST_ALPHA;
		case BlendFactor::OneMinusDstAlpha: return D3D11_BLEND_INV_DEST_ALPHA;
		default:                            return D3D11_BLEND_ZERO;
		}
	}

	D3D11_BLEND_OP _blend_op_to_d3d11_blend_op(BlendOp blend_op) {
		switch (blend_op) {
		case BlendOp::Add:             return D3D11_BLEND_OP_ADD;
		case BlendOp::Subtract:        return D3D11_BLEND_OP_SUBTRACT;
		case BlendOp::ReverseSubtract: return D3D11_BLEND_OP_REV_SUBTRACT;
		case BlendOp::Min:             return D3D11_BLEND_OP_MIN;
		case BlendOp::Max:             return D3D11_BLEND_OP_MAX;
		default:                       return D3D11_BLEND_OP_ADD;
		}
	}

	BlendStateHandle create_blend_state(const BlendDesc& desc) {
		if (desc.attachments.size() > D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT) {
			_output_debug_message("Too many blend state buffers: " + std::string(desc.debug_name));
			return BlendStateHandle();
		}
		D3D11_BLEND_DESC d3d11_blend_desc = CD3D11_BLEND_DESC(D3D11_DEFAULT);
		for (unsigned int i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i) {
			const AttachmentBlendDesc& attachment_blend_desc = *(desc.attachments.begin() + i);
			if (attachment_blend_desc.blend_enable && i > 0) {
				d3d11_blend_desc.IndependentBlendEnable = TRUE;
			}
			D3D11_RENDER_TARGET_BLEND_DESC& d3d11_rt_blend_desc = d3d11_blend_desc.RenderTarget[i];
			d3d11_rt_blend_desc.BlendEnable = attachment_blend_desc.blend_enable ? TRUE : FALSE;
			d3d11_rt_blend_desc.SrcBlend = _blend_factor_to_d3d11_blend(attachment_blend_desc.src_color_blend_factor);
			d3d11_rt_blend_desc.DestBlend = _blend_factor_to_d3d11_blend(attachment_blend_desc.dst_color_blend_factor);
			d3d11_rt_blend_desc.BlendOp = _blend_op_to_d3d11_blend_op(attachment_blend_desc.color_blend_op);
			d3d11_rt_blend_desc.SrcBlendAlpha = _blend_factor_to_d3d11_blend(attachment_blend_desc.src_alpha_blend_factor);
			d3d11_rt_blend_desc.DestBlendAlpha = _blend_factor_to_d3d11_blend(attachment_blend_desc.dst_alpha_blend_factor);
			d3d11_rt_blend_desc.BlendOpAlpha = _blend_op_to_d3d11_blend_op(attachment_blend_desc.alpha_blend_op);
			//d3d11_rt_blend_desc.RenderTargetWriteMask = attachment_blend_desc.color_write_mask;
		}
		ID3D11BlendState* d3d11_blend_state = nullptr;
		HRESULT result = _device->CreateBlendState(&d3d11_blend_desc, &d3d11_blend_state);
		if (FAILED(result)) {
			_output_debug_message("Failed to create blend state: " + std::string(desc.debug_name));
			return BlendStateHandle();
		}
		_set_debug_name(d3d11_blend_state, desc.debug_name);
		return BlendStateHandle{ .object = (uintptr_t)d3d11_blend_state };
	}

	void destroy_blend_state(BlendStateHandle state) {
		if (!state.object) return;
		ID3D11BlendState* d3d11_blend_state = (ID3D11BlendState*)state.object;
		d3d11_blend_state->Release();
	}

	void bind_blend_state(BlendStateHandle state) {
		// SIC: Allow binding a null state to unbind the current state.
		ID3D11BlendState* d3d11_blend_state = (ID3D11BlendState*)state.object;
		_device_context->OMSetBlendState(d3d11_blend_state, nullptr, 0xffffffff);
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
		_device_context->Draw(vertex_count, vertex_offset);
	}

	void draw_indexed(unsigned int index_count, unsigned int base_vertex) {
		_device_context->DrawIndexed(index_count, 0, base_vertex);
	}

} // namespace api
} // namespace graphics

#endif // GRAPHICS_API_D3D11