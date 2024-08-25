#include "stdafx.h"
#include "graphics.h"
#include "pool.h"
#include "console.h"
#include "filesystem.h"

#include <glad/glad.h>
#define KHRONOS_STATIC
#include <ktx.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#pragma comment(lib, "opengl32")

// Undefine pre-DSA functions to force the use of DSA whenever possible.

#undef glGenTextures
#undef glBindTexture
#undef glTexParameterf
#undef glTexParameteri
#undef glTexParameterfv
#undef glTexParameteriv
#undef glTexParameterIiv
#undef glTexParameterIuiv
#undef glTexImage2D
#undef glTexStorage2D
#undef glTexSubImage2D
#undef glGenerateMipmap
#undef glActiveTexture

#undef glGenFramebuffers
#undef glFramebufferTexture2D 
#undef glCheckFramebufferStatus
#undef glBlitFramebuffer
#undef glClearColor
#undef glClearDepth
#undef glClearStencil
#undef glClear
#undef glClearBufferiv
#undef glClearBufferuiv
#undef glClearBufferfv
#undef glClearBufferfi

#undef glGenBuffer
#undef glBufferData
#undef glBufferSubData
#undef glBufferStorage
#undef glNamedBufferData

#undef glGenVertexArrays
#undef glEnableVertexAttribArray
#undef glDisableVertexAttribArray
#undef glVertexAttribPointer
#undef glVertexAttribFormat
#undef glVertexAttribIFormat
#undef glVertexAttribLFormat
#undef glVertexAttribBinding
#undef glVertexBindingDivisor
#undef glBindVertexBuffer

namespace graphics
{
	constexpr GLsizei _UNIFORM_NAME_MAX_SIZE = 64;

	struct ShaderUniform
	{
		char name[_UNIFORM_NAME_MAX_SIZE] = { 0 };
		GLsizei name_size = 0;
		GLint location = -1;
	};

	struct Shader
	{
		std::string debug_name;
		std::vector<ShaderUniform> uniforms;
		GLuint program_object = 0;
	};

	struct Buffer
	{
		GLuint buffer_object = 0;
		BufferDesc desc{};
	};

	struct Texture
	{
		std::string debug_name;
		GLuint texture_object = 0;
		unsigned int width = 0;
		unsigned int height = 0;
		Format format = Format::UNKNOWN;
		unsigned int size = 0; // in bytes
	};

	struct Sampler
	{
		GLuint sampler_object = 0;
		SamplerDesc desc{};
	};

	struct Framebuffer
	{
		std::string debug_name;
		GLuint framebuffer_object = 0;
		Handle<Texture> texture;
	};

	GLint _uniform_buffer_offset_alignment = 0;
	GLuint _vertex_array_object = 0;
	Pool<Shader> _shader_pool;
	std::unordered_map<std::string, Handle<Shader>> _path_to_shader;
	Pool<Buffer> _buffer_pool;
	Pool<Texture> _texture_pool;
	std::unordered_map<std::string, Handle<Texture>> _path_to_texture;
	Pool<Sampler> _sampler_pool;
	Pool<Framebuffer> _framebuffer_pool;
	GLuint _last_bound_program_object = 0;
	unsigned int _total_texture_memory_usage_in_bytes = 0;

	const ShaderUniform* _get_shader_uniform(const std::vector<ShaderUniform> &uniforms, std::string_view name)
	{
		for (const ShaderUniform& uniform : uniforms) {
			if (name.size() != uniform.name_size) continue;
			if (memcmp(name.data(), uniform.name, uniform.name_size)) continue;
			return &uniform;
		}
		return nullptr;
	}

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

	void _set_debug_label(GLenum identifier, GLuint name, std::string_view label)
	{
#ifdef _DEBUG_GRAPHICS
		glObjectLabel(identifier, name, (GLsizei)label.size(), label.data());
#endif
	}

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
		_set_debug_label(GL_VERTEX_ARRAY, _vertex_array_object, "vertex array");
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
		_path_to_shader.clear();

		// DELETE BUFFERS

		for (const Buffer& buffer : _buffer_pool.span()) {
			glDeleteBuffers(1, &buffer.buffer_object);
		}

		// DELETE TEXTURES

		for (const Texture& texture : _texture_pool.span()) {
			if (texture.texture_object) {
				glDeleteTextures(1, &texture.texture_object);
			}
		}
		_texture_pool.clear();
		_path_to_texture.clear();

		// DELETE SAMPLERS
		for (const Sampler& sampler : _sampler_pool.span()) {
			glDeleteSamplers(1, &sampler.sampler_object);
		}

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

		// GET UNIFORM LOCATIONS

		std::vector<ShaderUniform> uniform_locations;
		{
			GLint uniform_count;
			glGetProgramiv(program_object, GL_ACTIVE_UNIFORMS, &uniform_count);
			uniform_locations.resize(uniform_count);
			for (GLint i = 0; i < uniform_count; i++) {
				int uniform_size;
				unsigned int uniform_type;
				ShaderUniform& uniform = uniform_locations[i];
				glGetActiveUniform(program_object, i, _UNIFORM_NAME_MAX_SIZE,
					&uniform.name_size, &uniform_size, &uniform_type, uniform.name);
				uniform.location = glGetUniformLocation(program_object, uniform.name);
			}
		}

		// STORE SHADER

		Shader shader{};
		shader.debug_name = desc.debug_name;
		shader.uniforms = std::move(uniform_locations);
		shader.program_object = program_object;
		_set_debug_label(GL_PROGRAM, program_object, desc.debug_name);

		return _shader_pool.emplace(std::move(shader));
	}

	Handle<Shader> load_shader(const std::string& vs_path, const std::string& fs_path)
	{
		const std::string normalized_vs_path = filesystem::get_normalized_path(vs_path);
		const std::string normalized_fs_path = filesystem::get_normalized_path(fs_path);
		const std::string paths = normalized_vs_path + ":" + normalized_fs_path;

		if (const auto it = _path_to_shader.find(paths); it != _path_to_shader.end()) {
			return it->second;
		}

		std::string vs_source;
		if (!filesystem::read_text(normalized_vs_path, vs_source)) {
			console::log_error("Failed to open vertex shader file: " + normalized_vs_path);
			return Handle<Shader>();
		}

		std::string fs_source;
		if (!filesystem::read_text(normalized_fs_path, fs_source)) {
			console::log_error("Failed to open fragment shader file: " + normalized_fs_path);
			return Handle<Shader>();
		}

		const Handle<Shader> handle = create_shader({
			.debug_name = paths,
			.vs_source = vs_source,
			.fs_source = fs_source });

		_path_to_shader[paths] = handle;

		return handle;
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
		if (const Shader* shader = _shader_pool.get(handle)) {
			_bind_program_if_not_already_bound(shader->program_object);
		}
	}

	void unbind_shader()
	{
		_bind_program_if_not_already_bound(0);
	}

	void set_uniform_1f(Handle<Shader> handle, std::string_view name, float x)
	{
		if (Shader* shader = _shader_pool.get(handle)) {
			if (const ShaderUniform* uniform = _get_shader_uniform(shader->uniforms, name)) {
				glProgramUniform1f(shader->program_object, uniform->location, x);
			}
		}
	}

	void set_uniform_2f(Handle<Shader> handle, std::string_view name, float x, float y)
	{
		if (Shader* shader = _shader_pool.get(handle)) {
			if (const ShaderUniform* uniform = _get_shader_uniform(shader->uniforms, name)) {
				glProgramUniform2f(shader->program_object, uniform->location, x, y);
			}
		}
	}

	void set_uniform_3f(Handle<Shader> handle, std::string_view name, float x, float y, float z)
	{
		if (Shader* shader = _shader_pool.get(handle)) {
			if (const ShaderUniform* uniform = _get_shader_uniform(shader->uniforms, name)) {
				glProgramUniform3f(shader->program_object, uniform->location, x, y, z);
			}
		}
	}

	void set_uniform_4f(Handle<Shader> handle, std::string_view name, float x, float y, float z, float w)
	{
		if (Shader* shader = _shader_pool.get(handle)) {
			if (const ShaderUniform* uniform = _get_shader_uniform(shader->uniforms, name)) {
				glProgramUniform4f(shader->program_object, uniform->location, x, y, z, w);
			}
		}
	}

	void set_uniform_1i(Handle<Shader> handle, std::string_view name, int x)
	{
		if (Shader* shader = _shader_pool.get(handle)) {
			if (const ShaderUniform* uniform = _get_shader_uniform(shader->uniforms, name)) {
				glProgramUniform1i(shader->program_object, uniform->location, x);
			}
		}
	}

	void set_uniform_2i(Handle<Shader> handle, std::string_view name, int x, int y)
	{
		if (Shader* shader = _shader_pool.get(handle)) {
			if (const ShaderUniform* uniform = _get_shader_uniform(shader->uniforms, name)) {
				glProgramUniform2i(shader->program_object, uniform->location, x, y);
			}
		}
	}

	void set_uniform_3i(Handle<Shader> handle, std::string_view name, int x, int y, int z)
	{
		if (Shader* shader = _shader_pool.get(handle)) {
			if (const ShaderUniform* uniform = _get_shader_uniform(shader->uniforms, name)) {
				glProgramUniform3i(shader->program_object, uniform->location, x, y, z);
			}
		}
	}

	void set_uniform_4i(Handle<Shader> handle, std::string_view name, int x, int y, int z, int w)
	{
		if (Shader* shader = _shader_pool.get(handle)) {
			if (const ShaderUniform* uniform = _get_shader_uniform(shader->uniforms, name)) {
				glProgramUniform4i(shader->program_object, uniform->location, x, y, z, w);
			}
		}
	}

	void set_uniform_mat4(Handle<Shader> handle, std::string_view name, const float matrix[16])
	{
		if (Shader* shader = _shader_pool.get(handle)) {
			if (const ShaderUniform* uniform = _get_shader_uniform(shader->uniforms, name)) {
				glProgramUniformMatrix4fv(shader->program_object, uniform->location, 1, GL_FALSE, matrix);
			}
		}
	}

	Handle<Buffer> create_buffer(const BufferDesc&& desc)
	{
		GLuint buffer_object = 0;
		glCreateBuffers(1, &buffer_object);
		_set_debug_label(GL_BUFFER, buffer_object, desc.debug_name);

		GLbitfield flags = 0;
		if (desc.dynamic) {
			flags |= GL_DYNAMIC_STORAGE_BIT;
		}
		glNamedBufferStorage(buffer_object, desc.size, desc.initial_data, flags);

		return _buffer_pool.emplace(buffer_object, desc);
	}

	void recreate_buffer(Handle<Buffer> handle, unsigned int size, const void* initial_data)
	{
		Buffer* buffer = _buffer_pool.get(handle);
		if (!buffer) return;
		glDeleteBuffers(1, &buffer->buffer_object);
		glCreateBuffers(1, &buffer->buffer_object);
		_set_debug_label(GL_BUFFER, buffer->buffer_object, buffer->desc.debug_name);
		GLbitfield flags = 0;
		if (buffer->desc.dynamic) {
			flags |= GL_DYNAMIC_STORAGE_BIT;
		}
		glNamedBufferStorage(buffer->buffer_object, size, initial_data, flags);
		buffer->desc.size = size;
	}

	void destroy_buffer(Handle<Buffer> handle)
	{
		Buffer* buffer = _buffer_pool.get(handle);
		if (!buffer) return;
		glDeleteBuffers(1, &buffer->buffer_object);
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
		glNamedBufferSubData(buffer->buffer_object, offset, size, data);
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
		const Buffer* buffer = _buffer_pool.get(handle);
		if (!buffer) return;
		glVertexArrayVertexBuffer(_vertex_array_object, binding, buffer->buffer_object, offset, stride);
	}

	void unbind_vertex_buffer(unsigned int binding)
	{
		glVertexArrayVertexBuffer(_vertex_array_object, binding, 0, 0, 0);
	}

	void bind_index_buffer(Handle<Buffer> handle)
	{
		const Buffer* buffer = _buffer_pool.get(handle);
		if (!buffer) return;
		glVertexArrayElementBuffer(_vertex_array_object, buffer->buffer_object);
	}

	void unbind_index_buffer()
	{
		glVertexArrayElementBuffer(_vertex_array_object, 0);
	}

	void bind_uniform_buffer(unsigned int binding, Handle<Buffer> handle)
	{
		const Buffer* buffer = _buffer_pool.get(handle);
		if (!buffer) return;
		glBindBufferBase(GL_UNIFORM_BUFFER, binding, buffer->buffer_object);
	}

	void bind_uniform_buffer_range(unsigned int binding, Handle<Buffer> handle, unsigned int size, unsigned offset)
	{
		if (size % _uniform_buffer_offset_alignment != 0) return; // must be a multiple of alignment
		const Buffer* buffer = _buffer_pool.get(handle);
		if (!buffer) return;
		if (offset + size > buffer->desc.size) return;
		glBindBufferRange(GL_UNIFORM_BUFFER, binding, buffer->buffer_object, offset, size);
	}

	void unbind_uniform_buffer(unsigned int binding)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, binding, 0);
	}

	GLenum _to_gl_base_format(Format format)
	{
		switch (format) {
		case Format::R8_UNORM:       return GL_RED;
		case Format::R8G8_UNORM:     return GL_RG;
		case Format::R8G8B8_UNORM:   return GL_RGB;
		case Format::R8G8B8A8_UNORM: return GL_RGBA;
		default: return 0;
		}
	}

	GLenum _to_gl_sized_format(Format format)
	{
		switch (format) {
		case Format::R8_UNORM:       return GL_R8;
		case Format::R8G8_UNORM:     return GL_RG8;
		case Format::R8G8B8_UNORM:   return GL_RGB8;
		case Format::R8G8B8A8_UNORM: return GL_RGBA8;
		default: return 0;
		}
	}

	unsigned _get_size(Format format)
	{
		switch (format) {
		case Format::R8_UNORM:       return 1;
		case Format::R8G8_UNORM:     return 2;
		case Format::R8G8B8_UNORM:   return 3;
		case Format::R8G8B8A8_UNORM: return 4;
		default: return 0;
		}
	}

	Handle<Texture> create_texture(const TextureDesc&& desc)
	{
		GLuint texture_object = 0;
		glCreateTextures(GL_TEXTURE_2D, 1, &texture_object);
		_set_debug_label(GL_TEXTURE, texture_object, desc.debug_name);

		const GLenum gl_sized_format = _to_gl_sized_format(desc.format);
		glTextureStorage2D(texture_object, 1, gl_sized_format, desc.width, desc.height);

		if (desc.initial_data) {
			const GLenum gl_base_format = _to_gl_base_format(desc.format);
			glTextureSubImage2D(texture_object, 0, 0, 0, desc.width, desc.height, gl_base_format, GL_UNSIGNED_BYTE, desc.initial_data);
		}

		Texture texture{};
		texture.debug_name = desc.debug_name;
		texture.texture_object = texture_object;
		texture.width = desc.width;
		texture.height = desc.height;
		texture.format = desc.format;
		texture.size = desc.width * desc.height * _get_size(desc.format);

		_total_texture_memory_usage_in_bytes += texture.size;

		return _texture_pool.emplace(std::move(texture));
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
				.format = Format::R8G8B8A8_UNORM, // we assume this format for now
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
		else if (channels == 2) format = Format::R8G8_UNORM;
		else if (channels == 3) format = Format::R8G8B8_UNORM;
		else if (channels == 4) format = Format::R8G8B8A8_UNORM;

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
			const std::string debug_name = src_texture->debug_name + " (copy)";
			dest = create_texture({
				.debug_name = debug_name,
				.width = src_texture->width,
				.height = src_texture->height,
				.format = src_texture->format });
			copy_texture(dest, src);
		}
		return dest;
	}

	void destroy_texture(Handle<Texture> handle)
	{
		Texture* texture = _texture_pool.get(handle);
		if (!texture) return;
		glDeleteTextures(1, &texture->texture_object);
		_total_texture_memory_usage_in_bytes -= texture->size;
		// HACK: When a texture is loaded, its debug_name is set to the path.
		_path_to_texture.erase(texture->debug_name);
		*texture = Texture();
		_texture_pool.free(handle);
	}

	void bind_texture(unsigned int binding, Handle<Texture> handle)
	{
		if (const Texture* texture = _texture_pool.get(handle)) {
			glBindTextureUnit(binding, texture->texture_object);
		}
	}

	void unbind_texture(unsigned int binding)
	{
		glBindTextureUnit(binding, 0);
	}

	void update_texture(Handle<Texture> handle, const unsigned char* data)
	{
		const Texture* texture = _texture_pool.get(handle);
		if (!texture) return;
		const GLenum gl_base_format = _to_gl_base_format(texture->format);
		glTextureSubImage2D(texture->texture_object, 0, 0, 0, texture->width, texture->height, gl_base_format, GL_UNSIGNED_BYTE, data);
	}

	void copy_texture(Handle<Texture> dest, Handle<Texture> src)
	{
		Texture* dest_texture = _texture_pool.get(dest);
		const Texture* src_texture = _texture_pool.get(src);
		if (!dest_texture || !src_texture) return;
		if (dest_texture->width != src_texture->width) return;
		if (dest_texture->height != src_texture->height) return;
		glCopyImageSubData(
			src_texture->texture_object, GL_TEXTURE_2D, 0, 0, 0, 0,
			dest_texture->texture_object, GL_TEXTURE_2D, 0, 0, 0, 0,
			dest_texture->width, dest_texture->height, 1);
	}

	void get_texture_size(Handle<Texture> handle, unsigned int& width, unsigned int& height)
	{
		if (const Texture* texture = _texture_pool.get(handle)) {
			width = texture->width;
			height = texture->height;
		} else {
			width = 0;
			height = 0;
		}
	}

	GLint _to_gl_filter(Filter filter)
	{
		switch (filter) {
		case Filter::Nearest: return GL_NEAREST;
		case Filter::Linear:  return GL_LINEAR;
		default:			  return 0;
		}
	}

	GLint _to_gl_wrap(Wrap wrap)
	{
		switch (wrap) {
		case Wrap::Repeat:            return GL_REPEAT;
		case Wrap::MirroredRepeat:    return GL_MIRRORED_REPEAT;
		case Wrap::ClampToEdge:       return GL_CLAMP_TO_EDGE;
		case Wrap::ClampToBorder:     return GL_CLAMP_TO_BORDER;
		case Wrap::MirrorClampToEdge: return GL_MIRROR_CLAMP_TO_EDGE;
		default:				      return 0;
		}
	}

	Handle<Sampler> create_sampler(const SamplerDesc&& desc)
	{
		GLuint sampler_object = 0;
		glCreateSamplers(1, &sampler_object);
		_set_debug_label(GL_SAMPLER, sampler_object, desc.debug_name);

		const GLint gl_filter = _to_gl_filter(desc.filter);
		glSamplerParameteri(sampler_object, GL_TEXTURE_MIN_FILTER, gl_filter);
		glSamplerParameteri(sampler_object, GL_TEXTURE_MAG_FILTER, gl_filter);

		const GLint gl_wrap = _to_gl_wrap(desc.wrap);
		glSamplerParameteri(sampler_object, GL_TEXTURE_WRAP_S, gl_wrap);
		glSamplerParameteri(sampler_object, GL_TEXTURE_WRAP_T, gl_wrap);

		glSamplerParameterfv(sampler_object, GL_TEXTURE_BORDER_COLOR, desc.border_color);

		return _sampler_pool.emplace(sampler_object, desc);
	}

	void destroy_sampler(Handle<Sampler> handle)
	{
		Sampler* sampler = _sampler_pool.get(handle);
		if (!sampler) return;
		glDeleteSamplers(1, &sampler->sampler_object);
		*sampler = Sampler();
		_sampler_pool.free(handle);
	}

	void bind_sampler(unsigned int binding, Handle<Sampler> handle)
	{
		if (const Sampler* sampler = _sampler_pool.get(handle)) {
			glBindSampler(binding, sampler->sampler_object);
		}
	}

	void unbind_sampler(unsigned int binding)
	{
		glBindSampler(binding, 0);
	}

	Handle<Framebuffer> create_framebuffer(const FramebufferDesc&& desc)
	{
		const std::string texture_debug_name = std::string(desc.debug_name) + " texture";
		const Handle<Texture> texture_handle = create_texture({
			.debug_name = texture_debug_name,
			.width = desc.width,
			.height = desc.height,
			.format = Format::R8G8B8A8_UNORM });
		if (texture_handle == Handle<Texture>()) {
			console::log_error("Failed to create framebuffer texture: " + std::string(desc.debug_name));
			return Handle<Framebuffer>();
		}

		const Texture* texture = _texture_pool.get(texture_handle);
		assert(texture);

		GLuint framebuffer_object = 0;
		glCreateFramebuffers(1, &framebuffer_object);
		_set_debug_label(GL_FRAMEBUFFER, framebuffer_object, desc.debug_name);

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

	void set_viewport(int x, int y, int width, int height)
	{
		glViewport(x, y, width, height);
	}

	void get_viewport(int& x, int& y, int& width, int& height)
	{
		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);
		x = viewport[0];
		y = viewport[1];
		width = viewport[2];
		height = viewport[3];
	}

	void set_scissor_test_enabled(bool enable)
	{
		if (enable) {
			glEnable(GL_SCISSOR_TEST);
		} else {
			glDisable(GL_SCISSOR_TEST);
		}
	}

	bool get_scissor_test_enabled()
	{
		return glIsEnabled(GL_SCISSOR_TEST);
	}

	void set_scissor_box(int x, int y, int width, int height)
	{
		glScissor(x, y, width, height);
	}

	void get_scissor_box(int& x, int& y, int& width, int& height)
	{
		GLint scissor_box[4];
		glGetIntegerv(GL_SCISSOR_BOX, scissor_box);
		x = scissor_box[0];
		y = scissor_box[1];
		width = scissor_box[2];
		height = scissor_box[3];
	}

	GLenum _to_gl_primitives(Primitives primitives)
	{
		switch (primitives) {
		case Primitives::PointList:     return GL_POINTS;
		case Primitives::LineList:      return GL_LINES;
		case Primitives::LineStrip:     return GL_LINE_STRIP;
		case Primitives::TriangleList:  return GL_TRIANGLES;
		case Primitives::TriangleStrip: return GL_TRIANGLE_STRIP;
		default:						return 0; // should never happen
		}
	}

	void draw(Primitives primitives, unsigned int vertex_count, unsigned int vertex_offset)
	{
		if (!vertex_count) return;
		glDrawArrays(_to_gl_primitives(primitives), vertex_offset, vertex_count);
	}

	void draw_indexed(Primitives primitives, unsigned int index_count)
	{
		if (!index_count) return;
		glDrawElements(_to_gl_primitives(primitives), index_count, GL_UNSIGNED_INT, 0);
	}

	void push_debug_group(std::string_view name)
	{
#ifdef _DEBUG_GRAPHICS
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, (GLsizei)name.size(), name.data());
#endif
	}

	void pop_debug_group()
	{
#ifdef _DEBUG_GRAPHICS
		glPopDebugGroup();
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
			if (ImGui::TreeNode(texture.debug_name.c_str())) {
				ImGui::Text("Dimensions: %dx%dx", texture.width, texture.height);
				ImGui::Text("Format: %s", magic_enum::enum_name(texture.format).data());
				unsigned int kb = texture.size / 1024;
				unsigned int mb = kb / 1024;
				if (mb) {
					ImGui::Text("Memory: %d MB", mb);
				} else {
					ImGui::Text("Memory: %d KB", kb);
				}
				ImGui::Image((ImTextureID)(uintptr_t)texture.texture_object,
					ImVec2((float)texture.width, (float)texture.height));
				ImGui::TreePop();
			}
		}
		ImGui::End();
#endif
	}
}