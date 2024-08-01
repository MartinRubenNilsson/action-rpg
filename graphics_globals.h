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
	extern Handle<Shader> ui_shader;

	extern Handle<UniformBuffer> frame_uniforms;

	void initialize_globals();
}