#pragma once

namespace graphics
{
	struct Shader;
	struct Buffer;
	struct Texture;
	struct Framebuffer;

	struct Vertex
	{
		Vector2f position;
		Color color;
		Vector2f tex_coord;
	};

	void initialize();
	void shutdown();

	unsigned int get_uniform_buffer_offset_alignment();

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

	// BUFFERS

	struct BufferDesc
	{
		std::string_view debug_name = "buffer";
		unsigned int size = 0; // in bytes
		const void* initial_data = nullptr;
		bool dynamic = false; // If true, the buffer can be updated with update_buffer().
	};

	Handle<Buffer> create_buffer(const BufferDesc&& desc);
	void recreate_buffer(Handle<Buffer> handle, unsigned int size, const void* initial_data = nullptr);
	void destroy_buffer(Handle<Buffer> handle);

	// Fails if the buffer is not dynamic, or if offset + size exceeds the buffer size.
	void update_buffer(Handle<Buffer> handle, const void* data, unsigned int size, unsigned int offset = 0);
	size_t get_buffer_size(Handle<Buffer> handle);

	void bind_vertex_buffer(unsigned int binding, Handle<Buffer> handle, unsigned int stride, unsigned int offset = 0);
	void unbind_vertex_buffer(unsigned int binding);
	void bind_index_buffer(Handle<Buffer> handle);
	void unbind_index_buffer();
	void bind_uniform_buffer(unsigned int binding, Handle<Buffer> handle);
	// Fails if offset + size exceeds the buffer size, or if offset
	// is not a multiple of get_uniform_buffer_offset_alignment().
	void bind_uniform_buffer_range(unsigned int binding, Handle<Buffer> handle, unsigned int size, unsigned offset = 0);
	void unbind_uniform_buffer(unsigned int binding);

	// TEXTURES

	enum class Format
	{
		UNKNOWN,
		R8_UNORM,
		R8G8_UNORM,
		R8G8B8_UNORM,
		R8G8B8A8_UNORM,
	};


	struct TextureDesc
	{
		std::string_view debug_name = "texture";
		unsigned int width = 0;
		unsigned int height = 0;
		Format format = Format::UNKNOWN;
		const unsigned char* initial_data = nullptr;
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

	// SAMPLERS

	enum class Filter
	{
		Nearest, // Sample nearest texel
		Linear, // Linear interpolation between texels
	};

	enum class Wrap
	{
		Repeat, // Repeat the texture
		MirroredRepeat, // Repeat with mirroring
		ClampToEdge, // Clamp to the edge
		ClampToBorder, // Clamp to a border color
		MirrorClampToEdge, // Mirror the texture once and then clamp to the edge
	};

	struct SamplerDesc
	{
		std::string_view debug_name = "sampler";
		Filter filter = Filter::Nearest;
		Wrap wrap = Wrap::Repeat;
		float border_color[4] = {};
	};

	Handle<Sampler> create_sampler(const SamplerDesc&& desc);
	void destroy_sampler(Handle<Sampler> handle);

	void bind_sampler(unsigned int binding, Handle<Sampler> handle);
	void unbind_sampler(unsigned int binding);

	// FRAMEBUFFERS

	struct FramebufferDesc
	{
		std::string_view debug_name = "framebuffer";
		unsigned int width = 0;
		unsigned int height = 0;
	};

	Handle<Framebuffer> create_framebuffer(const FramebufferDesc&& desc);

	// The default framebuffer is the window framebuffer.
	void bind_default_framebuffer();
	void bind_framebuffer(Handle<Framebuffer> handle);

	void clear_default_framebuffer(float r, float g, float b, float a);
	void clear_framebuffer(Handle<Framebuffer> handle, float r, float g, float b, float a);
	Handle<Texture> get_framebuffer_texture(Handle<Framebuffer> handle);

	// FIXED-FUNCTION PIPELINE

	void set_viewport(int x, int y, int width, int height);
	void get_viewport(int& x, int& y, int& width, int& height);
	void set_scissor_test_enabled(bool enable);
	bool get_scissor_test_enabled();
	void set_scissor_box(int x, int y, int width, int height);
	void get_scissor_box(int& x, int& y, int& width, int& height);

	// DRAWING

	enum class Primitives
	{
		PointList,
		LineList,
		LineStrip,
		TriangleList,
		TriangleStrip,
	};

	void draw(Primitives primitives, unsigned int vertex_count, unsigned int vertex_offset = 0);
	void draw_indexed(Primitives primitives, unsigned int index_count);

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