#pragma once

namespace graphics
{
	// Functions that return an integer ID return -1 on failure.

	struct Vertex
	{
		sf::Vector2f position;
		sf::Color color;
		sf::Vector2f tex_coords;
	};

	void initialize();
	void shutdown();

	// SHADERS

	int create_shader(
		const std::string& vertex_shader_bytecode,
		const std::string& fragment_shader_bytecode,
		const std::string& name_hint = "shader");
	int load_shader(const std::string& vertex_shader_path, const std::string& fragment_shader_path);
	void bind_shader(int shader_id);
	void unbind_shader();
	//You need to bind the shader before calling these functions!
	void set_shader_uniform_1f(int shader_id, const std::string& name, float x);
	void set_shader_uniform_2f(int shader_id, const std::string& name, float x, float y);
	void set_shader_uniform_3f(int shader_id, const std::string& name, float x, float y, float z);
	void set_shader_uniform_4f(int shader_id, const std::string& name, float x, float y, float z, float w);
	void set_shader_uniform_1i(int shader_id, const std::string& name, int x);
	void set_shader_uniform_2i(int shader_id, const std::string& name, int x, int y);

	// TEXTURES

	int create_texture(
		unsigned int width,
		unsigned int height,
		unsigned int channels,
		const unsigned char* data,
		std::string name_hint = "texture");
	int load_texture(const std::string& path);
	int copy_texture(unsigned int texture_object); //TODO: don't expose OpenGL objects like this
	void bind_texture(unsigned int texture_unit, int texture_id);
	void unbind_texture(unsigned int texture_unit);
	void get_texture_size(int texture_id, unsigned int& width, unsigned int& height);

	// FIXED-FUNCTION PIPELINE

	void set_viewport(int x, int y, int width, int height);
	void get_viewport(int& x, int& y, int& width, int& height);
	void set_modelview_matrix_to_identity();
	void set_projection_matrix_to_identity();
	void set_projection_matrix(const float matrix[16]);
	void set_texture_matrix_to_identity();

	// DRAWING

	void draw_lines(const Vertex* vertices, unsigned int vertex_count);
	void draw_line_strip(const Vertex* vertices, unsigned int vertex_count);
	void draw_line_loop(const Vertex* vertices, unsigned int vertex_count);
	void draw_triangle_strip(const Vertex* vertices, unsigned int vertex_count);
}