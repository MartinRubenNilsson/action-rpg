#pragma once
#include "graphics_vertices.h"

namespace graphics {

	struct FrameUniformBlock {
		float view_proj_matrix[16] = {};
		float app_time = 0.f;
		float game_time = 0.f;
		float padding0;
		float padding1;
	};

	struct UiUniformBlock {
		float transform[16] = {};
	};

	struct PlayerOutfitUniformBlock {
		int lut1_type = -1;
		int lut1_y = -1;
		int lut2_type = -1;
		int lut2_y = -1;
	};

	// You may use this as a temporary scratch buffer for vertices, for example
	// when building a list of vertices to eventually upload to a buffer.
	extern eastl::vector<graphics::Vertex> temp_vertices;

	// SHADERS

	extern Handle<VertexShader> fullscreen_vert;
	extern Handle<FragmentShader> fullscreen_frag;
	extern Handle<FragmentShader> gaussian_blur_hor_frag;
	extern Handle<FragmentShader> gaussian_blur_ver_frag;
	extern Handle<FragmentShader> screen_transition_frag;
	extern Handle<FragmentShader> shockwave_frag;
	extern Handle<VertexShader> sprite_vert;
	extern Handle<FragmentShader> sprite_frag;
	extern Handle<VertexShader> grass_vert;
	extern Handle<VertexShader> shape_vert;
	extern Handle<FragmentShader> shape_frag;
	extern Handle<FragmentShader> text_frag;
	extern Handle<VertexShader> ui_vert;
	extern Handle<FragmentShader> ui_frag;
	extern Handle<FragmentShader> player_outfit_frag;

	// FRAMEBUFFERS

	extern Handle<Framebuffer> game_framebuffer_0; // GAME_FRAMEBUFFER_WIDTH x GAME_FRAMEBUFFER_HEIGHT
	extern Handle<Framebuffer> game_framebuffer_1; // GAME_FRAMEBUFFER_WIDTH x GAME_FRAMEBUFFER_HEIGHT
	extern Handle<Framebuffer> player_outfit_framebuffer; // 1024 x 1024

	// BUFFERS

	extern Handle<Buffer> dynamic_vertex_buffer;
	extern Handle<Buffer> dynamic_index_buffer;
	extern Handle<Buffer> frame_uniform_buffer;
	extern Handle<Buffer> ui_uniform_buffer;
	extern Handle<Buffer> sprite_uniform_buffer;
	extern Handle<Buffer> player_outfit_uniform_buffer;

	// TEXTURES

	extern Handle<Texture> error_texture;
	extern Handle<Texture> white_texture; // 1x1 white texture

	// SAMPLERS

	extern Handle<Sampler> nearest_sampler; // nearest filtering, wrap
	extern Handle<Sampler> linear_sampler; // linear filtering, wrap

	void initialize_globals();
}