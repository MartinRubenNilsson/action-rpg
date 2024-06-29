#pragma once

namespace graphics
{
	// Functions that return an integer ID return -1 on failure.

	enum class TextureFilter
	{
		Nearest, // Sample nearest texel
		Linear, // Linear interpolation between texels
	};

	struct Vertex
	{
		sf::Vector2f position;
		sf::Color color;
		sf::Vector2f tex_coord;
	};

	extern int window_render_target_id;

	extern int default_shader_id;
	extern int fullscreen_shader_id;
	extern int color_only_shader_id;

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
	// You need to bind the shader before set_shader_uniform_*().
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
		unsigned int channels = 4,
		const unsigned char* data = nullptr, //pass nullptr to create an empty texture
		std::string name_hint = "texture");
	int load_texture(const std::string& path);
	int copy_texture(unsigned int texture_id);
	void bind_texture(unsigned int texture_unit, int texture_id);
	void unbind_texture(unsigned int texture_unit);
	void get_texture_size(int texture_id, unsigned int& width, unsigned int& height);
	void set_texture_filter(int texture_id, TextureFilter filter);
	TextureFilter get_texture_filter(int texture_id);

	// RENDER TARGETS

	int create_render_target(
		unsigned int width,
		unsigned int height,
		std::string name_hint = "render target");
	int acquire_pooled_render_target(unsigned int width, unsigned int height);
	void release_pooled_render_target(int render_target_id);
	void bind_render_target(int render_target_id);
	// You need to bind the render target before calling clear_render_target().
	void clear_render_target(float r, float g, float b, float a);
	int get_render_target_texture(int render_target_id);

	// FIXED-FUNCTION PIPELINE

	void set_viewport(int x, int y, int width, int height);
	void get_viewport(int& x, int& y, int& width, int& height);
	void set_modelview_matrix_to_identity();
	void set_modelview_matrix(const float matrix[16]);
	void set_projection_matrix_to_identity();
	void set_projection_matrix(const float matrix[16]);
	void set_texture_matrix_to_identity();

	// DRAWING

	void draw_lines(const Vertex* vertices, unsigned int vertex_count);
	void draw_line_strip(const Vertex* vertices, unsigned int vertex_count);
	void draw_line_loop(const Vertex* vertices, unsigned int vertex_count);
	void draw_triangle_strip(unsigned int vertex_count);
	void draw_triangle_strip(const Vertex* vertices, unsigned int vertex_count);
}