#version 450

layout(std140, binding = 0) uniform FrameUniformBlock
{
	mat4 view_proj_matrix;
	float app_time;
	float game_time;
};

layout(location = 0) in vec2 vertex_position;
layout(location = 1) in vec4 vertex_color;
layout(location = 2) in vec2 vertex_tex_coord;
out vec4 color;
out vec2 tex_coord;

void main()
{
	float sway_intensity = clamp(1.0 - vertex_tex_coord.y, 0.0, 1.0) * (0.5 + 0.2 * sin(0.3 * game_time));
	float sway_time_offset = vertex_tex_coord.x * 2.0;
	float sway_time_frequency = 2.0;
	float sway_x = 0.9 * sway_intensity * sin(1.927 * sway_time_frequency * (game_time - sway_time_offset));
	float sway_y = 0.4 * sway_intensity * sin(1.284 * sway_time_frequency * (game_time - sway_time_offset));
	vec2 position = vertex_position + vec2(sway_x, sway_y);
	gl_Position = view_proj_matrix * vec4(position, 0.0, 1.0);
	color = vertex_color;
	tex_coord = vertex_tex_coord;
}