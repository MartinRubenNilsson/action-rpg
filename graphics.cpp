#include "stdafx.h"
#include "graphics.h"
#include "graphics_api.h"
#include "graphics_vertices.h"
#include "window.h"
#include "window_graphics.h"
#include "pool.h"
#include "console.h"
#include "filesystem.h"
#include "images.h"
#include "platform.h"
#define KHRONOS_STATIC
#include <ktx.h>

namespace graphics
{
	struct VertexShader {
		api::VertexShaderHandle api_handle{};
	};

	struct FragmentShader {
		api::FragmentShaderHandle api_handle{};
	};

	struct Buffer {
		api::BufferHandle api_handle{};
		BufferDesc desc{};
	};

	struct Texture {
		api::TextureHandle api_handle{};
		TextureDesc desc{};
	};

	struct Sampler {
		api::SamplerHandle api_handle{};
		SamplerDesc desc{};
	};

	struct Framebuffer {
		api::FramebufferHandle api_handle{};
		FramebufferDesc desc{};
		Handle<Texture> texture;
	};

	Viewport _viewport;
	Rect _scissor;
	bool _scissor_test_enabled = false;
	api::VertexInputHandle _vertex_input; //todo: move this to graphics_globals.h
	Pool<VertexShader> _vertex_shader_pool;
	Pool<FragmentShader> _fragment_shader_pool;
	Pool<Buffer> _buffer_pool;
	Pool<Texture> _texture_pool;
	std::unordered_map<std::string, Handle<Texture>> _path_to_texture;
	Pool<Sampler> _sampler_pool;
	Pool<Framebuffer> _framebuffer_pool;
	unsigned int _total_texture_memory_usage_in_bytes = 0;

#ifdef GRAPHICS_API_DEBUG
	void _debug_message_callback(std::string_view message) {
		__debugbreak();
		console::log_error(message);
	}
#endif

	void initialize() {
#ifdef GRAPHICS_API_DEBUG
		api::set_debug_message_callback(_debug_message_callback);
#endif
#ifdef GRAPHICS_API_OPENGL
		window::make_opengl_context_current();
#endif
#ifdef GRAPHICS_API_VULKAN
		if (!window::is_vulkan_supported()) {
			console::log_error("Vulkan is not supported on this system.");
			return;
		}

		// PITFALL: On my laptop (ROG Zephyrus), there's a cross-reaction between these two
		// layers that causes vkEnumeratePhysicalDevices() to return 0 devices:
		// 
		// "VK_LAYER_NV_optimus"
		// "VK_LAYER_AMD_switchable_graphics"
		//
		// The first NVIDIA-provided layer filters out my integrated AMD GPU, while the second
		// AMD-provided layer filters out my dedicated NVIDIA GPU. I had to disable the second
		// layer to get the dedicated GPU to show up.
		//
		// https://stackoverflow.com/questions/68109171/vkenumeratephysicaldevices-not-finding-all-gpus

		platform::set_environment_variable("DISABLE_LAYER_AMD_SWITCHABLE_GRAPHICS_1", "1");
#endif

		// INITIALIZE GRAPHICS API
		{
			api::InitializeOptions options{};
#ifdef GRAPHICS_API_OPENGL
			options.glad_load_proc = window::get_glad_load_proc();
#endif
#ifdef GRAPHICS_API_VULKAN
			options.application_name = APPLICATION_NAME;
			options.engine_name = ENGINE_NAME;
			options.vulkan_instance_extensions = window::get_required_vulkan_instance_extensions();
#endif
#ifdef GRAPHICS_API_D3D11
			options.hwnd = window::get_hwnd();
#endif
			api::initialize(options);
		}

		// CREATE VERTEX ARRAY
		//todo: move this to graphics_globals.cpp
		{
			VertexInputAttribDesc attribs[] = {
				{ .location = 0, .format = Format::RGB32_FLOAT, .offset = offsetof(Vertex, position) },
				{ .location = 1, .format = Format::RGBA8_UNORM, .offset = offsetof(Vertex, color), .normalized = true },
				{ .location = 2, .format = Format::RG32_FLOAT,  .offset = offsetof(Vertex, tex_coord) },
			};
			VertexInputDesc desc = { .attributes = attribs };
			_vertex_input = api::create_vertex_input(desc);
		}
	}

	void shutdown() {

		// DELETE VERTEX SHADERS

		for (const VertexShader& shader : _vertex_shader_pool.span()) {
			if (shader.api_handle.object) {
				api::destroy_vertex_shader(shader.api_handle);
			}
		}
		_vertex_shader_pool.clear();

		// DELETE FRAGMENT SHADERS

		for (const FragmentShader& shader : _fragment_shader_pool.span()) {
			if (shader.api_handle.object) {
				api::destroy_fragment_shader(shader.api_handle);
			}
		}

		// DELETE BUFFERS

		for (const Buffer& buffer : _buffer_pool.span()) {
			if (buffer.api_handle.object) {
				api::destroy_buffer(buffer.api_handle);
			}
		}
		_buffer_pool.clear();

		// DELETE TEXTURES

		for (const Texture& texture : _texture_pool.span()) {
			if (texture.api_handle.object) {
				api::destroy_texture(texture.api_handle);
			}
		}
		_texture_pool.clear();
		_path_to_texture.clear();

		// DELETE SAMPLERS

		for (const Sampler& sampler : _sampler_pool.span()) {
			if (sampler.api_handle.object) {
				api::destroy_sampler(sampler.api_handle);
			}
		}
		_sampler_pool.clear();

		// DELETE FRAMEBUFFERS

		for (const Framebuffer& framebuffer : _framebuffer_pool.span()) {
			if (framebuffer.api_handle.object) {
				api::destroy_framebuffer(framebuffer.api_handle);
			}
		}
		_framebuffer_pool.clear();

		api::shutdown();
	}

	bool is_spirv_supported() {
		return api::is_spirv_supported();
	}

	void push_debug_group(std::string_view name) {
		api::push_debug_group(name);
	}

	void pop_debug_group() {
		api::pop_debug_group();
	}

	Handle<VertexShader> create_vertex_shader(ShaderDesc&& desc) {
		api::VertexShaderHandle api_handle = api::create_vertex_shader(desc);
		if (!api_handle.object) return Handle<VertexShader>();
		return _vertex_shader_pool.emplace(api_handle);
	}

	void bind_vertex_shader(Handle<VertexShader> handle) {
		if (handle == Handle<VertexShader>()) {
			api::bind_vertex_shader(api::VertexShaderHandle());
		} else if (const VertexShader* shader = _vertex_shader_pool.get(handle)) {
			api::bind_vertex_shader(shader->api_handle);
		}
	}

	Handle<FragmentShader> create_fragment_shader(ShaderDesc&& desc) {
		api::FragmentShaderHandle api_handle = api::create_fragment_shader(desc);
		if (!api_handle.object) return Handle<FragmentShader>();
		return _fragment_shader_pool.emplace(api_handle);
	}

	void bind_fragment_shader(Handle<FragmentShader> handle) {
		if (handle == Handle<FragmentShader>()) {
			api::bind_fragment_shader(api::FragmentShaderHandle());
		} else if (const FragmentShader* shader = _fragment_shader_pool.get(handle)) {
			api::bind_fragment_shader(shader->api_handle);
		}
	}

	Handle<Buffer> create_buffer(BufferDesc&& desc) {
		api::BufferHandle api_handle = api::create_buffer(desc);
		if (!api_handle.object) return Handle<Buffer>();
		desc.initial_data = nullptr;
		return _buffer_pool.emplace(api_handle, desc);
	}

	void recreate_buffer(Handle<Buffer> handle, unsigned int size, const void* initial_data) {
		Buffer* buffer = _buffer_pool.get(handle);
		if (!buffer) return;
		api::destroy_buffer(buffer->api_handle);
		buffer->desc.size = size;
		buffer->desc.initial_data = initial_data;
		buffer->api_handle = api::create_buffer(buffer->desc);
		buffer->desc.initial_data = nullptr;
	}

	void destroy_buffer(Handle<Buffer> handle) {
		Buffer* buffer = _buffer_pool.get(handle);
		if (!buffer) return;
		api::destroy_buffer(buffer->api_handle);
		*buffer = Buffer();
		_buffer_pool.free(handle);
	}

	void update_buffer(Handle<Buffer> handle, const void* data, unsigned int size, unsigned int offset) {
		if (!data || !size) return;
		Buffer* buffer = _buffer_pool.get(handle);
		if (!buffer) return;
		if (!buffer->desc.dynamic) return;
		if (offset + size > buffer->desc.size) return;
		api::update_buffer(buffer->api_handle, data, size, offset);
	}

	size_t get_buffer_size(Handle<Buffer> handle) {
		if (const Buffer* buffer = _buffer_pool.get(handle)) {
			return buffer->desc.size;
		}
		return 0;
	}

	void bind_vertex_buffer(unsigned int binding, Handle<Buffer> handle, unsigned int stride, unsigned int offset) {
		if (handle == Handle<Buffer>()) {
			api::bind_vertex_buffer(_vertex_input, binding, api::BufferHandle(), 0, 0);
		} else if (const Buffer* buffer = _buffer_pool.get(handle)) {
			api::bind_vertex_buffer(_vertex_input, binding, buffer->api_handle, stride, offset);
		}
	}

	void bind_index_buffer(Handle<Buffer> handle) {
		if (handle == Handle<Buffer>()) {
			api::bind_index_buffer(_vertex_input, api::BufferHandle());
		} else if (const Buffer* buffer = _buffer_pool.get(handle)) {
			api::bind_index_buffer(_vertex_input, buffer->api_handle);
		}
	}

	void bind_uniform_buffer(unsigned int binding, Handle<Buffer> handle) {
		if (handle == Handle<Buffer>()) {
			api::bind_uniform_buffer(binding, api::BufferHandle());
		} else if (const Buffer* buffer = _buffer_pool.get(handle)) {
			api::bind_uniform_buffer(binding, buffer->api_handle);
		}
	}

	void bind_uniform_buffer_range(unsigned int binding, Handle<Buffer> handle, unsigned int size, unsigned offset) {
		const Buffer* buffer = _buffer_pool.get(handle);
		if (!buffer) return;
		if (offset + size > buffer->desc.size) return;
		api::bind_uniform_buffer_range(binding, buffer->api_handle, size, offset);
	}

	unsigned _get_size(Format format) {
		switch (format) {
		case Format::R8_UNORM:    return 1;
		case Format::RG8_UNORM:   return 2;
		case Format::RGB8_UNORM:  return 3;
		case Format::RGBA8_UNORM: return 4;
		default: return 0;
		}
	}

	Handle<Texture> create_texture(TextureDesc&& desc) {
		api::TextureHandle api_handle = api::create_texture(desc);
		if (!api_handle.object) return Handle<Texture>();
		desc.initial_data = nullptr;
		_total_texture_memory_usage_in_bytes += desc.width * desc.height * _get_size(desc.format);
		return _texture_pool.emplace(api_handle, desc);
	}

	Handle<Texture> load_texture(const std::string& path) {
		const std::string normalized_path = filesystem::get_normalized_path(path);

#if 1
		// First, attempt to load the texture from a KTX2 file, if it exists.
		if (const std::string normalized_path_ktx2 = filesystem::replace_extension(normalized_path, ".ktx2");
			filesystem::file_exists(normalized_path_ktx2)) {

			if (const auto it = _path_to_texture.find(normalized_path_ktx2); it != _path_to_texture.end()) {
				return it->second;
			}

			ktxTexture2* ktx_texture;
			ktxResult result = ktxTexture2_CreateFromNamedFile(
				normalized_path_ktx2.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktx_texture);

			if (result != KTX_SUCCESS) {
				console::log_error("Failed to load KTX2 texture: " + normalized_path_ktx2);
				return Handle<Texture>();
			}

			// TODO: check ktx_texture->vkFormat and convert to OpenGL format if necessary

			const Handle<Texture> handle = create_texture({
				.debug_name = normalized_path_ktx2,
				.width = ktx_texture->baseWidth,
				.height = ktx_texture->baseHeight,
				.format = Format::RGBA8_UNORM, // we assume this format for now
				.initial_data = ktx_texture->pData });

			ktxTexture_Destroy(ktxTexture(ktx_texture));
			_path_to_texture[normalized_path_ktx2] = handle;
			return handle;
		}
#endif

		if (const auto it = _path_to_texture.find(normalized_path); it != _path_to_texture.end()) {
			return it->second;
		}

		unsigned int width, height, channels;
		unsigned char* data = images::load(normalized_path.c_str(), &width, &height, &channels, 0);
		if (!data) {
			console::log_error("Failed to load texture: " + normalized_path);
			console::log_error(images::failure_reason());
			return Handle<Texture>();
		}

		Format format = Format::UNKNOWN;
		if (channels == 1) format = Format::R8_UNORM;
		else if (channels == 2) format = Format::RG8_UNORM;
		else if (channels == 3) format = Format::RGB8_UNORM;
		else if (channels == 4) format = Format::RGBA8_UNORM;

		const Handle<Texture> handle = create_texture({
			.debug_name = normalized_path,
			.width = width,
			.height = height,
			.format = format,
			.initial_data = data });

		images::free(data);
		_path_to_texture[normalized_path] = handle;
		return handle;
	}

	Handle<Texture> copy_texture(Handle<Texture> src) {
		Handle<Texture> dest;
		if (const Texture* src_texture = _texture_pool.get(src)) {
			dest = create_texture(TextureDesc(src_texture->desc));
			copy_texture(dest, src);
		}
		return dest;
	}

	void destroy_texture(Handle<Texture> handle) {
		Texture* texture = _texture_pool.get(handle);
		if (!texture) return;
		api::destroy_texture(texture->api_handle);
		_total_texture_memory_usage_in_bytes -=
			texture->desc.width * texture->desc.height * _get_size(texture->desc.format);
		// HACK: When a texture is loaded, its debug_name is set to the path.
		_path_to_texture.erase(std::string(texture->desc.debug_name));
		*texture = Texture();
		_texture_pool.free(handle);
	}

	void bind_texture(unsigned int binding, Handle<Texture> handle) {
		if (handle == Handle<Texture>()) {
			api::bind_texture(binding, api::TextureHandle());
		} else if (const Texture* texture = _texture_pool.get(handle)) {
			api::bind_texture(binding, texture->api_handle);
		}
	}

	void update_texture(Handle<Texture> handle, const unsigned char* data) {
		const Texture* texture = _texture_pool.get(handle);
		if (!texture) return;
		api::update_texture(texture->api_handle, 0, 0, 0,
			texture->desc.width, texture->desc.height, texture->desc.format, data);
	}

	void copy_texture(Handle<Texture> dest, Handle<Texture> src) {
		Texture* dest_texture = _texture_pool.get(dest);
		const Texture* src_texture = _texture_pool.get(src);
		if (!dest_texture || !src_texture) return;
		if (dest_texture->desc.width != src_texture->desc.width) return;
		if (dest_texture->desc.height != src_texture->desc.height) return;
		api::copy_texture(
			dest_texture->api_handle, 0, 0, 0, 0,
			src_texture->api_handle, 0, 0, 0, 0,
			src_texture->desc.width, src_texture->desc.height, 1);
	}

	void get_texture_size(Handle<Texture> handle, unsigned int& width, unsigned int& height) {
		if (const Texture* texture = _texture_pool.get(handle)) {
			width = texture->desc.width;
			height = texture->desc.height;
		} else {
			width = 0;
			height = 0;
		}
	}

	Handle<Sampler> create_sampler(SamplerDesc&& desc) {
		api::SamplerHandle api_handle = api::create_sampler(desc);
		if (!api_handle.object) return Handle<Sampler>();
		return _sampler_pool.emplace(api_handle, desc);
	}

	void destroy_sampler(Handle<Sampler> handle) {
		Sampler* sampler = _sampler_pool.get(handle);
		if (!sampler) return;
		api::destroy_sampler(sampler->api_handle);
		*sampler = Sampler();
		_sampler_pool.free(handle);
	}

	void bind_sampler(unsigned int binding, Handle<Sampler> handle) {
		if (handle == Handle<Sampler>()) {
			api::bind_sampler(binding, api::SamplerHandle());
		} else if (const Sampler* sampler = _sampler_pool.get(handle)) {
			api::bind_sampler(binding, sampler->api_handle);
		}
	}

	Handle<Framebuffer> create_framebuffer(FramebufferDesc&& desc) {
		api::FramebufferHandle api_handle = api::create_framebuffer(desc);
		if (!api_handle.object) return Handle<Framebuffer>();
		return _framebuffer_pool.emplace(api_handle, desc);
	}

	void attach_framebuffer_texture(Handle<Framebuffer> framebuffer_handle, Handle<Texture> texture_handle) {
		Framebuffer* framebuffer = _framebuffer_pool.get(framebuffer_handle);
		if (!framebuffer) return;
		Texture* texture = _texture_pool.get(texture_handle);
		if (!texture) return;
		if (!api::attach_framebuffer_texture(framebuffer->api_handle, texture->api_handle)) {
			console::log_error(
				"Failed to attach texture " +
				std::string(texture->desc.debug_name) +
				" to framebuffer " +
				std::string(framebuffer->desc.debug_name));
			return;
		}
		framebuffer->texture = texture_handle;
	}

	void bind_default_framebuffer() {
		api::bind_framebuffer(api::get_default_framebuffer());
	}

	void bind_framebuffer(Handle<Framebuffer> handle) {
		if (const Framebuffer* framebuffer = _framebuffer_pool.get(handle)) {
			api::bind_framebuffer(framebuffer->api_handle);
		}
	}

	void clear_default_framebuffer(const float color[4]) {
		api::clear_framebuffer(api::get_default_framebuffer(), color);
	}

	void clear_framebuffer(Handle<Framebuffer> handle, const float color[4]) {
		if (const Framebuffer* framebuffer = _framebuffer_pool.get(handle)) {
			api::clear_framebuffer(framebuffer->api_handle, color);
		}
	}

	Handle<Texture> get_framebuffer_texture(Handle<Framebuffer> handle) {
		if (const Framebuffer* framebuffer = _framebuffer_pool.get(handle)) {
			return framebuffer->texture;
		}
		return Handle<Texture>();
	}

	void set_primitives(Primitives primitives) {
		api::set_primitives(primitives);
	}

	void set_viewport(const Viewport& viewport) {
		api::set_viewports(&viewport, 1);
		_viewport = viewport;
	}

	void get_viewport(Viewport& viewport) {
		viewport = _viewport;
	}

	void set_scissor(const Rect& scissor) {
		api::set_scissors(&scissor, 1);
		_scissor = scissor;
	}

	void set_scissor_test_enabled(bool enable) {
		api::set_scissor_test_enabled(enable);
		_scissor_test_enabled = enable;
	}

	bool get_scissor_test_enabled() {
		return _scissor_test_enabled;
	}

	void get_scissor(Rect& scissor) {
		scissor = _scissor;
	}

	void draw(unsigned int vertex_count, unsigned int vertex_offset) {
		api::draw(vertex_count, vertex_offset);
	}

	void draw_indexed(unsigned int index_count) {
		api::draw_indexed(index_count);
	}

	void swap_buffers() {
#ifdef GRAPHICS_API_OPENGL
		window::swap_buffers();
#endif
#ifdef GRAPHICS_API_D3D11
		api::swap_buffers();
#endif
	}

	void show_texture_debug_window() {
#ifdef _DEBUG_IMGUI
		ImGui::Begin("Textures");
		ImGui::Text("Total memory usage: %d MB", _total_texture_memory_usage_in_bytes / 1024 / 1024);
		for (size_t i = 0; i < _texture_pool.size(); ++i) {
			const Texture& texture = _texture_pool.data()[i];
			if (!texture.api_handle.object) continue;
			if (ImGui::TreeNode(texture.desc.debug_name.data())) {
				ImGui::Text("Dimensions: %dx%dx", texture.desc.width, texture.desc.height);
				ImGui::Text("Format: %s", magic_enum::enum_name(texture.desc.format).data());
				unsigned int size = texture.desc.width * texture.desc.height * _get_size(texture.desc.format);
				unsigned int kb = size / 1024;
				unsigned int mb = kb / 1024;
				if (mb) {
					ImGui::Text("Memory: %d MB", mb);
				} else {
					ImGui::Text("Memory: %d KB", kb);
				}
				ImVec2 texture_size = ImVec2((float)texture.desc.width, (float)texture.desc.height);
				ImGui::Image((ImTextureID)texture.api_handle.object, texture_size);
				ImGui::TreePop();
			}
		}
		ImGui::End();
#endif
	}
}