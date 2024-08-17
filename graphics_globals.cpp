#include "stdafx.h"
#include "graphics_globals.h"
#include "graphics.h"

namespace graphics
{
	enum UNIFORM_BLOCK_BINDING
	{
		UNIFORM_BLOCK_BINDING_FRAME = 0,
	};

	extern const float IDENTITY_MATRIX[16] = {
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f,
	};

	eastl::vector<graphics::Vertex> temp_vertices;

	Handle<Shader> fullscreen_shader;
	Handle<Shader> grass_shader;
	Handle<Shader> sprite_shader;
	Handle<Shader> shape_shader;
	Handle<Shader> text_shader;
	Handle<Shader> ui_shader;

	Handle<Buffer> dynamic_vertex_buffer;
	Handle<Buffer> dynamic_index_buffer;

	Handle<Buffer> frame_uniform_buffer;

	void initialize_globals()
	{
		fullscreen_shader = load_shader(
			"assets/shaders/fullscreen.vert",
			"assets/shaders/fullscreen.frag");
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

		constexpr unsigned int MAX_VERTICES = 8192;
		constexpr unsigned int MAX_INDICES = 8192;

		dynamic_vertex_buffer = create_buffer({
			.debug_name = "dynamic vertex buffer",
			.size = MAX_VERTICES * sizeof(Vertex),
			.dynamic = true });

		dynamic_index_buffer = create_buffer({
			.debug_name = "dynamic index buffer",
			.size = MAX_INDICES * sizeof(unsigned int),
			.dynamic = true });

		frame_uniform_buffer = create_buffer({
			.debug_name = "frame uniform buffer",
			.size = sizeof(FrameUniformBlock),
			.dynamic = true });

		bind_vertex_buffer(0, dynamic_vertex_buffer, sizeof(Vertex));
		bind_index_buffer(dynamic_index_buffer);
		bind_uniform_buffer(UNIFORM_BLOCK_BINDING_FRAME, frame_uniform_buffer);
	}
}