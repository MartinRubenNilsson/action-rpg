#version 430

uniform sampler2D tex;

in vec4 color;
in vec2 tex_coord;
out vec4 frag_color;

void main()
{
	float alpha = texture(tex, tex_coord).r;
	frag_color.rgb = color.rgb;
	frag_color.a = color.a * alpha;
}