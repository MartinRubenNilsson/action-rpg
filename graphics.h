#pragma once

namespace graphics
{
	struct Shader;
	struct Texture;
	struct RenderTarget;

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

	extern Handle<RenderTarget> window_render_target;
	extern Handle<Shader> sprite_shader;
	extern Handle<Shader> fullscreen_shader;
	extern Handle<Shader> shape_shader;
	extern Handle<Shader> ui_shader;

	void initialize();
	void shutdown();

	// SHADERS

	//TODO: split shaders into vertex and fragment shaders

	Handle<Shader> create_shader(
		const std::string& vertex_shader_bytecode,
		const std::string& fragment_shader_bytecode,
		const std::string& name_hint = "shader");
	Handle<Shader> load_shader(
		const std::string& vertex_shader_path,
		const std::string& fragment_shader_path);

	void bind_shader(Handle<Shader> handle);
	void unbind_shader();

	void set_uniform_1f(Handle<Shader> handle, std::string_view name, float x);
	void set_uniform_2f(Handle<Shader> handle, std::string_view name, float x, float y);
	void set_uniform_3f(Handle<Shader> handle, std::string_view name, float x, float y, float z);
	void set_uniform_4f(Handle<Shader> handle, std::string_view name, float x, float y, float z, float w);
	void set_uniform_1i(Handle<Shader> handle, std::string_view name, int x);
	void set_uniform_2i(Handle<Shader> handle, std::string_view name, int x, int y);
	void set_uniform_3i(Handle<Shader> handle, std::string_view name, int x, int y, int z);
	void set_uniform_4i(Handle<Shader> handle, std::string_view name, int x, int y, int z, int w);
	void set_uniform_mat4(Handle<Shader> handle, std::string_view name, const float matrix[16]);

	// TEXTURES

	// TODO: load compressed textures :)

	struct TextureDesc
	{
		std::string_view debug_name = "texture";
		unsigned int width = 0;
		unsigned int height = 0;
		unsigned int channels = 4;
		const unsigned char* initial_data = nullptr;
	};

	Handle<Texture> create_texture(const TextureDesc&& desc);
	Handle<Texture> load_texture(const std::string& path, bool flip_y = true);
	Handle<Texture> copy_texture(Handle<Texture> handle);
	void destroy_texture(Handle<Texture> handle);

	void bind_texture(unsigned int texture_unit, Handle<Texture> handle);
	void unbind_texture(unsigned int texture_unit);

	void update_texture(Handle<Texture> handle, const unsigned char* data);
	void get_texture_size(Handle<Texture> handle, unsigned int& width, unsigned int& height);
	void set_texture_filter(Handle<Texture> handle, TextureFilter filter);
	TextureFilter get_texture_filter(Handle<Texture> handle);

	// RENDER TARGETS

	Handle<RenderTarget> create_render_target(
		unsigned int width,
		unsigned int height,
		const std::string& name_hint = "render target");
	Handle<RenderTarget> aquire_temporary_render_target(unsigned int width, unsigned int height);
	void release_temporary_render_target(Handle<RenderTarget> handle);

	void bind_render_target(Handle<RenderTarget> handle);

	// IMPORTANT: You need to bind the render target before calling clear_render_target()!
	void clear_render_target(float r, float g, float b, float a);
	Handle<Texture> get_render_target_texture(Handle<RenderTarget> handle);

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
	void draw_triangles(const Vertex* vertices, unsigned int vertex_count);
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