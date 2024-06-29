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
		Vector2f position;
		Color color;
		Vector2f tex_coord;
	};

	extern int window_render_target_id;

	extern ShaderHandle default_shader;
	extern ShaderHandle fullscreen_shader;
	extern ShaderHandle color_only_shader;

	void initialize();
	void shutdown();

	// SHADERS

	// Returns ShaderHandle::Invalid on failure.
	ShaderHandle create_shader(
		const std::string& vertex_shader_bytecode,
		const std::string& fragment_shader_bytecode,
		const std::string& name_hint = "shader");
	// Returns ShaderHandle::Invalid on failure.
	ShaderHandle load_shader(
		const std::string& vertex_shader_path,
		const std::string& fragment_shader_path);

	void bind_shader(ShaderHandle handle);
	void unbind_shader();

	// You need to bind the shader before set_shader_uniform_*().
	void set_shader_uniform_1f(ShaderHandle handle, const std::string& name, float x);
	void set_shader_uniform_2f(ShaderHandle handle, const std::string& name, float x, float y);
	void set_shader_uniform_3f(ShaderHandle handle, const std::string& name, float x, float y, float z);
	void set_shader_uniform_4f(ShaderHandle handle, const std::string& name, float x, float y, float z, float w);
	void set_shader_uniform_1i(ShaderHandle handle, const std::string& name, int x);
	void set_shader_uniform_2i(ShaderHandle handle, const std::string& name, int x, int y);

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
	void set_scissor_test_enabled(bool enable);
	bool get_scissor_test_enabled();
	void set_scissor_box(int x, int y, int width, int height);
	void get_scissor_box(int& x, int& y, int& width, int& height);
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