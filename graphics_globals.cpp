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

	Handle<Framebuffer> gameworld_framebuffer_target;
	Handle<Framebuffer> gameworld_framebuffer_source;
	Handle<Framebuffer> player_outfit_framebuffer;

	Handle<Buffer> dynamic_vertex_buffer;
	Handle<Buffer> dynamic_index_buffer;
	Handle<Buffer> frame_uniform_buffer;
	Handle<Buffer> sprite_uniform_buffer;

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

	void initialize_globals()
	{
		gameworld_framebuffer_target = create_framebuffer({
			.debug_name = "gameworld framebuffer 0",
			.width = GAMEWORLD_FRAMEBUFFER_WIDTH,
			.height = GAMEWORLD_FRAMEBUFFER_HEIGHT });
		gameworld_framebuffer_source = create_framebuffer({
			.debug_name = "gameworld framebuffer 1",
			.width = GAMEWORLD_FRAMEBUFFER_WIDTH,
			.height = GAMEWORLD_FRAMEBUFFER_HEIGHT });
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

		bind_vertex_buffer(0, dynamic_vertex_buffer, sizeof(Vertex));
		bind_index_buffer(dynamic_index_buffer);
		bind_uniform_buffer(0, frame_uniform_buffer);
	}
}