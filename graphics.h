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

	extern const float IDENTITY_MATRIX[16];

	extern RenderTargetHandle window_render_target;
	extern ShaderHandle sprite_shader;
	extern ShaderHandle fullscreen_shader;
	extern ShaderHandle shape_shader;
	extern ShaderHandle ui_shader;

	void initialize();
	void shutdown();

	// SHADERS

	//TODO: split shaders into vertex and fragment shaders

	ShaderHandle create_shader(
		const std::string& vertex_shader_bytecode,
		const std::string& fragment_shader_bytecode,
		const std::string& name_hint = "shader");
	ShaderHandle load_shader(
		const std::string& vertex_shader_path,
		const std::string& fragment_shader_path);

	void bind_shader(ShaderHandle handle);
	void unbind_shader();

	void set_uniform_1f(ShaderHandle handle, std::string_view name, float x);
	void set_uniform_2f(ShaderHandle handle, std::string_view name, float x, float y);
	void set_uniform_3f(ShaderHandle handle, std::string_view name, float x, float y, float z);
	void set_uniform_4f(ShaderHandle handle, std::string_view name, float x, float y, float z, float w);
	void set_uniform_1i(ShaderHandle handle, std::string_view name, int x);
	void set_uniform_2i(ShaderHandle handle, std::string_view name, int x, int y);
	void set_uniform_3i(ShaderHandle handle, std::string_view name, int x, int y, int z);
	void set_uniform_4i(ShaderHandle handle, std::string_view name, int x, int y, int z, int w);
	void set_uniform_mat4(ShaderHandle handle, std::string_view name, const float matrix[16]);

	// TEXTURES

	// TODO: load compressed textures :)

	TextureHandle create_texture(
		unsigned int width,
		unsigned int height,
		unsigned int channels = 4,
		const unsigned char* data = nullptr, //pass nullptr to create an empty texture
		const std::string& name_hint = "texture");
	TextureHandle load_texture(const std::string& path, bool flip_y = true);
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

	// IMPORTANT: You need to bind the render target before calling clear_render_target()!
	void clear_render_target(float r, float g, float b, float a);
	TextureHandle get_render_target_texture(RenderTargetHandle handle);

	// FIXED-FUNCTION PIPELINE

	void set_viewport(int x, int y, int width, int height);
	void get_viewport(int& x, int& y, int& width, int& height);
	void set_scissor_test_enabled(bool enable);
	bool get_scissor_test_enabled();
	void set_scissor_box(int x, int y, int width, int height);
	void get_scissor_box(int& x, int& y, int& width, int& height);

	// DRAWING

	void draw_lines(const Vertex* vertices, unsigned int vertex_count);
	void draw_line_strip(const Vertex* vertices, unsigned int vertex_count);
	void draw_line_loop(const Vertex* vertices, unsigned int vertex_count);
	void draw_triangle_strip(unsigned int vertex_count);
	void draw_triangle_strip(const Vertex* vertices, unsigned int vertex_count);
	void draw_triangles(unsigned int vertex_count);
	void draw_triangles(
		const Vertex* vertices, unsigned int vertex_count,
		unsigned int* indices, unsigned int index_count);

	// DEBUGGING

	void push_debug_group(std::string_view name);
	void pop_debug_group();

	struct ScopedDebugGroup
	{
		ScopedDebugGroup(std::string_view name) { push_debug_group(name); }
		~ScopedDebugGroup() { pop_debug_group(); }
	};

	void show_texture_debug_window();
}