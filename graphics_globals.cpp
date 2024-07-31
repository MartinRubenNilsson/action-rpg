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

	Handle<Shader> sprite_shader;
	Handle<Shader> fullscreen_shader;
	Handle<Shader> shape_shader;
	Handle<Shader> ui_shader;

	void initialize_globals()
	{


		sprite_shader = load_shader(
			"assets/shaders/sprite.vert",
			"assets/shaders/sprite.frag");
		fullscreen_shader = load_shader(
			"assets/shaders/fullscreen_triangle.vert",
			"assets/shaders/fullscreen.frag");
		shape_shader = load_shader(
			"assets/shaders/shape.vert",
			"assets/shaders/shape.frag");
		ui_shader = load_shader(
			"assets/shaders/ui.vert",
			"assets/shaders/ui.frag");
	}
}