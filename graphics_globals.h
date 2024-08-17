#pragma once

namespace graphics
{
	// Stores uniform data that only changes once per frame.
	struct FrameUniformBlock
	{
		float view_proj_matrix[16] = {};
		float app_time = 0.f;
		float game_time = 0.f;
	};

	extern const float IDENTITY_MATRIX[16];

	// You may use this as a temporary scratch buffer for vertices, for example
	// when building a list of vertices to eventually upload to a buffer.
	extern eastl::vector<graphics::Vertex> temp_vertices;

	extern Handle<Shader> fullscreen_shader;
	extern Handle<Shader> sprite_shader;
	extern Handle<Shader> grass_shader;
	extern Handle<Shader> shape_shader;
	extern Handle<Shader> text_shader;
	extern Handle<Shader> ui_shader;

	extern Handle<Buffer> dynamic_vertex_buffer;
	extern Handle<Buffer> dynamic_index_buffer;

	extern Handle<Buffer> frame_uniform_buffer;

	void initialize_globals();
}