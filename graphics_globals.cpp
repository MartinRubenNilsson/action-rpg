#include "stdafx.h"
#include "graphics_globals.h"
#include "graphics.h"

namespace graphics
{
	enum UNIFORM_BUFFER_BINDING
	{
		UNIFORM_BUFFER_BINDING_FRAME = 0,
	};

	extern const float IDENTITY_MATRIX[16] = {
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f,
	};

	Handle<Shader> sprite_shader;
	Handle<Shader> fullscreen_shader;
	Handle<Shader> shape_shader;
	Handle<Shader> text_shader;
	Handle<Shader> ui_shader;

	Handle<Buffer> vertex_buffer;
	Handle<Buffer> index_buffer;

	Handle<Buffer> frame_uniform_buffer;

	void initialize_globals()
	{
		sprite_shader = load_shader(
			"assets/shaders/sprite.vert",
			"assets/shaders/sprite.frag");
		fullscreen_shader = load_shader(
			"assets/shaders/fullscreen.vert",
			"assets/shaders/fullscreen.frag");
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

		vertex_buffer = create_buffer({
			.debug_name = "vertex buffer",
			.type = BufferType::Vertex,
			.usage = Usage::DynamicDraw,
			.byte_size = sizeof(Vertex) * MAX_VERTICES });

		index_buffer = create_buffer({
			.debug_name = "index buffer",
			.type = BufferType::Index,
			.usage = Usage::DynamicDraw,
			.byte_size = sizeof(unsigned int) * MAX_INDICES });

		frame_uniform_buffer = create_buffer({
			.debug_name = "frame uniform buffer",
			.type = BufferType::Uniform,
			.usage = Usage::DynamicDraw,
			.byte_size = sizeof(FrameUniforms) });

		bind_vertex_buffer(0, vertex_buffer, sizeof(Vertex));
		bind_index_buffer(index_buffer);
		bind_uniform_buffer(UNIFORM_BUFFER_BINDING_FRAME, frame_uniform_buffer);
	}
}