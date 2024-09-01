#version 450

//TODO: all of these can be combined into a single matrix
uniform mat4 transform;
uniform vec2 translation;
uniform vec2 viewport_size;

layout(location = 0) in vec2 vertex_position;
layout(location = 1) in vec4 vertex_color;
layout(location = 2) in vec2 vertex_tex_coord;
out vec4 color;
out vec2 tex_coord;

void main()
{
	vec2 position = vertex_position;
	position += translation;
	position = (transform * vec4(position, 0.0, 1.0)).xy;
	position = position / viewport_size * 2.0 - 1.0;
	position.y = -position.y;
	gl_Position = vec4(position, 0.0, 1.0);
	color = vertex_color;
	tex_coord = vertex_tex_coord;
}