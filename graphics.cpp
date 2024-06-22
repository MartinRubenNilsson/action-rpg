#include "stdafx.h"
#include "graphics.h"
#include "console.h"
#include <glad/glad.h>
#pragma comment(lib, "opengl32")
//#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace graphics
{
	constexpr char _GLSL_VERSION_STRING[] = "#version 430 compatibility\n";

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

	struct Shader
	{
		std::string name; // unique name
		std::unordered_map<std::string, int> uniform_locations;
		unsigned int program_object = 0;
	};

	struct Texture
	{
		std::string name; // unique name
		unsigned int width = 0;
		unsigned int height = 0;
		unsigned int texture_object = 0;
	};

	std::vector<Shader> _shaders;
	std::unordered_map<std::string, int> _shader_name_to_id;
	std::vector<Texture> _textures;
	std::unordered_map<std::string, int> _texture_name_to_id;

	std::string _generate_unique_name(std::unordered_map<std::string, int> name_container, const std::string& name_hint)
	{
		std::string name = name_hint;
		for (int i = 1; name_container.contains(name); i++) {
			name = name_hint + std::to_string(i);
		}
		return name;
	}

	Shader* _get_shader(int shader_id)
	{
		if (shader_id < 0 || shader_id >= (int)_shaders.size())
			return nullptr;
		return &_shaders[shader_id];
	}

	Texture* _get_texture(int texture_id)
	{
		if (texture_id < 0 || texture_id >= (int)_textures.size())
			return nullptr;
		return &_textures[texture_id];
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
		system("copy /Y ..\\*.frag .\\assets\\shaders\\");
#endif
		if (!gladLoadGL()) {
			console::log_error("Failed to load OpenGL functions.");
			return;
		}
#ifdef _DEBUG
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(_debug_message_callback, 0);
#endif
		load_shader({}, {}); // Load default shader
	}

	void shutdown()
	{
		// DELETE SHADERS

		for (Shader& shader : _shaders) {
			glDeleteProgram(shader.program_object);
		}
		_shaders.clear();
		_shader_name_to_id.clear();

		// DELETE TEXTURES

		for (Texture& texture : _textures) {
			glDeleteTextures(1, &texture.texture_object);
		}
		_textures.clear();
		_texture_name_to_id.clear();
	}

	int create_shader(
		const std::string& vertex_shader_bytecode,
		const std::string& fragment_shader_bytecode,
		const std::string& name_hint)
	{
		const int program_object = glCreateProgram();

		{
			// COMPILE VERTEX SHADER

			const unsigned int vertex_shader_object = glCreateShader(GL_VERTEX_SHADER);
			const char* vertex_shader_strings[] = { _GLSL_VERSION_STRING, vertex_shader_bytecode.c_str() };
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
				return -1;
			}

			// ATTACH VERTEX SHADER

			glAttachShader(program_object, vertex_shader_object);
			glDeleteShader(vertex_shader_object);
		}

		{
			// COMPILE FRAGMENT SHADER

			const unsigned int fragment_shader_object = glCreateShader(GL_FRAGMENT_SHADER);
			const char* fragment_shader_strings[] = { _GLSL_VERSION_STRING, fragment_shader_bytecode.c_str() };
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
				return -1;
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
				return -1;
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

		const int shader_id = (int)_shaders.size();
		Shader& shader = _shaders.emplace_back();
		shader.name = _generate_unique_name(_shader_name_to_id, name_hint);
		shader.uniform_locations = std::move(uniform_locations);
		shader.program_object = program_object;

		_shader_name_to_id[shader.name] = shader_id;

		return shader_id;
	}

	int load_shader(const std::string& vertex_shader_path, const std::string& fragment_shader_path)
	{
		// CHECK CACHE FOR EXISTING SHADER
		
		const std::string name = vertex_shader_path + ":" + fragment_shader_path;
		{
			const auto it = _shader_name_to_id.find(name);
			if (it != _shader_name_to_id.end()) {
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
				return -1;
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
				return -1;
			}
			fragment_shader_bytecode = {
				std::istreambuf_iterator<char>(fragment_shader_file),
				std::istreambuf_iterator<char>() };
		}

		// CREATE SHADER

		return create_shader(vertex_shader_bytecode, fragment_shader_bytecode, name);
	}

	void bind_shader(int shader_id)
	{
		unsigned int program_object = 0;
		if (const Shader* shader = _get_shader(shader_id)) {
			program_object = shader->program_object;
		}
		glUseProgram(program_object);
	}

	void set_shader_uniform_1f(int shader_id, const std::string& name, float x)
	{
		if (const Shader* shader = _get_shader(shader_id)) {
			const auto it = shader->uniform_locations.find(name);
			if (it != shader->uniform_locations.end()) {
				glUniform1f(it->second, x);
			}
		}
	}

	void set_shader_uniform_2f(int shader_id, const std::string& name, float x, float y)
	{
		if (const Shader* shader = _get_shader(shader_id)) {
			const auto it = shader->uniform_locations.find(name);
			if (it != shader->uniform_locations.end()) {
				glUniform2f(it->second, x, y);
			}
		}
	}

	void set_shader_uniform_3f(int shader_id, const std::string& name, float x, float y, float z)
	{
		if (const Shader* shader = _get_shader(shader_id)) {
			const auto it = shader->uniform_locations.find(name);
			if (it != shader->uniform_locations.end()) {
				glUniform3f(it->second, x, y, z);
			}
		}
	}

	void set_shader_uniform_4f(int shader_id, const std::string& name, float x, float y, float z, float w)
	{
		if (const Shader* shader = _get_shader(shader_id)) {
			const auto it = shader->uniform_locations.find(name);
			if (it != shader->uniform_locations.end()) {
				glUniform4f(it->second, x, y, z, w);
			}
		}
	}

	void set_shader_uniform_1i(int shader_id, const std::string& name, int x)
	{
		if (const Shader* shader = _get_shader(shader_id)) {
			const auto it = shader->uniform_locations.find(name);
			if (it != shader->uniform_locations.end()) {
				glUniform1i(it->second, x);
			}
		}
	}

	void set_shader_uniform_2i(int shader_id, const std::string& name, int x, int y)
	{
		if (const Shader* shader = _get_shader(shader_id)) {
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

		unsigned int texture_object;
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
		texture.texture_object = texture_object;

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

	int copy_texture(unsigned int texture_object)
	{
		glBindTexture(GL_TEXTURE_2D, texture_object);
		int internal_format, width, height;
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &internal_format);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
		unsigned int copy_texture_object;
		glGenTextures(1, &copy_texture_object);
		glBindTexture(GL_TEXTURE_2D, copy_texture_object);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glCopyImageSubData(
			texture_object, GL_TEXTURE_2D, 0, 0, 0, 0,
			copy_texture_object, GL_TEXTURE_2D, 0, 0, 0, 0,
			width, height, 1);

		const int texture_id = (int)_textures.size();
		Texture& texture = _textures.emplace_back();
		texture.width = width;
		texture.height = height;
		texture.texture_object = copy_texture_object;

		return texture_id;
	}

	void bind_texture(unsigned int texture_slot, int texture_id)
	{
		unsigned int texture_object = 0;
		if (const Texture* texture = _get_texture(texture_id)) {
			texture_object = texture->texture_object;
		}
		glActiveTexture(GL_TEXTURE0 + texture_slot);
		glBindTexture(GL_TEXTURE_2D, texture_object);
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

	void set_modelview_matrix_to_identity()
	{
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
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

	void draw_triangle_strip(const Vertex* vertices, unsigned int vertex_count)
	{
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glVertexPointer(2, GL_FLOAT, sizeof(Vertex), (unsigned char*)vertices + 0);
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vertex), (unsigned char*)vertices + 8);
		glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), (unsigned char*)vertices + 12);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, vertex_count);
	}
}