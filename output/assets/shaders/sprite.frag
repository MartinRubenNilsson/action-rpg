#version 450

uniform sampler2D tex;

in vec4 color;
in vec2 tex_coord;
out vec4 frag_color;

void main()
{
	frag_color = color * texture(tex, tex_coord);
}