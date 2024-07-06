#include "stdafx.h"
#include "graphics.h"
#include "console.h"
#include <glad/glad.h>
#pragma comment(lib, "opengl32")
#include <ktx.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace graphics
{
	extern const float IDENTITY_MATRIX[16] = {
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f,
	};

	RenderTargetHandle window_render_target = RenderTargetHandle::Invalid;
	ShaderHandle sprite_shader = ShaderHandle::Invalid;
	ShaderHandle fullscreen_shader = ShaderHandle::Invalid;
	ShaderHandle shape_shader = ShaderHandle::Invalid;
	ShaderHandle ui_shader = ShaderHandle::Invalid;

	constexpr GLsizei _UNIFORM_NAME_MAX_SIZE = 64;

	struct ShaderUniform
	{
		char name[_UNIFORM_NAME_MAX_SIZE] = { 0 };
		GLsizei name_size = 0;
		GLint location = -1;
	};

	struct Shader
	{
		ShaderHandle handle = ShaderHandle::Invalid;
		std::string name; // unique name
		std::vector<ShaderUniform> uniforms;
		GLuint program_object = 0;
	};

	struct Texture
	{
		TextureHandle handle = TextureHandle::Invalid;
		std::string name; // unique name
		unsigned int width = 0;
		unsigned int height = 0;
		unsigned int channels = 0;
		unsigned int bytes = 0; // for debugging
		GLuint texture_object = 0;
		TextureFilter filter = TextureFilter::Nearest;
	};

	struct RenderTarget
	{
		RenderTargetHandle handle = RenderTargetHandle::Invalid;
		std::string name; // unique name
		TextureHandle texture = TextureHandle::Invalid;
		GLuint framebuffer_object = 0;
	};

	constexpr unsigned int _MAX_VERTEX_COUNT = 65536;
	constexpr unsigned int _MAX_INDEX_COUNT = 65536;
	GLuint _vertex_array_object = 0;
	GLuint _vertex_buffer_object = 0;
	GLuint _element_buffer_object = 0;
	std::vector<Shader> _shaders;
	std::unordered_map<std::string, ShaderHandle> _shader_name_to_handle;
	std::vector<Texture> _textures;
	std::vector<TextureHandle> _free_texture_handles;
	std::unordered_map<std::string, TextureHandle> _texture_name_to_handle;
	std::vector<RenderTarget> _render_targets;
	std::unordered_map<std::string, RenderTargetHandle> _render_target_name_to_handle;
	std::vector<RenderTargetHandle> _pooled_render_targets;
	GLuint _last_bound_program_object = 0;
	unsigned int _total_texture_memory_usage_in_bytes = 0;

	template <class NameContainer>
	std::string _generate_unique_name(const NameContainer& name_container, const std::string& name_hint)
	{
		std::string name = name_hint;
		for (int i = 1; name_container.contains(name); i++) {
			name = name_hint + " (" + std::to_string(i) + ")";
		}
		return name;
	}

	const ShaderUniform* _get_shader_uniform(const std::vector<ShaderUniform> &uniforms, std::string_view name)
	{
		for (const ShaderUniform& uniform : uniforms) {
			if (name.size() != uniform.name_size) continue;
			if (memcmp(name.data(), uniform.name, uniform.name_size)) continue;
			return &uniform;
		}
		return nullptr;
	}

	Shader* _get_shader(ShaderHandle handle)
	{
		const unsigned int index = get_handle_index(handle);
		if (index >= (unsigned int)_shaders.size()) return nullptr;
		if (_shaders[index].handle != handle) return nullptr;
		return &_shaders[index];
	}

	Texture* _get_texture(TextureHandle handle)
	{
		const unsigned int index = get_handle_index(handle);
		if (index >= (unsigned int)_textures.size()) return nullptr;
		if (_textures[index].handle != handle) return nullptr;
		return &_textures[index];
	}

	RenderTarget* _get_render_target(RenderTargetHandle handle)
	{
		const unsigned int index = get_handle_index(handle);
		if (index >= (unsigned int)_textures.size()) return nullptr;
		if (_render_targets[index].handle != handle) return nullptr;
		return &_render_targets[index];
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
		console::log_error(std::format("OpenGL: {}", message));
	}
#endif

	void _set_debug_label(GLenum identifier, GLuint name, const std::string& label)
	{
#ifdef DEBUG_GRAPHICS
		glObjectLabel(identifier, name, (GLsizei)label.size(), label.c_str());
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
		// CREATE AND BIND VERTEX ARRAY OBJECT

		glGenVertexArrays(1, &_vertex_array_object);
		glBindVertexArray(_vertex_array_object);

		// CREATE AND BIND VERTEX BUFFER OBJECT

		glGenBuffers(1, &_vertex_buffer_object);
		glBindBuffer(GL_ARRAY_BUFFER, _vertex_buffer_object);
		glBufferData(GL_ARRAY_BUFFER, _MAX_VERTEX_COUNT * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)offsetof(Vertex, color));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex_coord));	

		// CREATE AND BIND ELEMENT BUFFER OBJECT

		glGenBuffers(1, &_element_buffer_object);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _element_buffer_object);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, _MAX_INDEX_COUNT * sizeof(unsigned int), nullptr, GL_DYNAMIC_DRAW);

		// SETUP BLENDING

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// CREATE WINDOW RENDER TARGET
		{
			window_render_target = (RenderTargetHandle)_render_targets.size();

			RenderTarget& render_target = _render_targets.emplace_back();
			render_target.handle = window_render_target;
			render_target.name = "window render target";
			// The window has both a front and back buffer, so there's no point
			// in setting a single texture for its render target.
			render_target.texture = TextureHandle::Invalid;
			render_target.framebuffer_object = 0;

			_render_target_name_to_handle[render_target.name] = window_render_target;
		}

		// LOAD DEFAULT SHADERS

		sprite_shader = load_shader(
			"assets/shaders/sprite.vert",
			"assets/shaders/sprite.frag");
		fullscreen_shader = load_shader(
			"assets/shaders/fullscreen_triangle.vert",
			"assets/shaders/fullscreen.frag");
		shape_shader = load_shader(
			"assets/shaders/shape.vert",
			"assets/shaders/shape.frag");
		ui_shader = load_shader(
			"assets/shaders/ui.vert",
			"assets/shaders/ui.frag");
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

		for (Shader& shader : _shaders) {
			glDeleteProgram(shader.program_object);
		}
		_shaders.clear();
		_shader_name_to_handle.clear();

		// DELETE TEXTURES

		for (Texture& texture : _textures) {
			glDeleteTextures(1, &texture.texture_object);
		}
		_textures.clear();
		_texture_name_to_handle.clear();

		// DELETE RENDER TEXTURES

		for (RenderTarget& render_texture : _render_targets) {
			glDeleteFramebuffers(1, &render_texture.framebuffer_object);
		}
		_render_targets.clear();
		_render_target_name_to_handle.clear();
	}

	ShaderHandle create_shader(
		const std::string& vertex_shader_bytecode,
		const std::string& fragment_shader_bytecode,
		const std::string& name_hint)
	{
		const GLuint program_object = glCreateProgram();

		{
			// COMPILE VERTEX SHADER

			const GLuint vertex_shader_object = glCreateShader(GL_VERTEX_SHADER);
			const char* vertex_shader_string = vertex_shader_bytecode.c_str();
			glShaderSource(vertex_shader_object, 1, &vertex_shader_string, nullptr);
			glCompileShader(vertex_shader_object);
			int success;
			glGetShaderiv(vertex_shader_object, GL_COMPILE_STATUS, &success);
			if (!success) {
				char info_log[512];
				glGetShaderInfoLog(vertex_shader_object, sizeof(info_log), nullptr, info_log);
				console::log_error("Failed to compile vertex shader: " + name_hint);
				console::log_error(info_log);
				glDeleteShader(vertex_shader_object);
				glDeleteProgram(program_object);
				return ShaderHandle::Invalid;
			}

			// ATTACH VERTEX SHADER

			glAttachShader(program_object, vertex_shader_object);
			glDeleteShader(vertex_shader_object);
		}

		{
			// COMPILE FRAGMENT SHADER

			const GLuint fragment_shader_object = glCreateShader(GL_FRAGMENT_SHADER);
			const char* fragment_shader_string = fragment_shader_bytecode.c_str();
			glShaderSource(fragment_shader_object, 1, &fragment_shader_string, nullptr);
			glCompileShader(fragment_shader_object);
			int success;
			glGetShaderiv(fragment_shader_object, GL_COMPILE_STATUS, &success);
			if (!success) {
				char info_log[512];
				glGetShaderInfoLog(fragment_shader_object, sizeof(info_log), nullptr, info_log);
				console::log_error("Failed to compile fragment shader: " + name_hint);
				console::log_error(info_log);
				glDeleteShader(fragment_shader_object);
				glDeleteProgram(program_object);
				return ShaderHandle::Invalid;
			}

			// ATTACH FRAGMENT SHADER

			glAttachShader(program_object, fragment_shader_object);
			glDeleteShader(fragment_shader_object);
		}

		// LINK PROGRAM OBJECT
		{
			glLinkProgram(program_object);
			int success;
			glGetProgramiv(program_object, GL_LINK_STATUS, &success);
			if (!success) {
				char info_log[512];
				glGetProgramInfoLog(program_object, sizeof(info_log), nullptr, info_log);
				console::log_error("Failed to link program object: " + name_hint);
				console::log_error(info_log);
				glDeleteProgram(program_object);
				return ShaderHandle::Invalid;
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

		const ShaderHandle handle = (ShaderHandle)_shaders.size();

		Shader& shader = _shaders.emplace_back();
		shader.handle = handle;
		shader.name = _generate_unique_name(_shader_name_to_handle, name_hint);
		shader.uniforms = std::move(uniform_locations);
		shader.program_object = program_object;
		_set_debug_label(GL_PROGRAM, program_object, shader.name);

		_shader_name_to_handle[shader.name] = handle;

		return handle;
	}

	ShaderHandle load_shader(const std::string& vertex_shader_path, const std::string& fragment_shader_path)
	{
		// CHECK CACHE FOR EXISTING SHADER
		
		const std::string name = vertex_shader_path + ":" + fragment_shader_path;
		{
			const auto it = _shader_name_to_handle.find(name);
			if (it != _shader_name_to_handle.end()) {
				return it->second;
			}
		}

		// LOAD VERTEX SHADER BYTECODE

		std::string vertex_shader_bytecode;
		{
			std::ifstream vertex_shader_file{ vertex_shader_path };
			if (!vertex_shader_file) {
				console::log_error("Failed to open vertex shader file: " + vertex_shader_path);
				return ShaderHandle::Invalid;
			}
			vertex_shader_bytecode = {
				std::istreambuf_iterator<char>(vertex_shader_file),
				std::istreambuf_iterator<char>() };
		}

		// LOAD FRAGMENT SHADER BYTECODE

		std::string fragment_shader_bytecode;
		{
			std::ifstream fragment_shader_file{ fragment_shader_path };
			if (!fragment_shader_file) {
				console::log_error("Failed to open fragment shader file: " + fragment_shader_path);
				return ShaderHandle::Invalid;
			}
			fragment_shader_bytecode = {
				std::istreambuf_iterator<char>(fragment_shader_file),
				std::istreambuf_iterator<char>() };
		}

		// CREATE SHADER

		return create_shader(vertex_shader_bytecode, fragment_shader_bytecode, name);
	}

	void _bind_program_if_not_already_bound(GLuint program_object)
	{
		// OpenGL does not guard agains redundant state changes,
		// so if we do it ourselves we get a performance boost.
		if (_last_bound_program_object == program_object) return;
		glUseProgram(program_object);
		_last_bound_program_object = program_object;
	}

	void bind_shader(ShaderHandle handle)
	{
		if (const Shader* shader = _get_shader(handle)) {
			_bind_program_if_not_already_bound(shader->program_object);
		}
	}

	void unbind_shader()
	{
		_bind_program_if_not_already_bound(0);
	}

	void set_uniform_1f(ShaderHandle handle, std::string_view name, float x)
	{
		if (Shader* shader = _get_shader(handle)) {
			if (const ShaderUniform* uniform = _get_shader_uniform(shader->uniforms, name)) {
				glProgramUniform1f(shader->program_object, uniform->location, x);
			}
		}
	}

	void set_uniform_2f(ShaderHandle handle, std::string_view name, float x, float y)
	{
		if (Shader* shader = _get_shader(handle)) {
			if (const ShaderUniform* uniform = _get_shader_uniform(shader->uniforms, name)) {
				glProgramUniform2f(shader->program_object, uniform->location, x, y);
			}
		}
	}

	void set_uniform_3f(ShaderHandle handle, std::string_view name, float x, float y, float z)
	{
		if (Shader* shader = _get_shader(handle)) {
			if (const ShaderUniform* uniform = _get_shader_uniform(shader->uniforms, name)) {
				glProgramUniform3f(shader->program_object, uniform->location, x, y, z);
			}
		}
	}

	void set_uniform_4f(ShaderHandle handle, std::string_view name, float x, float y, float z, float w)
	{
		if (Shader* shader = _get_shader(handle)) {
			if (const ShaderUniform* uniform = _get_shader_uniform(shader->uniforms, name)) {
				glProgramUniform4f(shader->program_object, uniform->location, x, y, z, w);
			}
		}
	}

	void set_uniform_1i(ShaderHandle handle, std::string_view name, int x)
	{
		if (Shader* shader = _get_shader(handle)) {
			if (const ShaderUniform* uniform = _get_shader_uniform(shader->uniforms, name)) {
				glProgramUniform1i(shader->program_object, uniform->location, x);
			}
		}
	}

	void set_uniform_2i(ShaderHandle handle, std::string_view name, int x, int y)
	{
		if (Shader* shader = _get_shader(handle)) {
			if (const ShaderUniform* uniform = _get_shader_uniform(shader->uniforms, name)) {
				glProgramUniform2i(shader->program_object, uniform->location, x, y);
			}
		}
	}

	void set_uniform_3i(ShaderHandle handle, std::string_view name, int x, int y, int z)
	{
		if (Shader* shader = _get_shader(handle)) {
			if (const ShaderUniform* uniform = _get_shader_uniform(shader->uniforms, name)) {
				glProgramUniform3i(shader->program_object, uniform->location, x, y, z);
			}
		}
	}

	void set_uniform_4i(ShaderHandle handle, std::string_view name, int x, int y, int z, int w)
	{
		if (Shader* shader = _get_shader(handle)) {
			if (const ShaderUniform* uniform = _get_shader_uniform(shader->uniforms, name)) {
				glProgramUniform4i(shader->program_object, uniform->location, x, y, z, w);
			}
		}
	}

	void set_uniform_mat4(ShaderHandle handle, std::string_view name, const float matrix[16])
	{
		if (Shader* shader = _get_shader(handle)) {
			if (const ShaderUniform* uniform = _get_shader_uniform(shader->uniforms, name)) {
				glProgramUniformMatrix4fv(shader->program_object, uniform->location, 1, GL_FALSE, matrix);
			}
		}
	
	}

	TextureHandle create_texture(
		unsigned int width,
		unsigned int height,
		unsigned int channels,
		const unsigned char* data,
		const std::string& name_hint)
	{
		// DETERMINE FORMAT

		GLenum format = GL_RGBA;
		if (channels == 1) {
			format = GL_RED;
		} else if (channels == 2) {
			format = GL_RG;
		} else if (channels == 3) {
			format = GL_RGB;
		} else if (channels == 4) {
			format = GL_RGBA;
		}

		// CREATE TEXTURE OBJECT

		GLuint texture_object;
		glGenTextures(1, &texture_object);
		glBindTexture(GL_TEXTURE_2D, texture_object);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

		// STORE TEXTURE

		TextureHandle handle = TextureHandle::Invalid;
		Texture* texture = nullptr;

		if (_free_texture_handles.empty()) {
			handle = (TextureHandle)_textures.size();
			texture = &_textures.emplace_back();
		} else {
			handle = _free_texture_handles.back();
			_free_texture_handles.pop_back();
			texture = &_textures[get_handle_index(handle)];
		}

		texture->handle = handle;
		texture->name = _generate_unique_name(_texture_name_to_handle, name_hint);
		texture->width = width;
		texture->height = height;
		texture->channels = channels;
		texture->bytes = width * height * channels;
		texture->texture_object = texture_object;
		_set_debug_label(GL_TEXTURE, texture_object, texture->name);
		texture->filter = TextureFilter::Nearest;

		_texture_name_to_handle[texture->name] = handle;
		_total_texture_memory_usage_in_bytes += texture->bytes;

		return handle;
	}

	TextureHandle load_texture(const std::string& path, bool flip_y)
	{
		// CHECK IF ALREADY LOADED

		const std::string normalized_path = std::filesystem::path(path).lexically_normal().string();
		const auto it = _texture_name_to_handle.find(normalized_path);
		if (it != _texture_name_to_handle.end()) {
			return it->second;
		}

		// LOAD TEXTURE DATA

		const int was_flip_on_load = stbi__vertically_flip_on_load_global;
		stbi__vertically_flip_on_load_global = flip_y;
		int width, height, channels;
		unsigned char* data = stbi_load(normalized_path.c_str(), &width, &height, &channels, 0);
		stbi__vertically_flip_on_load_global = was_flip_on_load;
		if (!data) {
			console::log_error("Failed to load texture: " + normalized_path);
			console::log_error(stbi_failure_reason());
			return TextureHandle::Invalid;
		}

		// CREATE TEXTURE

		return create_texture(width, height, channels, data, normalized_path);
	}

	TextureHandle copy_texture(TextureHandle handle)
	{
		// This fixes a bug where texture would become a dangling pointer
		// if create_texture() causes _textures to reallocate.
		_textures.reserve(_textures.size() + 1);

		const Texture* texture = _get_texture(handle);
		if (!texture) return TextureHandle::Invalid;

		const TextureHandle copy_handle = create_texture(
			texture->width,
			texture->height,
			texture->channels,
			nullptr,
			texture->name + " (copy)");
		const Texture* copy_texture = _get_texture(copy_handle);
		if (!copy_texture) return TextureHandle::Invalid;

		glCopyImageSubData(
			texture->texture_object, GL_TEXTURE_2D, 0, 0, 0, 0,
			copy_texture->texture_object, GL_TEXTURE_2D, 0, 0, 0, 0,
			texture->width, texture->height, 1);

		return copy_handle;
	}

	void destroy_texture(TextureHandle handle)
	{
		Texture* texture = _get_texture(handle);
		if (!texture) return;
		_texture_name_to_handle.erase(texture->name);
		glDeleteTextures(1, &texture->texture_object);
		*texture = Texture();
		_free_texture_handles.push_back(increment_handle_generation(handle));
		_total_texture_memory_usage_in_bytes -= texture->bytes;
	}

	void bind_texture(unsigned int texture_unit, TextureHandle handle)
	{
		if (const Texture* texture = _get_texture(handle)) {
			glActiveTexture(GL_TEXTURE0 + texture_unit);
			glBindTexture(GL_TEXTURE_2D, texture->texture_object);
		}
	}

	void unbind_texture(unsigned int texture_unit)
	{
		glActiveTexture(GL_TEXTURE0 + texture_unit);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void get_texture_size(TextureHandle handle, unsigned int& width, unsigned int& height)
	{
		if (const Texture* texture = _get_texture(handle)) {
			width = texture->width;
			height = texture->height;
		} else {
			width = 0;
			height = 0;
		}
	}

	void set_texture_filter(TextureHandle handle, TextureFilter filter)
	{
		Texture* texture = _get_texture(handle);
		if (!texture) return;
		if (texture->filter == filter) return;
		texture->filter = filter;
		const GLint gl_filter = (filter == TextureFilter::Nearest) ? GL_NEAREST : GL_LINEAR;
		glBindTexture(GL_TEXTURE_2D, texture->texture_object);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter);
	}

	TextureFilter get_texture_filter(TextureHandle handle)
	{
		const Texture* texture = _get_texture(handle);
		if (!texture) return TextureFilter::Nearest;
		return texture->filter;
	}

	RenderTargetHandle create_render_target(
		unsigned int width,
		unsigned int height,
		const std::string& name_hint)
	{
		// CREATE TEXTURE

		const TextureHandle texture_handle = create_texture(width, height, 4, nullptr, name_hint);
		const Texture* texture = _get_texture(texture_handle);
		if (!texture) {
			console::log_error("Failed to create render target: " + name_hint);
			return RenderTargetHandle::Invalid;
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
			console::log_error("Failed to create render target: " + name_hint);
			glDeleteFramebuffers(1, &framebuffer_object);
			destroy_texture(texture_handle);
			return RenderTargetHandle::Invalid;
		}

		// STORE RENDER TARGET

		const RenderTargetHandle target_handle = (RenderTargetHandle)_render_targets.size();

		RenderTarget& render_target = _render_targets.emplace_back();
		render_target.handle = target_handle;
		render_target.name = _generate_unique_name(_render_target_name_to_handle, name_hint);
		render_target.texture = texture_handle;
		render_target.framebuffer_object = framebuffer_object;
		_set_debug_label(GL_FRAMEBUFFER, framebuffer_object, render_target.name);

		_render_target_name_to_handle[render_target.name] = target_handle;

		return target_handle;
	}

	RenderTargetHandle acquire_pooled_render_target(unsigned int width, unsigned int height)
	{
		for (size_t i = 0; i < _pooled_render_targets.size(); ++i) {
			const RenderTargetHandle handle = _pooled_render_targets[i];
			const RenderTarget* render_target = _get_render_target(handle);
			if (!render_target) continue;
			const Texture* texture = _get_texture(render_target->texture);
			if (!texture) continue;
			if (texture->width != width) continue;
			if (texture->height != height) continue;
			std::swap(_pooled_render_targets[i], _pooled_render_targets.back());
			_pooled_render_targets.pop_back();
			return handle;
		}
		std::string name_hint = "pooled render target " + std::to_string(width) + "x" + std::to_string(height);
		return create_render_target(width, height, name_hint);
	}

	void release_pooled_render_target(RenderTargetHandle handle)
	{
		_pooled_render_targets.push_back(handle);
	}

	void bind_render_target(RenderTargetHandle handle)
	{
		if (const RenderTarget* render_target = _get_render_target(handle)) {
			glBindFramebuffer(GL_FRAMEBUFFER, render_target->framebuffer_object);
		}
	}

	void clear_render_target(float r, float g, float b, float a)
	{
		glClearColor(r, g, b, a);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	TextureHandle get_render_target_texture(RenderTargetHandle handle)
	{
		if (const RenderTarget* render_target = _get_render_target(handle)) {
			return render_target->texture;
		}
		return TextureHandle::Invalid;
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

	void draw_lines(const Vertex* vertices, unsigned int vertex_count)
	{
		if (!vertices || !vertex_count) return;
		vertex_count = std::min(vertex_count, _MAX_VERTEX_COUNT);
		glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_count * sizeof(Vertex), vertices);
		glDrawArrays(GL_LINES, 0, vertex_count);
	}

	void draw_line_strip(const Vertex* vertices, unsigned int vertex_count)
	{
		if (!vertices || !vertex_count) return;
		vertex_count = std::min(vertex_count, _MAX_VERTEX_COUNT);
		glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_count * sizeof(Vertex), vertices);
		glDrawArrays(GL_LINE_STRIP, 0, vertex_count);
	}

	void draw_line_loop(const Vertex* vertices, unsigned int vertex_count)
	{
		if (!vertices || !vertex_count) return;
		vertex_count = std::min(vertex_count, _MAX_VERTEX_COUNT);
		glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_count * sizeof(Vertex), vertices);
		glDrawArrays(GL_LINE_LOOP, 0, vertex_count);
	}

	void draw_triangle_strip(unsigned int vertex_count)
	{
		if (!vertex_count) return;
		vertex_count = std::min(vertex_count, _MAX_VERTEX_COUNT);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, vertex_count);
	}

	void draw_triangle_strip(const Vertex* vertices, unsigned int vertex_count)
	{
		if (!vertices || !vertex_count) return;
		vertex_count = std::min(vertex_count, _MAX_VERTEX_COUNT);
		glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_count * sizeof(Vertex), vertices);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, vertex_count);
	}

	void draw_triangles(unsigned int vertex_count)
	{
		if (!vertex_count) return;
		vertex_count = std::min(vertex_count, _MAX_VERTEX_COUNT);
		glDrawArrays(GL_TRIANGLES, 0, vertex_count);
	}

	void draw_triangles(
		const Vertex* vertices, unsigned int vertex_count,
		unsigned int* indices, unsigned int index_count)
	{
		if (!vertices || !vertex_count || !indices || !index_count) return;
		vertex_count = std::min(vertex_count, _MAX_VERTEX_COUNT);
		index_count = std::min(index_count, _MAX_INDEX_COUNT);
		glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_count * sizeof(Vertex), vertices);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, index_count * sizeof(unsigned int), indices);
		glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, nullptr);
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
		for (const Texture& texture : _textures) {
			if (texture.handle == TextureHandle::Invalid) continue;
			if (ImGui::TreeNode(texture.name.c_str())) {
				ImGui::Text("Dimensions: %dx%dx%d", texture.width, texture.height, texture.channels);
				unsigned int kb = texture.bytes / 1024;
				unsigned int mb = kb / 1024;
				if (mb) {
					ImGui::Text("Memory: %d MB", mb);
				} else {
					ImGui::Text("Memory: %d KB", kb);
				}
				ImGui::Image((ImTextureID)(uintptr_t)texture.texture_object,
					ImVec2((float)texture.width, (float)texture.height),
					ImVec2(0.f, 1.f), ImVec2(1.f, 0.f));
				ImGui::TreePop();
			}
		}
		ImGui::End();
#endif
	}
}