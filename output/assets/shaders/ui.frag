#version 450

uniform sampler2D tex;
uniform bool has_tex;

in vec4 color;
in vec2 tex_coord;
out vec4 frag_color;

void main()
{
	frag_color = color;
	if (has_tex) {
		frag_color *= texture(tex, tex_coord);
	}
}