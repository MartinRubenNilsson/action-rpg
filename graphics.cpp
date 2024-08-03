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
		std::string debug_name;
		BufferType type = BufferType::Vertex;
		Usage usage = Usage::StaticDraw;
		unsigned int byte_size = 0;
		GLuint buffer_object = 0;
	};

	struct Texture
	{
		std::string debug_name;
		unsigned int width = 0;
		unsigned int height = 0;
		unsigned int channels = 0;
		unsigned int byte_size = 0; // for debugging
		GLuint texture_object = 0;
		Filter filter = Filter::Nearest;
	};

	struct Framebuffer
	{
		std::string debug_name;
		Handle<Texture> texture;
		GLuint framebuffer_object = 0;
	};

	constexpr unsigned int _MAX_VERTEX_COUNT = 65536;
	constexpr unsigned int _MAX_INDEX_COUNT = 65536;
	GLuint _vertex_array_object = 0;
	GLuint _vertex_buffer_object = 0;
	GLuint _element_buffer_object = 0;
	Pool<Shader> _shader_pool;
	std::unordered_map<std::string, Handle<Shader>> _shader_paths_to_handle;
	Pool<Buffer> _buffer_pool;
	Pool<Texture> _texture_pool;
	std::unordered_map<std::string, Handle<Texture>> _texture_path_to_handle;
	Pool<Framebuffer> _framebuffer_pool;
	std::vector<Handle<Framebuffer>> _temporary_framebuffers;
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

#ifdef DEBUG_GRAPHICS
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
#ifdef DEBUG_GRAPHICS
		glObjectLabel(identifier, name, (GLsizei)label.size(), label.data());
#endif
	}

	void initialize()
	{
#ifdef DEBUG_GRAPHICS
		// HACK: We should be using a post-build event to copy the shaders,
		// but then it doesn't run when only debugging and not recompiling,
		// which is annoying when you've changed a shader but not the code,
		// because then the new shader doesn't get copied.
		system("copy /Y ..\\*.vert .\\assets\\shaders\\");
		system("copy /Y ..\\*.frag .\\assets\\shaders\\");

		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(_debug_message_callback, 0);
#endif
		// CREATE AND BIND VERTEX ARRAY

		glGenVertexArrays(1, &_vertex_array_object);
		glBindVertexArray(_vertex_array_object);

		// ENABLE VERTEX ATTRIBUTES

		glEnableVertexAttribArray(0);
		glVertexAttribFormat(0, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
		glVertexAttribBinding(0, 0);
		glEnableVertexAttribArray(1);
		glVertexAttribFormat(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, offsetof(Vertex, color));
		glVertexAttribBinding(1, 0);
		glEnableVertexAttribArray(2);
		glVertexAttribFormat(2, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, tex_coord));
		glVertexAttribBinding(2, 0);

		// CREATE AND BIND VERTEX BUFFER

		glGenBuffers(1, &_vertex_buffer_object);
		glBindBuffer(GL_ARRAY_BUFFER, _vertex_buffer_object);
		glBufferData(GL_ARRAY_BUFFER, _MAX_VERTEX_COUNT * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

		glBindVertexBuffer(0, _vertex_buffer_object, 0, sizeof(Vertex));

		// CREATE AND BIND ELEMENT BUFFER

		glGenBuffers(1, &_element_buffer_object);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _element_buffer_object);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, _MAX_INDEX_COUNT * sizeof(unsigned int), nullptr, GL_DYNAMIC_DRAW);

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

		// DELETE VERTEX BUFFER OBJECT

		if (_vertex_buffer_object) {
			glDeleteBuffers(1, &_vertex_buffer_object);
			_vertex_buffer_object = 0;
		}

		// DELETE ELEMENT BUFFER OBJECT

		if (_element_buffer_object) {
			glDeleteBuffers(1, &_element_buffer_object);
			_element_buffer_object = 0;
		}

		// DELETE SHADERS

		for (const Shader& shader : _shader_pool.span()) {
			glDeleteProgram(shader.program_object);
		}
		_shader_pool.clear();
		_shader_paths_to_handle.clear();

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
		_texture_path_to_handle.clear();

		// DELETE FRAMEBUFFERS

		for (const Framebuffer& framebuffer : _framebuffer_pool.span()) {
			glDeleteFramebuffers(1, &framebuffer.framebuffer_object);
		}
		_framebuffer_pool.clear();
		_temporary_framebuffers.clear();
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

		if (const auto it = _shader_paths_to_handle.find(paths); it != _shader_paths_to_handle.end()) {
			return it->second;
		}

		// LOAD VERTEX SHADER SOURCE CODE

		std::string vs_source;
		{
			std::ifstream vs_file{ normalized_vs_path };
			if (!vs_file) {
				console::log_error("Failed to open vertex shader file: " + normalized_vs_path);
				return Handle<Shader>();
			}
			vs_source = {
				std::istreambuf_iterator<char>(vs_file),
				std::istreambuf_iterator<char>() };
		}

		// LOAD FRAGMENT SHADER SOURCE CODE

		std::string fs_source;
		{
			std::ifstream fs_file{ normalized_fs_path };
			if (!fs_file) {
				console::log_error("Failed to open fragment shader file: " + normalized_fs_path);
				return Handle<Shader>();
			}
			fs_source = {
				std::istreambuf_iterator<char>(fs_file),
				std::istreambuf_iterator<char>() };
		}

		const Handle<Shader> handle = create_shader({
			.debug_name = paths,
			.vs_source = vs_source,
			.fs_source = fs_source });

		_shader_paths_to_handle[paths] = handle;

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

	GLenum _buffer_type_to_gl(BufferType type)
	{
		switch (type) {
		case BufferType::Vertex:  return GL_ARRAY_BUFFER;
		case BufferType::Index:   return GL_ELEMENT_ARRAY_BUFFER;
		case BufferType::Uniform: return GL_UNIFORM_BUFFER;
		default:					 return 0; // should never happen
		}
	}

	GLenum _usage_to_gl(Usage usage)
	{
		switch (usage) {
		case Usage::StaticDraw:  return GL_STATIC_DRAW;
		case Usage::StaticRead:  return GL_STATIC_READ;
		case Usage::StaticCopy:  return GL_STATIC_COPY;
		case Usage::DynamicDraw: return GL_DYNAMIC_DRAW;
		case Usage::DynamicRead: return GL_DYNAMIC_READ;
		case Usage::DynamicCopy: return GL_DYNAMIC_COPY;
		default:				 return 0; // should never happen
		}
	}

	Handle<Buffer> create_buffer(const BufferDesc&& desc)
	{
		GLuint buffer_object;
		glGenBuffers(1, &buffer_object);
		const GLenum gl_buffer_type = _buffer_type_to_gl(desc.type);
		const GLenum gl_usage = _usage_to_gl(desc.usage);
		glBindBuffer(gl_buffer_type, buffer_object);
		glBufferData(gl_buffer_type, desc.byte_size, desc.initial_data, gl_usage);
		_set_debug_label(GL_BUFFER, buffer_object, desc.debug_name);

		Buffer buffer{};
		buffer.debug_name = desc.debug_name;
		buffer.type = desc.type;
		buffer.usage = desc.usage;
		buffer.byte_size = desc.byte_size;
		buffer.buffer_object = buffer_object;

		return _buffer_pool.emplace(std::move(buffer));
	}

	void update_buffer(Handle<Buffer> handle, const void* data, unsigned int byte_size)
	{
		if (!data || !byte_size) return;
		Buffer* buffer = _buffer_pool.get(handle);
		if (!buffer) return;
		if (byte_size != buffer->byte_size) return;
		const GLenum gl_buffer_type = _buffer_type_to_gl(buffer->type);
		glBindBuffer(gl_buffer_type, buffer->buffer_object);
		glBufferSubData(gl_buffer_type, 0, byte_size, data);
	}

	void bind_uniform_buffer(unsigned int binding, Handle<Buffer> handle)
	{
		const Buffer* buffer = _buffer_pool.get(handle);
		if (!buffer) return;
		if (buffer->type != BufferType::Uniform) return;
		glBindBufferBase(GL_UNIFORM_BUFFER, binding, buffer->buffer_object);
	}

	void unbind_uniform_buffer(unsigned int binding)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, binding, 0);
	}

	GLint _filter_to_gl(Filter filter)
	{
		switch (filter) {
		case Filter::Nearest: return GL_NEAREST;
		case Filter::Linear:  return GL_LINEAR;
		default:			  return 0; // should never happen
		}
	}

	Handle<Texture> create_texture(const TextureDesc&& desc)
	{
		GLenum format = GL_RGBA;
		if (desc.channels == 1) {
			format = GL_RED;
		} else if (desc.channels == 2) {
			format = GL_RG;
		} else if (desc.channels == 3) {
			format = GL_RGB;
		} else if (desc.channels == 4) {
			format = GL_RGBA;
		}

		GLuint texture_object;
		glGenTextures(1, &texture_object);
		glBindTexture(GL_TEXTURE_2D, texture_object);
		const GLint gl_filter = _filter_to_gl(desc.filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter);
		glTexImage2D(GL_TEXTURE_2D, 0, format, desc.width, desc.height, 0, format, GL_UNSIGNED_BYTE, desc.initial_data);
		_set_debug_label(GL_TEXTURE, texture_object, desc.debug_name);

		Texture texture{};
		texture.debug_name = desc.debug_name;
		texture.width = desc.width;
		texture.height = desc.height;
		texture.channels = desc.channels;
		texture.byte_size = desc.width * desc.height * desc.channels;
		texture.texture_object = texture_object;
		texture.filter = desc.filter;

		_total_texture_memory_usage_in_bytes += texture.byte_size;

		return _texture_pool.emplace(std::move(texture));
	}

	Handle<Texture> load_texture(const std::string& path)
	{
		const std::string normalized_path = filesystem::get_normalized_path(path);

#if 1
		// First, attempt to load the texture from a KTX2 file, if it exists.
		if (const std::string normalized_path_ktx2 = filesystem::replace_extension(normalized_path, ".ktx2");
			filesystem::file_exists(normalized_path_ktx2)) {

			if (const auto it = _texture_path_to_handle.find(normalized_path_ktx2); it != _texture_path_to_handle.end()) {
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
				.channels = 4, // we assume 4 channels (RGBA) for now
				.initial_data = ktx_texture->pData });

			ktxTexture_Destroy(ktxTexture(ktx_texture));
			_texture_path_to_handle[normalized_path_ktx2] = handle;
			return handle;
		}
#endif

		if (const auto it = _texture_path_to_handle.find(normalized_path); it != _texture_path_to_handle.end()) {
			return it->second;
		}

		int width, height, channels;
		unsigned char* data = stbi_load(normalized_path.c_str(), &width, &height, &channels, 0);
		if (!data) {
			console::log_error("Failed to load texture: " + normalized_path);
			console::log_error(stbi_failure_reason());
			return Handle<Texture>();
		}

		const Handle<Texture> handle = create_texture({
			.debug_name = normalized_path,
			.width = (unsigned int)width,
			.height = (unsigned int)height,
			.channels = (unsigned int)channels,
			.initial_data = data });

		stbi_image_free(data);
		_texture_path_to_handle[normalized_path] = handle;
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
				.channels = src_texture->channels });
			copy_texture(dest, src);
		}
		return dest;
	}

	void destroy_texture(Handle<Texture> handle)
	{
		Texture* texture = _texture_pool.get(handle);
		if (!texture) return;
		glDeleteTextures(1, &texture->texture_object);
		_total_texture_memory_usage_in_bytes -= texture->byte_size;
		_texture_path_to_handle.erase(texture->debug_name);
		_texture_pool.free(handle);
		*texture = Texture();
	}

	void bind_texture(unsigned int binding, Handle<Texture> handle)
	{
		if (const Texture* texture = _texture_pool.get(handle)) {
			glActiveTexture(GL_TEXTURE0 + binding);
			glBindTexture(GL_TEXTURE_2D, texture->texture_object);
		}
	}

	void unbind_texture(unsigned int binding)
	{
		glActiveTexture(GL_TEXTURE0 + binding);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void update_texture(Handle<Texture> handle, const unsigned char* data)
	{
		const Texture* texture = _texture_pool.get(handle);
		if (!texture) return;
		glBindTexture(GL_TEXTURE_2D, texture->texture_object);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture->width, texture->height, GL_RED, GL_UNSIGNED_BYTE, data);
	}

	void copy_texture(Handle<Texture> dest, Handle<Texture> src)
	{
		Texture* dest_texture = _texture_pool.get(dest);
		Texture* src_texture = _texture_pool.get(src);
		if (!dest_texture || !src_texture) return;
		if (dest_texture->width != src_texture->width) return;
		if (dest_texture->height != src_texture->height) return;
		glBindTexture(GL_TEXTURE_2D, dest_texture->texture_object);
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

	void set_texture_filter(Handle<Texture> handle, Filter filter)
	{
		Texture* texture = _texture_pool.get(handle);
		if (!texture) return;
		if (texture->filter == filter) return;
		texture->filter = filter;
		const GLint gl_filter = _filter_to_gl(filter);
		glBindTexture(GL_TEXTURE_2D, texture->texture_object);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter);
	}

	Filter get_texture_filter(Handle<Texture> handle)
	{
		const Texture* texture = _texture_pool.get(handle);
		return texture ? texture->filter : Filter::Nearest;
	}

	Handle<Framebuffer> create_framebuffer(const FramebufferDesc&& desc)
	{
		// CREATE TEXTURE

		const std::string texture_debug_name = std::string(desc.debug_name) + " texture";
		const Handle<Texture> texture_handle = create_texture({
			.debug_name = texture_debug_name,
			.width = desc.width,
			.height = desc.height });

		const Texture* texture = _texture_pool.get(texture_handle);
		if (!texture) {
			console::log_error("Failed to create framebuffer: " + std::string(desc.debug_name));
			return Handle<Framebuffer>();
		}

		// SAVE CURRENT FRAMEBUFFER OBJECT

		int previously_bound_framebuffer_object;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previously_bound_framebuffer_object);

		// CREATE FRAMEBUFFER OBJECT

		GLuint framebuffer_object;
		glGenFramebuffers(1, &framebuffer_object);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_object);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->texture_object, 0);
		const GLenum framebuffer_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

		// RESTORE PREVIOUS FRAMEBUFFER OBJECT

		glBindFramebuffer(GL_FRAMEBUFFER, previously_bound_framebuffer_object);

		// CHECK COMPLETENESS

		if (framebuffer_status != GL_FRAMEBUFFER_COMPLETE) {
			console::log_error("Failed to create framebuffer: " + std::string(desc.debug_name));
			glDeleteFramebuffers(1, &framebuffer_object);
			destroy_texture(texture_handle);
			return Handle<Framebuffer>();
		}

		// STORE RENDER TARGET

		Framebuffer framebuffer{};
		framebuffer.debug_name = desc.debug_name;
		framebuffer.texture = texture_handle;
		framebuffer.framebuffer_object = framebuffer_object;
		_set_debug_label(GL_FRAMEBUFFER, framebuffer_object, desc.debug_name);

		return  _framebuffer_pool.emplace(std::move(framebuffer));
	}

	Handle<Framebuffer> aquire_temporary_framebuffer(unsigned int width, unsigned int height)
	{
		for (size_t i = 0; i < _temporary_framebuffers.size(); ++i) {
			const Handle<Framebuffer> handle = _temporary_framebuffers[i];
			const Framebuffer* framebuffer = _framebuffer_pool.get(handle);
			if (!framebuffer) continue;
			const Texture* texture = _texture_pool.get(framebuffer->texture);
			if (!texture) continue;
			if (texture->width != width) continue;
			if (texture->height != height) continue;
			std::swap(_temporary_framebuffers[i], _temporary_framebuffers.back());
			_temporary_framebuffers.pop_back();
			return handle;
		}
		std::string debug_name = "temporary framebuffer " + std::to_string(width) + "x" + std::to_string(height);
		return create_framebuffer({ .debug_name = debug_name, .width = width, .height = height });
	}

	void release_temporary_framebuffer(Handle<Framebuffer> handle)
	{
		_temporary_framebuffers.push_back(handle);
	}

	void bind_window_framebuffer()
	{
		// The default framebuffer is the framebuffer of the window and has name 0.
		// It is created at the same time as the OpenGL context.
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void bind_framebuffer(Handle<Framebuffer> handle)
	{
		if (const Framebuffer* framebuffer = _framebuffer_pool.get(handle)) {
			glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->framebuffer_object);
		}
	}

	void clear_framebuffer(float r, float g, float b, float a)
	{
		glClearColor(r, g, b, a);
		glClear(GL_COLOR_BUFFER_BIT);
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

	GLenum _primitives_to_gl(Primitives primitives)
	{
		switch (primitives) {
		case Primitives::PointList:     return GL_POINTS;
		case Primitives::LineList:      return GL_LINES;
		case Primitives::LineStrip:     return GL_LINE_STRIP;
		case Primitives::TriangleList:  return GL_TRIANGLES;
		case Primitives::TriangleStrip: return GL_TRIANGLE_STRIP;
		default:							   return 0; // should never happen
		}
	}

	void draw(Primitives primitives, unsigned int vertex_count)
	{
		if (!vertex_count) return;
		vertex_count = std::min(vertex_count, _MAX_VERTEX_COUNT);
		glDrawArrays(_primitives_to_gl(primitives), 0, vertex_count);
	}

	void draw(Primitives primitives, const Vertex* vertices, unsigned int vertex_count)
	{
		if (!vertices || !vertex_count) return;
		vertex_count = std::min(vertex_count, _MAX_VERTEX_COUNT);
		glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_count * sizeof(Vertex), vertices);
		glDrawArrays(_primitives_to_gl(primitives), 0, vertex_count);
	}

	void draw(Primitives primitives, const Vertex* vertices, unsigned int vertex_count, unsigned int* indices, unsigned int index_count)
	{
		if (!vertices || !vertex_count || !indices || !index_count) return;
		vertex_count = std::min(vertex_count, _MAX_VERTEX_COUNT);
		index_count = std::min(index_count, _MAX_INDEX_COUNT);
		glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_count * sizeof(Vertex), vertices);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, index_count * sizeof(unsigned int), indices);
		glDrawElements(_primitives_to_gl(primitives), index_count, GL_UNSIGNED_INT, nullptr);
	}

	void push_debug_group(std::string_view name)
	{
#ifdef DEBUG_GRAPHICS
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, (GLsizei)name.size(), name.data());
#endif
	}

	void pop_debug_group()
	{
#ifdef DEBUG_GRAPHICS
		glPopDebugGroup();
#endif
	}

	void show_texture_debug_window()
	{
#ifdef DEBUG_IMGUI
		ImGui::Begin("Textures");
		ImGui::Text("Total memory usage: %d MB", _total_texture_memory_usage_in_bytes / 1024 / 1024);
		for (size_t i = 0; i < _texture_pool.size(); ++i) {
			const Texture& texture = _texture_pool.data()[i];
			if (texture.texture_object == 0) continue;
			if (ImGui::TreeNode(texture.debug_name.c_str())) {
				ImGui::Text("Dimensions: %dx%dx%d", texture.width, texture.height, texture.channels);
				unsigned int kb = texture.byte_size / 1024;
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