#version 430

uniform mat4 view_proj_matrix;

layout (location = 0) in vec2 vertex_position;
layout (location = 1) in vec4 vertex_color;
layout (location = 2) in vec2 vertex_tex_coord;
out vec4 color;
out vec2 tex_coord;

void main()
{
	gl_Position = view_proj_matrix * vec4(vertex_position, 0.0, 1.0);
	color = vertex_color;
	tex_coord = vertex_tex_coord;
}