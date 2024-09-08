#include "stdafx.h"
#include "graphics.h"
#include "graphics_backend.h"
#include "pool.h"
#include "console.h"
#include "filesystem.h"
#include "images.h"
#define KHRONOS_STATIC
#include <ktx.h>

namespace graphics
{
	struct Shader
	{
		uintptr_t shader_object = 0;
		ShaderDesc desc{};
	};

	struct Buffer
	{
		uintptr_t buffer_object = 0;
		BufferDesc desc{};
	};

	struct Texture
	{
		uintptr_t texture_object = 0;
		TextureDesc desc{};
	};

	struct Sampler
	{
		uintptr_t sampler_object = 0;
		SamplerDesc desc{};
	};

	struct Framebuffer
	{
		uintptr_t framebuffer_object = 0;
		FramebufferDesc desc{};
		Handle<Texture> texture;
	};

	Viewport _viewport;
	Rect _scissor;
	bool _scissor_test_enabled = false;
	Pool<Shader> _shader_pool;
	Pool<Buffer> _buffer_pool;
	Pool<Texture> _texture_pool;
	std::unordered_map<std::string, Handle<Texture>> _path_to_texture;
	Pool<Sampler> _sampler_pool;
	Pool<Framebuffer> _framebuffer_pool;
	unsigned int _total_texture_memory_usage_in_bytes = 0;

#ifdef _DEBUG_GRAPHICS
	void _debug_message_callback(std::string_view message)
	{
		__debugbreak();
		console::log_error(std::string(message));
	}
#endif

	void initialize()
	{
#ifdef _DEBUG_GRAPHICS
		graphics_backend::set_debug_message_callback(_debug_message_callback);
#endif
		graphics_backend::initialize();
	}

	void shutdown()
	{
		// DELETE SHADERS

		for (const Shader& shader : _shader_pool.span()) {
			if (shader.shader_object) {
				graphics_backend::destroy_shader(shader.shader_object);
			}
		}
		_shader_pool.clear();

		// DELETE BUFFERS

		for (const Buffer& buffer : _buffer_pool.span()) {
			if (buffer.buffer_object) {
				graphics_backend::destroy_buffer(buffer.buffer_object);
			}
		}

		// DELETE TEXTURES

		for (const Texture& texture : _texture_pool.span()) {
			if (texture.texture_object) {
				graphics_backend::destroy_texture(texture.texture_object);
			}
		}
		_texture_pool.clear();
		_path_to_texture.clear();

		// DELETE SAMPLERS

		for (const Sampler& sampler : _sampler_pool.span()) {
			if (sampler.sampler_object) {
				graphics_backend::destroy_sampler(sampler.sampler_object);
			}
		}
		_sampler_pool.clear();

		// DELETE FRAMEBUFFERS

		for (const Framebuffer& framebuffer : _framebuffer_pool.span()) {
			if (framebuffer.framebuffer_object) {
				graphics_backend::destroy_framebuffer(framebuffer.framebuffer_object);
			}
		}
		_framebuffer_pool.clear();

		graphics_backend::shutdown();
	}

	Handle<Shader> create_shader(ShaderDesc&& desc)
	{
		uintptr_t shader_object = graphics_backend::create_shader(desc);
		if (!shader_object) return Handle<Shader>();
		desc.fs_source = "";
		desc.vs_source = "";
		return _shader_pool.emplace(shader_object, desc);
	}

	void bind_shader(Handle<Shader> handle)
	{
		if (handle == Handle<Shader>()) {
			graphics_backend::bind_shader(0);
		} else if (const Shader* shader = _shader_pool.get(handle)) {
			graphics_backend::bind_shader(shader->shader_object);
		}
	}

	Handle<Buffer> create_buffer(BufferDesc&& desc)
	{
		uintptr_t buffer_object = graphics_backend::create_buffer(desc);
		if (!buffer_object) return Handle<Buffer>();
		desc.initial_data = nullptr;
		return _buffer_pool.emplace(buffer_object, desc);
	}

	void recreate_buffer(Handle<Buffer> handle, unsigned int size, const void* initial_data)
	{
		Buffer* buffer = _buffer_pool.get(handle);
		if (!buffer) return;
		graphics_backend::destroy_buffer(buffer->buffer_object);
		buffer->desc.size = size;
		buffer->desc.initial_data = initial_data;
		buffer->buffer_object = graphics_backend::create_buffer(buffer->desc);
		buffer->desc.initial_data = nullptr;
	}

	void destroy_buffer(Handle<Buffer> handle)
	{
		Buffer* buffer = _buffer_pool.get(handle);
		if (!buffer) return;
		graphics_backend::destroy_buffer(buffer->buffer_object);
		*buffer = Buffer();
		_buffer_pool.free(handle);
	}

	void update_buffer(Handle<Buffer> handle, const void* data, unsigned int size, unsigned int offset)
	{
		if (!data || !size) return;
		Buffer* buffer = _buffer_pool.get(handle);
		if (!buffer) return;
		if (!buffer->desc.dynamic) return;
		if (offset + size > buffer->desc.size) return;
		graphics_backend::update_buffer(buffer->buffer_object, data, size, offset);
	}

	size_t get_buffer_size(Handle<Buffer> handle)
	{
		if (const Buffer* buffer = _buffer_pool.get(handle)) {
			return buffer->desc.size;
		}
		return 0;
	}

	void bind_vertex_buffer(unsigned int binding, Handle<Buffer> handle, unsigned int stride, unsigned int offset)
	{
		if (handle == Handle<Buffer>()) {
			graphics_backend::bind_vertex_buffer(binding, 0, 0, 0);
		} else if (const Buffer* buffer = _buffer_pool.get(handle)) {
			graphics_backend::bind_vertex_buffer(binding, buffer->buffer_object, stride, offset);
		}
	}

	void bind_index_buffer(Handle<Buffer> handle)
	{
		if (handle == Handle<Buffer>()) {
			graphics_backend::bind_index_buffer(0);
		} else if (const Buffer* buffer = _buffer_pool.get(handle)) {
			graphics_backend::bind_index_buffer(buffer->buffer_object);
		}
	}

	void bind_uniform_buffer(unsigned int binding, Handle<Buffer> handle)
	{
		if (handle == Handle<Buffer>()) {
			graphics_backend::bind_uniform_buffer(binding, 0);
		} else if (const Buffer* buffer = _buffer_pool.get(handle)) {
			graphics_backend::bind_uniform_buffer(binding, buffer->buffer_object);
		}
	}

	void bind_uniform_buffer_range(unsigned int binding, Handle<Buffer> handle, unsigned int size, unsigned offset)
	{
		const Buffer* buffer = _buffer_pool.get(handle);
		if (!buffer) return;
		if (offset + size > buffer->desc.size) return;
		graphics_backend::bind_uniform_buffer_range(binding, buffer->buffer_object, size, offset);
	}

	unsigned _get_size(Format format)
	{
		switch (format) {
		case Format::R8_UNORM:    return 1;
		case Format::RG8_UNORM:   return 2;
		case Format::RGB8_UNORM:  return 3;
		case Format::RGBA8_UNORM: return 4;
		default: return 0;
		}
	}

	Handle<Texture> create_texture(TextureDesc&& desc)
	{
		uintptr_t texture_object = graphics_backend::create_texture(desc);
		if (!texture_object) return Handle<Texture>();
		desc.initial_data = nullptr;
		_total_texture_memory_usage_in_bytes += desc.width * desc.height * _get_size(desc.format);
		return _texture_pool.emplace(texture_object, desc);
	}

	Handle<Texture> load_texture(const std::string& path)
	{
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

		int width, height, channels;
		unsigned char* data = images::load(normalized_path.c_str(), &width, &height, &channels, 0);
		if (!data) {
			console::log_error("Failed to load texture: " + normalized_path);
			console::log_error(images::failure_reason());
			return Handle<Texture>();
		}

		Format format = Format::UNKNOWN;
		if      (channels == 1) format = Format::R8_UNORM;
		else if (channels == 2) format = Format::RG8_UNORM;
		else if (channels == 3) format = Format::RGB8_UNORM;
		else if (channels == 4) format = Format::RGBA8_UNORM;

		const Handle<Texture> handle = create_texture({
			.debug_name = normalized_path,
			.width = (unsigned int)width,
			.height = (unsigned int)height,
			.format = format,
			.initial_data = data });

		images::free(data);
		_path_to_texture[normalized_path] = handle;
		return handle;
	}

	Handle<Texture> copy_texture(Handle<Texture> src)
	{
		Handle<Texture> dest;
		if (const Texture* src_texture = _texture_pool.get(src)) {
			dest = create_texture(TextureDesc(src_texture->desc));
			copy_texture(dest, src);
		}
		return dest;
	}

	void destroy_texture(Handle<Texture> handle)
	{
		Texture* texture = _texture_pool.get(handle);
		if (!texture) return;
		graphics_backend::destroy_texture(texture->texture_object);
		_total_texture_memory_usage_in_bytes -=
			texture->desc.width * texture->desc.height * _get_size(texture->desc.format);
		// HACK: When a texture is loaded, its debug_name is set to the path.
		_path_to_texture.erase(std::string(texture->desc.debug_name));
		*texture = Texture();
		_texture_pool.free(handle);
	}

	void bind_texture(unsigned int binding, Handle<Texture> handle)
	{
		if (handle == Handle<Texture>()) {
			graphics_backend::bind_texture(binding, 0);
		} else if (const Texture* texture = _texture_pool.get(handle)) {
			graphics_backend::bind_texture(binding, texture->texture_object);
		}
	}

	void update_texture(Handle<Texture> handle, const unsigned char* data)
	{
		const Texture* texture = _texture_pool.get(handle);
		if (!texture) return;
		graphics_backend::update_texture(texture->texture_object, 0, 0, 0,
			texture->desc.width, texture->desc.height, texture->desc.format, data);
	}

	void copy_texture(Handle<Texture> dest, Handle<Texture> src)
	{
		Texture* dest_texture = _texture_pool.get(dest);
		const Texture* src_texture = _texture_pool.get(src);
		if (!dest_texture || !src_texture) return;
		if (dest_texture->desc.width != src_texture->desc.width) return;
		if (dest_texture->desc.height != src_texture->desc.height) return;
		graphics_backend::copy_texture(
			dest_texture->texture_object, 0, 0, 0, 0,
			src_texture->texture_object, 0, 0, 0, 0,
			src_texture->desc.width, src_texture->desc.height, 1);
	}

	void get_texture_size(Handle<Texture> handle, unsigned int& width, unsigned int& height)
	{
		if (const Texture* texture = _texture_pool.get(handle)) {
			width = texture->desc.width;
			height = texture->desc.height;
		} else {
			width = 0;
			height = 0;
		}
	}

	Handle<Sampler> create_sampler(SamplerDesc&& desc)
	{
		uintptr_t sampler_object = graphics_backend::create_sampler(desc);
		if (!sampler_object) return Handle<Sampler>();
		return _sampler_pool.emplace(sampler_object, desc);
	}

	void destroy_sampler(Handle<Sampler> handle)
	{
		Sampler* sampler = _sampler_pool.get(handle);
		if (!sampler) return;
		graphics_backend::destroy_sampler(sampler->sampler_object);
		*sampler = Sampler();
		_sampler_pool.free(handle);
	}

	void bind_sampler(unsigned int binding, Handle<Sampler> handle)
	{
		if (handle == Handle<Sampler>()) {
			graphics_backend::bind_sampler(binding, 0);
		} else if (const Sampler* sampler = _sampler_pool.get(handle)) {
			graphics_backend::bind_sampler(binding, sampler->sampler_object);
		}
	}

	Handle<Framebuffer> create_framebuffer(FramebufferDesc&& desc)
	{
		uintptr_t framebuffer_object = graphics_backend::create_framebuffer(desc);
		if (!framebuffer_object) return Handle<Framebuffer>();
		return _framebuffer_pool.emplace(framebuffer_object, desc);
	}

	void attach_framebuffer_texture(Handle<Framebuffer> framebuffer_handle, Handle<Texture> texture_handle)
	{
		Framebuffer* framebuffer = _framebuffer_pool.get(framebuffer_handle);
		if (!framebuffer) return;
		Texture* texture = _texture_pool.get(texture_handle);
		if (!texture) return;
		if (!graphics_backend::attach_framebuffer_texture(framebuffer->framebuffer_object, texture->texture_object)) {
			console::log_error(
				"Failed to attach texture " +
				std::string(texture->desc.debug_name) +
				" to framebuffer " +
				std::string(framebuffer->desc.debug_name));
			return;
		}
		framebuffer->texture = texture_handle;
	}

	void bind_default_framebuffer()
	{
		graphics_backend::bind_framebuffer(0);
	}

	void bind_framebuffer(Handle<Framebuffer> handle)
	{
		if (const Framebuffer* framebuffer = _framebuffer_pool.get(handle)) {
			graphics_backend::bind_framebuffer(framebuffer->framebuffer_object);
		}
	}

	void clear_default_framebuffer(const float color[4])
	{
		graphics_backend::clear_framebuffer(0, color);
	}

	void clear_framebuffer(Handle<Framebuffer> handle, const float color[4])
	{
		if (const Framebuffer* framebuffer = _framebuffer_pool.get(handle)) {
			graphics_backend::clear_framebuffer(framebuffer->framebuffer_object, color);
		}
	}

	Handle<Texture> get_framebuffer_texture(Handle<Framebuffer> handle)
	{
		if (const Framebuffer* framebuffer = _framebuffer_pool.get(handle)) {
			return framebuffer->texture;
		}
		return Handle<Texture>();
	}

	void set_viewport(const Viewport& viewport)
	{
		graphics_backend::set_viewports(&viewport, 1);
		_viewport = viewport;
	}

	void get_viewport(Viewport& viewport)
	{
		viewport = _viewport;
	}

	void set_scissor(const Rect& scissor)
	{
		graphics_backend::set_scissors(&scissor, 1);
		_scissor = scissor;
	}

	void set_scissor_test_enabled(bool enable)
	{
		graphics_backend::set_scissor_test_enabled(enable);
		_scissor_test_enabled = enable;
	}

	bool get_scissor_test_enabled()
	{
		return _scissor_test_enabled;
	}

	void get_scissor(Rect& scissor)
	{
		scissor = _scissor;
	}

	void draw(Primitives primitives, unsigned int vertex_count, unsigned int vertex_offset)
	{
		graphics_backend::draw(primitives, vertex_count, vertex_offset);
	}

	void draw_indexed(Primitives primitives, unsigned int index_count)
	{
		graphics_backend::draw_indexed(primitives, index_count);
	}

	void push_debug_group(std::string_view name)
	{
#ifdef _DEBUG_GRAPHICS
		graphics_backend::push_debug_group(name);
#endif
	}

	void pop_debug_group()
	{
#ifdef _DEBUG_GRAPHICS
		graphics_backend::pop_debug_group();
#endif
	}

	void show_texture_debug_window()
	{
#ifdef _DEBUG_IMGUI
		ImGui::Begin("Textures");
		ImGui::Text("Total memory usage: %d MB", _total_texture_memory_usage_in_bytes / 1024 / 1024);
		for (size_t i = 0; i < _texture_pool.size(); ++i) {
			const Texture& texture = _texture_pool.data()[i];
			if (texture.texture_object == 0) continue;
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
				ImGui::Image((ImTextureID)texture.texture_object, texture_size);
				ImGui::TreePop();
			}
		}
		ImGui::End();
#endif
	}
}