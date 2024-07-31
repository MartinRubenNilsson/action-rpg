#pragma once

namespace graphics
{
	extern const float IDENTITY_MATRIX[16];

	extern Handle<Shader> sprite_shader;
	extern Handle<Shader> fullscreen_shader;
	extern Handle<Shader> shape_shader;
	extern Handle<Shader> ui_shader;

	void initialize_globals();
}