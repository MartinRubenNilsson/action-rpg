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

	// FRAMEBUFFERS

	extern Handle<Framebuffer> gameworld_framebuffer_target; // GAMEWORLD_FRAMEBUFFER_WIDTH x GAMEWORLD_FRAMEBUFFER_HEIGHT
	extern Handle<Framebuffer> gameworld_framebuffer_source; // GAMEWORLD_FRAMEBUFFER_WIDTH x GAMEWORLD_FRAMEBUFFER_HEIGHT
	extern Handle<Framebuffer> player_outfit_framebuffer; // 1024 x 1024

	// BUFFERS

	extern Handle<Buffer> dynamic_vertex_buffer;
	extern Handle<Buffer> dynamic_index_buffer;
	extern Handle<Buffer> frame_uniform_buffer;
	extern Handle<Buffer> sprite_uniform_buffer;

	// FULLSCREEN SHADERS

	extern Handle<Shader> fullscreen_shader; // just a simple passthrough shader
	extern Handle<Shader> gaussian_blur_hor_shader;
	extern Handle<Shader> gaussian_blur_ver_shader;
	extern Handle<Shader> screen_transition_shader;
	extern Handle<Shader> shockwave_shader;

	// SPRITE SHADERS

	extern Handle<Shader> sprite_shader;
	extern Handle<Shader> grass_shader;
	extern Handle<Shader> shape_shader;
	extern Handle<Shader> text_shader;
	extern Handle<Shader> ui_shader;

	// SAMPLERS

	extern Handle<Sampler> nearest_sampler; // nearest filtering, wrap
	extern Handle<Sampler> linear_sampler; // linear filtering, wrap

	void initialize_globals();
}