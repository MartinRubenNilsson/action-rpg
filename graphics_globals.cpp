#include "stdafx.h"
#include "graphics_globals.h"
#include "graphics.h"

namespace graphics
{
	extern const float IDENTITY_MATRIX[16] = {
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f,
	};

	eastl::vector<graphics::Vertex> temp_vertices;

	Handle<Framebuffer> game_framebuffer_0;
	Handle<Framebuffer> game_framebuffer_1;
	Handle<Framebuffer> player_outfit_framebuffer;

	Handle<Buffer> dynamic_vertex_buffer;
	Handle<Buffer> dynamic_index_buffer;
	Handle<Buffer> frame_uniform_buffer;
	Handle<Buffer> sprite_uniform_buffer;
	Handle<Buffer> player_outfit_uniform_buffer;

	Handle<Shader> fullscreen_shader;
	Handle<Shader> gaussian_blur_hor_shader;
	Handle<Shader> gaussian_blur_ver_shader;
	Handle<Shader> screen_transition_shader;
	Handle<Shader> shockwave_shader;

	Handle<Shader> grass_shader;
	Handle<Shader> sprite_shader;
	Handle<Shader> shape_shader;
	Handle<Shader> text_shader;
	Handle<Shader> ui_shader;

	Handle<Texture> error_texture; // 32x32 magenta-black checkerboard
	Handle<Texture> white_texture; // 1x1 white texture

	Handle<Sampler> nearest_sampler;
	Handle<Sampler> linear_sampler;

	void initialize_globals()
	{
		game_framebuffer_0 = create_framebuffer({
			.debug_name = "game framebuffer 0",
			.width = GAME_FRAMEBUFFER_WIDTH,
			.height = GAME_FRAMEBUFFER_HEIGHT });
		game_framebuffer_1 = create_framebuffer({
			.debug_name = "game framebuffer 1",
			.width = GAME_FRAMEBUFFER_WIDTH,
			.height = GAME_FRAMEBUFFER_HEIGHT });
		player_outfit_framebuffer = create_framebuffer({
			.debug_name = "player outfit framebuffer",
			.width = 1024, // size of player tileset
			.height = 1024 });

		dynamic_vertex_buffer = create_buffer({
			.debug_name = "dynamic vertex buffer",
			.size = 8192 * sizeof(Vertex), // 8192 is an initial estimate
			.dynamic = true });
		dynamic_index_buffer = create_buffer({
			.debug_name = "dynamic index buffer",
			.size = 8192 * sizeof(unsigned int), // 8192 is an initial estimate
			.dynamic = true });
		frame_uniform_buffer = create_buffer({
			.debug_name = "frame uniform buffer",
			.size = sizeof(FrameUniformBlock),
			.dynamic = true });
		sprite_uniform_buffer = create_buffer({
			.debug_name = "sprite uniform buffer",
			.size = 256 * 256, // estimate we won't need more than 256 custom blocks on screen at same time
			.dynamic = true });
		player_outfit_uniform_buffer = create_buffer({
			.debug_name = "player outfit uniform buffer",
			.size = sizeof(PlayerOutfitUniformBlock),
			.dynamic = true });

		fullscreen_shader = load_shader(
			"assets/shaders/fullscreen.vert",
			"assets/shaders/fullscreen.frag");
		gaussian_blur_hor_shader = load_shader(
			"assets/shaders/fullscreen.vert",
			"assets/shaders/gaussian_blur_hor.frag");
		gaussian_blur_ver_shader = load_shader(
			"assets/shaders/fullscreen.vert",
			"assets/shaders/gaussian_blur_ver.frag");
		screen_transition_shader = load_shader(
			"assets/shaders/fullscreen.vert",
			"assets/shaders/screen_transition.frag");
		shockwave_shader = load_shader(
			"assets/shaders/fullscreen.vert",
			"assets/shaders/shockwave.frag");

		sprite_shader = load_shader(
			"assets/shaders/sprite.vert",
			"assets/shaders/sprite.frag");
		grass_shader = load_shader(
			"assets/shaders/grass.vert",
			"assets/shaders/sprite.frag");
		shape_shader = load_shader(
			"assets/shaders/shape.vert",
			"assets/shaders/shape.frag");
		text_shader = load_shader(
			"assets/shaders/sprite.vert",
			"assets/shaders/text.frag");
		ui_shader = load_shader(
			"assets/shaders/ui.vert",
			"assets/shaders/ui.frag");

		{
			constexpr unsigned int ERROR_TEXTURE_SIZE = 32;
			unsigned char error_texture_data[ERROR_TEXTURE_SIZE * ERROR_TEXTURE_SIZE * 4];
			//Make a magenta-and-black checkerboard pattern
			for (unsigned int i = 0; i < ERROR_TEXTURE_SIZE; ++i) {
				for (unsigned int j = 0; j < ERROR_TEXTURE_SIZE; ++j) {
					unsigned int k = (i * ERROR_TEXTURE_SIZE + j) * 4;
					unsigned char value = (i + j) % 2 == 0 ? 255 : 0;
					error_texture_data[k + 0] = value;
					error_texture_data[k + 1] = 0;
					error_texture_data[k + 2] = value;
					error_texture_data[k + 3] = 255;
				}
			}
			error_texture = create_texture({
				.debug_name = "error texture",
				.width = ERROR_TEXTURE_SIZE,
				.height = ERROR_TEXTURE_SIZE,
				.format = Format::R8G8B8A8_UNORM,
				.initial_data = error_texture_data });
		}
		{
			unsigned char white_texture_data[4] = { 255, 255, 255, 255 };
			white_texture = create_texture({
				.debug_name = "white texture",
				.width = 1,
				.height = 1,
				.format = Format::R8G8B8A8_UNORM,
				.initial_data = white_texture_data });
		}

		nearest_sampler = create_sampler({
			.debug_name = "nearest sampler",
			.filter = Filter::Nearest });
		linear_sampler = create_sampler({
			.debug_name = "linear sampler",
			.filter = Filter::Linear,
			.wrap = Wrap::ClampToEdge });

		bind_vertex_buffer(0, dynamic_vertex_buffer, sizeof(Vertex));
		bind_index_buffer(dynamic_index_buffer);
		bind_uniform_buffer(0, frame_uniform_buffer);
		bind_sampler(0, nearest_sampler);
	}
}