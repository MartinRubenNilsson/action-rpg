#pragma once

namespace graphics
{
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

	extern RenderTargetHandle window_render_target;
	extern ShaderHandle default_shader;
	extern ShaderHandle fullscreen_shader;
	extern ShaderHandle color_only_shader;

	void initialize();
	void shutdown();

	// SHADERS

	ShaderHandle create_shader(
		const std::string& vertex_shader_bytecode,
		const std::string& fragment_shader_bytecode,
		const std::string& name_hint = "shader");
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

	// TODO: load compressed textures :)

	TextureHandle create_texture(
		unsigned int width,
		unsigned int height,
		unsigned int channels = 4,
		const unsigned char* data = nullptr, //pass nullptr to create an empty texture
		const std::string& name_hint = "texture");
	TextureHandle load_texture(const std::string& path);
	TextureHandle copy_texture(TextureHandle handle);

	void bind_texture(unsigned int texture_unit, TextureHandle handle);
	void unbind_texture(unsigned int texture_unit);

	void get_texture_size(TextureHandle handle, unsigned int& width, unsigned int& height);
	void set_texture_filter(TextureHandle handle, TextureFilter filter);
	TextureFilter get_texture_filter(TextureHandle handle);

	// RENDER TARGETS

	RenderTargetHandle create_render_target(
		unsigned int width,
		unsigned int height,
		const std::string& name_hint = "render target");
	RenderTargetHandle acquire_pooled_render_target(unsigned int width, unsigned int height);
	void release_pooled_render_target(RenderTargetHandle handle);

	void bind_render_target(RenderTargetHandle handle);

	// You need to bind the render target before calling clear_render_target().
	void clear_render_target(float r, float g, float b, float a);
	TextureHandle get_render_target_texture(RenderTargetHandle handle);

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