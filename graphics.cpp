#include "stdafx.h"
#include "graphics.h"
#include "console.h"
#include <glad/glad.h>
#pragma comment(lib, "opengl32")
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace graphics
{
	constexpr char _DEFAULT_VERTEX_SHADER_BYTECODE[] = R"(
void main()
{
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
    gl_FrontColor = gl_Color;
}
)";

	constexpr char _DEFAULT_FRAGMENT_SHADER_BYTECODE[] = R"(
uniform sampler2D tex;

void main()
{
    vec4 pixel = texture(tex, gl_TexCoord[0].xy);
    gl_FragColor = gl_Color * pixel;
}
)";

	constexpr char _FULLSCREEN_VERTEX_SHADER_BYTECODE[] = R"(
void main()
{
	const vec2 vertices[4] = vec2[4](
		vec2(-1.0, -1.0),  // bottom-left
		vec2( 1.0, -1.0),  // bottom-right
		vec2(-1.0,  1.0),  // top-left
		vec2( 1.0,  1.0)); // top-right
	const vec2 tex_coords[4] = vec2[4](
		vec2(0.0, 0.0),  // bottom-left
		vec2(1.0, 0.0),  // bottom-right
		vec2(0.0, 1.0),  // top-left
		vec2(1.0, 1.0)); // top-right
	gl_Position = vec4(vertices[gl_VertexID], 0.0, 1.0);
	gl_TexCoord[0].xy = tex_coords[gl_VertexID];
}
)";

	constexpr char _FULLSCREEN_FRAGMENT_SHADER_BYTECODE[] = R"(
uniform sampler2D tex;

void main()
{
	gl_FragColor = texture(tex, gl_TexCoord[0].xy);
}
)";

	constexpr char _COLOR_ONLY_VERTEX_SHADER_BYTECODE[] = R"(
void main()
{
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    gl_FrontColor = gl_Color;
}
)";

	constexpr char _COLOR_ONLY_PIXEL_SHADER_BYTECODE[] = R"(
void main()
{
    gl_FragColor = gl_Color;
}
)";

	int window_render_target_id = -2; // HACK: magic value -2
	ShaderHandle default_shader = ShaderHandle::Invalid;
	ShaderHandle fullscreen_shader = ShaderHandle::Invalid;
	ShaderHandle color_only_shader = ShaderHandle::Invalid;

	struct Shader
	{
		std::string name; // unique name
		std::unordered_map<std::string, int> uniform_locations;
		GLuint program_object = 0;
	};

	struct Texture
	{
		std::string name; // unique name
		unsigned int width = 0;
		unsigned int height = 0;
		unsigned int channels = 0;
		GLuint texture_object = 0;
		TextureFilter filter = TextureFilter::Nearest;
	};

	struct RenderTarget
	{
		std::string name; // unique name
		int texture_id = -1;
		GLuint framebuffer_object = 0;
	};

	std::vector<Shader> _shaders;
	std::unordered_map<std::string, ShaderHandle> _shader_name_to_handle;
	std::vector<Texture> _textures;
	std::unordered_map<std::string, int> _texture_name_to_id;
	std::vector<RenderTarget> _render_targets;
	std::unordered_map<std::string, int> _render_target_name_to_id;
	std::vector<int> _pooled_render_target_ids;
	GLuint _last_bound_program_object = 0;

	template <class Container>
	std::string _generate_unique_name(const Container& name_container, const std::string& name_hint)
	{
		std::string name = name_hint;
		for (int i = 1; name_container.contains(name); i++) {
			name = name_hint + " (" + std::to_string(i) + ")";
		}
		return name;
	}

	Shader* _get_shader(ShaderHandle handle)
	{
		const int index = (int)handle;
		if (index < 0 || index >= (int)_shaders.size()) return nullptr;
		return &_shaders[index];
	}

	Texture* _get_texture(int texture_id)
	{
		if (texture_id < 0 || texture_id >= (int)_textures.size())
			return nullptr;
		return &_textures[texture_id];
	}

	RenderTarget* _get_render_target(int render_texture_id)
	{
		if (render_texture_id < 0 || render_texture_id >= (int)_render_targets.size())
			return nullptr;
		return &_render_targets[render_texture_id];
	}

#ifdef _DEBUG
	void GLAPIENTRY _debug_message_callback(
		GLenum source,
		GLenum type,
		GLuint id,
		GLenum severity,
		GLsizei length,
		const GLchar* message,
		const void* userParam)
	{
		console::log_error(std::format("OpenGL: {}", message));
	}
#endif

	void initialize()
	{
#ifdef _DEBUG
		// HACK: We should be using a post-build event to copy the shaders,
		// but then it doesn't run when only debugging and not recompiling,
		// which is annoying when you've changed a shader but not the code,
		// because then the new shader doesn't get copied.
		system("copy /Y ..\\*.vert .\\assets\\shaders\\");
		system("copy /Y ..\\*.frag .\\assets\\shaders\\");

		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(_debug_message_callback, 0);
#endif

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		default_shader = create_shader(
			_DEFAULT_VERTEX_SHADER_BYTECODE,
			_DEFAULT_FRAGMENT_SHADER_BYTECODE,
			"default shader");

		fullscreen_shader = create_shader(
			_FULLSCREEN_VERTEX_SHADER_BYTECODE,
			_FULLSCREEN_FRAGMENT_SHADER_BYTECODE,
			"fullscreen shader");

		color_only_shader = create_shader(
			_COLOR_ONLY_VERTEX_SHADER_BYTECODE,
			_COLOR_ONLY_PIXEL_SHADER_BYTECODE,
			"color only shader");
	}

	void shutdown()
	{
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
		_texture_name_to_id.clear();

		// DELETE RENDER TEXTURES

		for (RenderTarget& render_texture : _render_targets) {
			glDeleteFramebuffers(1, &render_texture.framebuffer_object);
		}
		_render_targets.clear();
		_render_target_name_to_id.clear();
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
			const char* vertex_shader_strings[] = { GLSL_VERSION_STRING, vertex_shader_bytecode.c_str() };
			glShaderSource(vertex_shader_object, 2, vertex_shader_strings, nullptr);
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
			const char* fragment_shader_strings[] = { GLSL_VERSION_STRING, fragment_shader_bytecode.c_str() };
			glShaderSource(fragment_shader_object, 2, fragment_shader_strings, nullptr);
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

		std::unordered_map<std::string, int> uniform_locations;
		{
			int uniform_count;
			glGetProgramiv(program_object, GL_ACTIVE_UNIFORMS, &uniform_count);
			for (int i = 0; i < uniform_count; i++) {
				char uniform_name[256];
				int uniform_size;
				unsigned int uniform_type;
				glGetActiveUniform(program_object, i, sizeof(uniform_name),
					nullptr, &uniform_size, &uniform_type, uniform_name);
				uniform_locations[uniform_name] = glGetUniformLocation(program_object, uniform_name);
			}
		}

		// STORE SHADER

		const ShaderHandle handle = (ShaderHandle)_shaders.size();
		Shader& shader = _shaders.emplace_back();
		shader.name = _generate_unique_name(_shader_name_to_handle, name_hint);
		shader.uniform_locations = std::move(uniform_locations);
		shader.program_object = program_object;

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
		if (vertex_shader_path.empty()) {
			vertex_shader_bytecode = _DEFAULT_VERTEX_SHADER_BYTECODE;
		} else {
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
		if (fragment_shader_path.empty()) {
			fragment_shader_bytecode = _DEFAULT_FRAGMENT_SHADER_BYTECODE;
		} else {
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

	void _bind_program_object(GLuint program_object)
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
			_bind_program_object(shader->program_object);
		}
	}

	void unbind_shader()
	{
		_bind_program_object(0);
	}

	void set_shader_uniform_1f(ShaderHandle handle, const std::string& name, float x)
	{
		if (const Shader* shader = _get_shader(handle)) {
			const auto it = shader->uniform_locations.find(name);
			if (it != shader->uniform_locations.end()) {
				glUniform1f(it->second, x);
			}
		}
	}

	void set_shader_uniform_2f(ShaderHandle handle, const std::string& name, float x, float y)
	{
		if (const Shader* shader = _get_shader(handle)) {
			const auto it = shader->uniform_locations.find(name);
			if (it != shader->uniform_locations.end()) {
				glUniform2f(it->second, x, y);
			}
		}
	}

	void set_shader_uniform_3f(ShaderHandle handle, const std::string& name, float x, float y, float z)
	{
		if (const Shader* shader = _get_shader(handle)) {
			const auto it = shader->uniform_locations.find(name);
			if (it != shader->uniform_locations.end()) {
				glUniform3f(it->second, x, y, z);
			}
		}
	}

	void set_shader_uniform_4f(ShaderHandle handle, const std::string& name, float x, float y, float z, float w)
	{
		if (const Shader* shader = _get_shader(handle)) {
			const auto it = shader->uniform_locations.find(name);
			if (it != shader->uniform_locations.end()) {
				glUniform4f(it->second, x, y, z, w);
			}
		}
	}

	void set_shader_uniform_1i(ShaderHandle handle, const std::string& name, int x)
	{
		if (const Shader* shader = _get_shader(handle)) {
			const auto it = shader->uniform_locations.find(name);
			if (it != shader->uniform_locations.end()) {
				glUniform1i(it->second, x);
			}
		}
	}

	void set_shader_uniform_2i(ShaderHandle handle, const std::string& name, int x, int y)
	{
		if (const Shader* shader = _get_shader(handle)) {
			const auto it = shader->uniform_locations.find(name);
			if (it != shader->uniform_locations.end()) {
				glUniform2i(it->second, x, y);
			}
		}
	}

	int create_texture(
		unsigned int width,
		unsigned int height,
		unsigned int channels,
		const unsigned char* data,
		std::string name_hint)
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

		const int texture_id = (int)_textures.size();
		Texture& texture = _textures.emplace_back();
		texture.name = _generate_unique_name(_texture_name_to_id, name_hint);
		texture.width = width;
		texture.height = height;
		texture.channels = channels;
		texture.texture_object = texture_object;
		texture.filter = TextureFilter::Nearest;

		_texture_name_to_id[texture.name] = texture_id;

		return texture_id;
	}

	int load_texture(const std::string& path)
	{
		// CHECK IF ALREADY LOADED

		const auto it = _texture_name_to_id.find(path);
		if (it != _texture_name_to_id.end()) {
			return it->second;
		}

		// LOAD TEXTURE DATA

		int width, height, channels;
		//stbi_set_flip_vertically_on_load(true);
		unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		//stbi_set_flip_vertically_on_load(false);
		if (!data) {
			console::log_error("Failed to load texture: " + path);
			console::log_error(stbi_failure_reason());
			return -1;
		}

		// CREATE TEXTURE

		return create_texture(width, height, channels, data, path);
	}

	int copy_texture(unsigned int texture_id)
	{
		// This fixes a bug where texture would become a dangling pointer
		// if create_texture() causes _textures to reallocate.
		_textures.reserve(_textures.size() + 1);

		const Texture* texture = _get_texture(texture_id);
		if (!texture) return -1;

		const int texture_copy_id = create_texture(
			texture->width,
			texture->height,
			texture->channels,
			nullptr,
			texture->name + " (copy)");
		const Texture* texture_copy = _get_texture(texture_copy_id);
		if (!texture_copy) return -1;

		glCopyImageSubData(
			texture->texture_object, GL_TEXTURE_2D, 0, 0, 0, 0,
			texture_copy->texture_object, GL_TEXTURE_2D, 0, 0, 0, 0,
			texture->width, texture->height, 1);

		return texture_copy_id;
	}

	void bind_texture(unsigned int texture_unit, int texture_id)
	{
		if (const Texture* texture = _get_texture(texture_id)) {
			glActiveTexture(GL_TEXTURE0 + texture_unit);
			glBindTexture(GL_TEXTURE_2D, texture->texture_object);
		}
	}

	void unbind_texture(unsigned int texture_unit)
	{
		glActiveTexture(GL_TEXTURE0 + texture_unit);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void get_texture_size(int texture_id, unsigned int& width, unsigned int& height)
	{
		if (const Texture* texture = _get_texture(texture_id)) {
			width = texture->width;
			height = texture->height;
		} else {
			width = 0;
			height = 0;
		}
	}

	void set_texture_filter(int texture_id, TextureFilter filter)
	{
		Texture* texture = _get_texture(texture_id);
		if (!texture) return;
		if (texture->filter == filter) return;
		texture->filter = filter;
		const GLint gl_filter = (filter == TextureFilter::Nearest) ? GL_NEAREST : GL_LINEAR;
		glBindTexture(GL_TEXTURE_2D, texture->texture_object);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter);
	}

	TextureFilter get_texture_filter(int texture_id)
	{
		const Texture* texture = _get_texture(texture_id);
		if (!texture) return TextureFilter::Nearest;
		return texture->filter;
	}

	int create_render_target(unsigned int width, unsigned int height, std::string name_hint)
	{
		// CREATE TEXTURE

		const int texture_id = create_texture(width, height, 4, nullptr, name_hint);
		const Texture* texture = _get_texture(texture_id);
		if (!texture) {
			console::log_error("Failed to create render target: " + name_hint);
			return -1;
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
			//TODO: delete texture
			return -1;
		}

		// STORE RENDER TARGET

		const int render_target_id = (int)_render_targets.size();
		RenderTarget& render_target = _render_targets.emplace_back();
		render_target.name = _generate_unique_name(_render_target_name_to_id, name_hint);
		render_target.texture_id = texture_id;
		render_target.framebuffer_object = framebuffer_object;

		_render_target_name_to_id[render_target.name] = render_target_id;

		return render_target_id;
	}

	int acquire_pooled_render_target(unsigned int width, unsigned int height)
	{
		for (size_t i = 0; i < _pooled_render_target_ids.size(); ++i) {
			const int render_target_id = _pooled_render_target_ids[i];
			const RenderTarget* render_target = _get_render_target(render_target_id);
			if (!render_target) continue;
			const Texture* texture = _get_texture(render_target->texture_id);
			if (!texture) continue;
			if (texture->width != width) continue;
			if (texture->height != height) continue;
			std::swap(_pooled_render_target_ids[i], _pooled_render_target_ids.back());
			_pooled_render_target_ids.pop_back();
			return render_target_id;
		}
		std::string name_hint = "pooled render target " + std::to_string(width) + "x" + std::to_string(height);
		return create_render_target(width, height, name_hint);
	}

	void release_pooled_render_target(int render_target_id)
	{
		_pooled_render_target_ids.push_back(render_target_id);
	}

	void bind_render_target(int render_target_id)
	{
		if (render_target_id == window_render_target_id) {
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		} else if (const RenderTarget* render_target = _get_render_target(render_target_id)) {
			glBindFramebuffer(GL_FRAMEBUFFER, render_target->framebuffer_object);
		}
	}

	void clear_render_target(float r, float g, float b, float a)
	{
		glClearColor(r, g, b, a);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	int get_render_target_texture(int render_target_id)
	{
		if (const RenderTarget* render_target = _get_render_target(render_target_id)) {
			return render_target->texture_id;
		}
		return -1;
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

	void set_modelview_matrix_to_identity()
	{
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	}

	void set_modelview_matrix(const float matrix[16])
	{
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(matrix);
	}

	void set_projection_matrix_to_identity()
	{
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
	}

	void set_projection_matrix(const float matrix[16])
	{
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(matrix);
	}

	void set_texture_matrix_to_identity()
	{
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
	}

	void draw_lines(const Vertex* vertices, unsigned int vertex_count)
	{
		if (!vertices || !vertex_count) return;
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glVertexPointer(2, GL_FLOAT, sizeof(Vertex), &vertices[0].position);
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vertex), &vertices[0].color);
		glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &vertices[0].tex_coord);
		glDrawArrays(GL_LINES, 0, vertex_count);
	}

	void draw_line_strip(const Vertex* vertices, unsigned int vertex_count)
	{
		if (!vertices || !vertex_count) return;
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glVertexPointer(2, GL_FLOAT, sizeof(Vertex), &vertices[0].position);
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vertex), &vertices[0].color);
		glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &vertices[0].tex_coord);
		glDrawArrays(GL_LINE_STRIP, 0, vertex_count);
	}

	void draw_line_loop(const Vertex* vertices, unsigned int vertex_count)
	{
		if (!vertices || !vertex_count) return;
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glVertexPointer(2, GL_FLOAT, sizeof(Vertex), &vertices[0].position);
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vertex), &vertices[0].color);
		glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &vertices[0].tex_coord);
		glDrawArrays(GL_LINE_LOOP, 0, vertex_count);
	}

	void draw_triangle_strip(unsigned int vertex_count)
	{
		if (!vertex_count) return;
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, vertex_count);
	}

	void draw_triangle_strip(const Vertex* vertices, unsigned int vertex_count)
	{
		if (!vertices || !vertex_count) return;
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glVertexPointer(2, GL_FLOAT, sizeof(Vertex), &vertices[0].position);
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vertex), &vertices[0].color);
		glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &vertices[0].tex_coord);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, vertex_count);
	}
}