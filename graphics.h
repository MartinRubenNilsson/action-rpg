#pragma once

namespace graphics
{
	struct Shader;
	struct UniformBuffer;
	struct Texture;
	struct Framebuffer;

	enum class PrimitiveTopology
	{
		PointList,
		LineList,
		LineStrip,
		TriangleList,
		TriangleStrip,
	};

	struct Vertex
	{
		Vector2f position;
		Color color;
		Vector2f tex_coord;
	};

	void initialize();
	void shutdown();

	// SHADERS

	struct ShaderDesc
	{
		std::string_view debug_name = "shader";
		std::string_view vs_source; // vertex shader source code
		std::string_view fs_source; // fragment shader source code
	};

	Handle<Shader> create_shader(const ShaderDesc&& desc);
	Handle<Shader> load_shader(const std::string& vs_path, const std::string& fs_path);

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

	// UNIFORM BUFFERS

	struct UniformBufferDesc
	{
		std::string_view debug_name = "uniform buffer";
		unsigned int size = 0;
		const void* initial_data = nullptr;
	};

	Handle<UniformBuffer> create_uniform_buffer(const UniformBufferDesc&& desc);

	void update_uniform_buffer(Handle<UniformBuffer> handle, const void* data, unsigned int size);

	void bind_uniform_buffer(unsigned int binding, Handle<UniformBuffer> handle);
	void unbind_uniform_buffer(unsigned int binding);

	// TEXTURES

	enum class Filter
	{
		Nearest, // Sample nearest texel
		Linear, // Linear interpolation between texels
	};

	struct TextureDesc
	{
		std::string_view debug_name = "texture";
		unsigned int width = 0;
		unsigned int height = 0;
		unsigned int channels = 4; // TODO: replace with format enum
		const unsigned char* initial_data = nullptr;
		Filter filter = Filter::Nearest;
	};

	Handle<Texture> create_texture(const TextureDesc&& desc);
	Handle<Texture> load_texture(const std::string& path);
	Handle<Texture> copy_texture(Handle<Texture> src);
	void destroy_texture(Handle<Texture> handle);

	void bind_texture(unsigned int binding, Handle<Texture> handle);
	void unbind_texture(unsigned int binding);

	void update_texture(Handle<Texture> handle, const unsigned char* data);
	void copy_texture(Handle<Texture> dest, Handle<Texture> src);

	void get_texture_size(Handle<Texture> handle, unsigned int& width, unsigned int& height);
	void set_texture_filter(Handle<Texture> handle, Filter filter);
	Filter get_texture_filter(Handle<Texture> handle);

	// FRAMEBUFFER

	struct FramebufferDesc
	{
		std::string_view debug_name = "framebuffer";
		unsigned int width = 0;
		unsigned int height = 0;
	};

	Handle<Framebuffer> create_framebuffer(const FramebufferDesc&& desc);
	Handle<Framebuffer> aquire_temporary_framebuffer(unsigned int width, unsigned int height);
	void release_temporary_framebuffer(Handle<Framebuffer> handle);

	void bind_window_framebuffer();
	void bind_framebuffer(Handle<Framebuffer> handle);

	void clear_framebuffer(float r, float g, float b, float a); // Clears the currently bound framebuffer.
	Handle<Texture> get_framebuffer_texture(Handle<Framebuffer> handle);

	// FIXED-FUNCTION PIPELINE

	void set_viewport(int x, int y, int width, int height);
	void get_viewport(int& x, int& y, int& width, int& height);
	void set_scissor_test_enabled(bool enable);
	bool get_scissor_test_enabled();
	void set_scissor_box(int x, int y, int width, int height);
	void get_scissor_box(int& x, int& y, int& width, int& height);

	// DRAWING

	void draw(PrimitiveTopology topology, unsigned int vertex_count);
	void draw(PrimitiveTopology topology, const Vertex* vertices, unsigned int vertex_count);
	void draw(PrimitiveTopology topology, const Vertex* vertices, unsigned int vertex_count, unsigned int* indices, unsigned int index_count);

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