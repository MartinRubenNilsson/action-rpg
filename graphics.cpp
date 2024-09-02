#include "stdafx.h"
#include "graphics.h"
#include "pool.h"
#include "console.h"
#include "filesystem.h"
#include "graphics_backend.h"

#include <glad/glad.h>
#define KHRONOS_STATIC
#include <ktx.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace graphics
{
	struct Shader
	{
		GLuint program_object = 0;
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
		std::string debug_name;
		GLuint framebuffer_object = 0;
		Handle<Texture> texture;
	};

	Viewport _viewport;
	Rect _scissor;
	bool _scissor_test_enabled = false;
	GLint _uniform_buffer_offset_alignment = 0;
	GLuint _vertex_array_object = 0;
	Pool<Shader> _shader_pool;
	Pool<Buffer> _buffer_pool;
	Pool<Texture> _texture_pool;
	std::unordered_map<std::string, Handle<Texture>> _path_to_texture;
	Pool<Sampler> _sampler_pool;
	Pool<Framebuffer> _framebuffer_pool;
	GLuint _last_bound_program_object = 0;
	unsigned int _total_texture_memory_usage_in_bytes = 0;

#ifdef _DEBUG_GRAPHICS
	void GLAPIENTRY _debug_message_callback(
		GLenum source,
		GLenum type,
		GLuint id,
		GLenum severity,
		GLsizei length,
		const GLchar* message,
		const void* userParam)
	{
		if (type == GL_DEBUG_TYPE_PUSH_GROUP) return;
		if (type == GL_DEBUG_TYPE_POP_GROUP) return;
		console::log_error("OpenGL: "s + message);
	}
#endif

	void initialize()
	{
#ifdef _DEBUG_GRAPHICS
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(_debug_message_callback, 0);
#endif
		// GET UNIFORM BUFFER OFFSET ALIGNMENT

		glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &_uniform_buffer_offset_alignment);

		// CREATE AND BIND VERTEX ARRAY

		glCreateVertexArrays(1, &_vertex_array_object);
#ifdef _DEBUG_GRAPHICS
		glObjectLabel(GL_VERTEX_ARRAY, _vertex_array_object, 0, "vertex array object");
#endif
		glBindVertexArray(_vertex_array_object);

		// SETUP VERTEX ARRAY ATTRIBUTES

		glEnableVertexArrayAttrib(_vertex_array_object, 0);
		glEnableVertexArrayAttrib(_vertex_array_object, 1);
		glEnableVertexArrayAttrib(_vertex_array_object, 2);
		glVertexArrayAttribFormat(_vertex_array_object, 0, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
		glVertexArrayAttribFormat(_vertex_array_object, 1, 4, GL_UNSIGNED_BYTE, GL_TRUE, offsetof(Vertex, color));
		glVertexArrayAttribFormat(_vertex_array_object, 2, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, tex_coord));
		glVertexArrayAttribBinding(_vertex_array_object, 0, 0);
		glVertexArrayAttribBinding(_vertex_array_object, 1, 0);
		glVertexArrayAttribBinding(_vertex_array_object, 2, 0);

		// SETUP BLENDING

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	void shutdown()
	{
		// DELETE VERTEX ARRAY OBJECT

		if (_vertex_array_object) {
			glDeleteVertexArrays(1, &_vertex_array_object);
			_vertex_array_object = 0;
		}

		// DELETE SHADERS

		for (const Shader& shader : _shader_pool.span()) {
			glDeleteProgram(shader.program_object);
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
			glDeleteFramebuffers(1, &framebuffer.framebuffer_object);
		}
		_framebuffer_pool.clear();
	}

	unsigned int get_uniform_buffer_offset_alignment()
	{
		return _uniform_buffer_offset_alignment;
	}

	Handle<Shader> create_shader(const ShaderDesc&& desc)
	{
		if (desc.vs_source.empty()) {
			console::log_error("Vertex shader source code is empty: " + std::string(desc.debug_name));
			return Handle<Shader>();
		}
		if (desc.fs_source.empty()) {
			console::log_error("Fragment shader source code is empty: " + std::string(desc.debug_name));
			return Handle<Shader>();
		}

		const GLuint program_object = glCreateProgram();

		// COMPILE AND ATTACH VERTEX SHADER
		{
			const char* vs_string = desc.vs_source.data();
			const GLint vs_length = (GLint)desc.vs_source.size();
			const GLuint vs_object = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vs_object, 1, &vs_string, &vs_length);
			glCompileShader(vs_object);
			int success;
			glGetShaderiv(vs_object, GL_COMPILE_STATUS, &success);
			if (!success) {
				char info_log[512];
				glGetShaderInfoLog(vs_object, sizeof(info_log), nullptr, info_log);
				console::log_error("Failed to compile vertex shader: " + std::string(desc.debug_name));
				console::log_error(info_log);
				glDeleteShader(vs_object);
				glDeleteProgram(program_object);
				return Handle<Shader>();
			}
			glAttachShader(program_object, vs_object);
			glDeleteShader(vs_object);
		}

		// COMPILE AND ATTACH FRAGMENT SHADER
		{
			const char* fs_string = desc.fs_source.data();
			const GLint fs_length = (GLint)desc.fs_source.size();
			const GLuint fs_object = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fs_object, 1, &fs_string, &fs_length);
			glCompileShader(fs_object);
			int success;
			glGetShaderiv(fs_object, GL_COMPILE_STATUS, &success);
			if (!success) {
				char info_log[512];
				glGetShaderInfoLog(fs_object, sizeof(info_log), nullptr, info_log);
				console::log_error("Failed to compile fragment shader: " + std::string(desc.debug_name));
				console::log_error(info_log);
				glDeleteShader(fs_object);
				glDeleteProgram(program_object);
				return Handle<Shader>();
			}
			glAttachShader(program_object, fs_object);
			glDeleteShader(fs_object);
		}

		// LINK PROGRAM OBJECT
		{
			glLinkProgram(program_object);
			int success;
			glGetProgramiv(program_object, GL_LINK_STATUS, &success);
			if (!success) {
				char info_log[512];
				glGetProgramInfoLog(program_object, sizeof(info_log), nullptr, info_log);
				console::log_error("Failed to link program object: " + std::string(desc.debug_name));
				console::log_error(info_log);
				glDeleteProgram(program_object);
				return Handle<Shader>();
			}
		}

		// STORE SHADER

#ifdef _DEBUG_GRAPHICS
		if (!desc.debug_name.empty()) {
			glObjectLabel(GL_PROGRAM, program_object, (GLsizei)desc.debug_name.size(), desc.debug_name.data());
		}
#endif

		return _shader_pool.emplace(program_object);
	}

	void _bind_program_if_not_already_bound(GLuint program_object)
	{
		// OpenGL does not guard agains redundant state changes,
		// so if we do it ourselves we get a performance boost.
		if (_last_bound_program_object == program_object) return;
		glUseProgram(program_object);
		_last_bound_program_object = program_object;
	}

	void bind_shader(Handle<Shader> handle)
	{
		if (handle == Handle<Shader>()) {
			_bind_program_if_not_already_bound(0);
		} else if (const Shader* shader = _shader_pool.get(handle)) {
			_bind_program_if_not_already_bound(shader->program_object);
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
			glVertexArrayVertexBuffer(_vertex_array_object, binding, 0, 0, 0);
		} else if (const Buffer* buffer = _buffer_pool.get(handle)) {
			glVertexArrayVertexBuffer(_vertex_array_object, binding, buffer->buffer_object, offset, stride);
		}
	}

	void bind_index_buffer(Handle<Buffer> handle)
	{
		if (handle == Handle<Buffer>()) {
			glVertexArrayElementBuffer(_vertex_array_object, 0);
		} else if (const Buffer* buffer = _buffer_pool.get(handle)) {
			glVertexArrayElementBuffer(_vertex_array_object, buffer->buffer_object);
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
		if (size % _uniform_buffer_offset_alignment != 0) return; // must be a multiple of alignment
		const Buffer* buffer = _buffer_pool.get(handle);
		if (!buffer) return;
		if (offset + size > buffer->desc.size) return;
		graphics_backend::bind_uniform_buffer_range(binding, buffer->buffer_object, size, offset);
	}

	GLenum _to_gl_base_format(Format format)
	{
		switch (format) {
		case Format::R8_UNORM:    return GL_RED;
		case Format::RG8_UNORM:   return GL_RG;
		case Format::RGB8_UNORM:  return GL_RGB;
		case Format::RGBA8_UNORM: return GL_RGBA;
		default: return 0;
		}
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
		unsigned char* data = stbi_load(normalized_path.c_str(), &width, &height, &channels, 0);
		if (!data) {
			console::log_error("Failed to load texture: " + normalized_path);
			console::log_error(stbi_failure_reason());
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

		stbi_image_free(data);
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
			return;
		}
		if (const Texture* texture = _texture_pool.get(handle)) {
			graphics_backend::bind_texture(binding, texture->texture_object);
		}
	}

	void update_texture(Handle<Texture> handle, const unsigned char* data)
	{
		const Texture* texture = _texture_pool.get(handle);
		if (!texture) return;
		const GLenum gl_base_format = _to_gl_base_format(texture->desc.format);
		glTextureSubImage2D(
			texture->texture_object,
			0, // level
			0, // xoffset
			0, // yoffset
			texture->desc.width,
			texture->desc.height,
			gl_base_format,
			GL_UNSIGNED_BYTE,
			data);
	}

	void copy_texture(Handle<Texture> dest, Handle<Texture> src)
	{
		Texture* dest_texture = _texture_pool.get(dest);
		const Texture* src_texture = _texture_pool.get(src);
		if (!dest_texture || !src_texture) return;
		if (dest_texture->desc.width != src_texture->desc.width) return;
		if (dest_texture->desc.height != src_texture->desc.height) return;
		graphics_backend::copy_texture_region(
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

	Handle<Sampler> create_sampler(const SamplerDesc&& desc)
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

	Handle<Framebuffer> create_framebuffer(const FramebufferDesc&& desc)
	{
		const std::string texture_debug_name = std::string(desc.debug_name) + " texture";
		const Handle<Texture> texture_handle = create_texture({
			.debug_name = texture_debug_name,
			.width = desc.width,
			.height = desc.height,
			.format = Format::RGBA8_UNORM });
		if (texture_handle == Handle<Texture>()) {
			console::log_error("Failed to create framebuffer texture: " + std::string(desc.debug_name));
			return Handle<Framebuffer>();
		}

		const Texture* texture = _texture_pool.get(texture_handle);
		assert(texture);

		GLuint framebuffer_object = 0;
		glCreateFramebuffers(1, &framebuffer_object);
#ifdef _DEBUG_GRAPHICS
		if (!desc.debug_name.empty()) {
			glObjectLabel(GL_FRAMEBUFFER, framebuffer_object, (GLsizei)desc.debug_name.size(), desc.debug_name.data());
		}
#endif

		glNamedFramebufferTexture(framebuffer_object, GL_COLOR_ATTACHMENT0, texture->texture_object, 0);

		if (glCheckNamedFramebufferStatus(framebuffer_object, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			console::log_error("Failed to create framebuffer: " + std::string(desc.debug_name));
			glDeleteFramebuffers(1, &framebuffer_object);
			destroy_texture(texture_handle);
			return Handle<Framebuffer>();
		}

		Framebuffer framebuffer{};
		framebuffer.debug_name = desc.debug_name;
		framebuffer.framebuffer_object = framebuffer_object;
		framebuffer.texture = texture_handle;

		return _framebuffer_pool.emplace(std::move(framebuffer));
	}

	void bind_default_framebuffer()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void bind_framebuffer(Handle<Framebuffer> handle)
	{
		if (const Framebuffer* framebuffer = _framebuffer_pool.get(handle)) {
			glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->framebuffer_object);
		}
	}

	void clear_default_framebuffer(float r, float g, float b, float a)
	{
		float color[4] = { r, g, b, a };
		glClearNamedFramebufferfv(0, GL_COLOR, 0, color);
	}

	void clear_framebuffer(Handle<Framebuffer> handle, float r, float g, float b, float a)
	{
		if (const Framebuffer* framebuffer = _framebuffer_pool.get(handle)) {
			float color[4] = { r, g, b, a };
			glClearNamedFramebufferfv(framebuffer->framebuffer_object, GL_COLOR, 0, color);
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
				ImGui::Image((ImTextureID)(uintptr_t)texture.texture_object,
					ImVec2((float)texture.desc.width, (float)texture.desc.height));
				ImGui::TreePop();
			}
		}
		ImGui::End();
#endif
	}
}