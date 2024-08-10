#pragma once

namespace graphics
{
	// Stores uniform data that only changes once per frame.
	struct FrameUniforms
	{
		float view_proj_matrix[16] = {};
	};

	extern const float IDENTITY_MATRIX[16];

	extern Handle<Shader> sprite_shader;
	extern Handle<Shader> fullscreen_shader;
	extern Handle<Shader> shape_shader;
	extern Handle<Shader> text_shader;
	extern Handle<Shader> ui_shader;

	extern Handle<Buffer> dynamic_vertex_buffer;
	extern Handle<Buffer> dynamic_index_buffer;

	extern Handle<Buffer> frame_uniform_buffer;

	void initialize_globals();
}