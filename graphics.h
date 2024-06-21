#pragma once

namespace graphics
{
	void initialize();
	void shutdown();

	// SHADERS

	//Returns -1 on failure.
	int load_shader(const std::string& vertex_shader_path, const std::string& fragment_shader_path);
	//Pass shader_id = -1 to unbind the current shader.
	void bind_shader(int shader_id = -1);
	//You need to bind the shader before calling these functions!
	void set_shader_uniform_1f(int shader_id, const std::string& name, float x);
	void set_shader_uniform_2f(int shader_id, const std::string& name, float x, float y);
	void set_shader_uniform_3f(int shader_id, const std::string& name, float x, float y, float z);
	void set_shader_uniform_4f(int shader_id, const std::string& name, float x, float y, float z, float w);
	void set_shader_uniform_1i(int shader_id, const std::string& name, int x);
	void set_shader_uniform_2i(int shader_id, const std::string& name, int x, int y);

	// TEXTURES

	//Returns -1 on failure.
	int load_texture(const std::string& path);
	//Pass texture_id = -1 to unbind the current texture.
	void bind_texture(int texture_slot, int texture_id = -1);
	void get_texture_size(int texture_id, unsigned int& width, unsigned int& height);

	// FIXED-FUNCTION PIPELINE

	void set_viewport(int x, int y, int width, int height);
	void get_viewport(int& x, int& y, int& width, int& height);
	void set_modelview_matrix_to_identity();
	void set_projection_matrix_to_identity();
	void set_texture_matrix_to_identity();

	// DRAWING

	void draw_triangle_strip(const sf::Vertex* vertices, unsigned int vertex_count);
}