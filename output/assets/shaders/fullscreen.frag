#version 460

uniform sampler2D tex;

layout(location = 0) in vec2 tex_coord;

layout(location = 0) out vec4 frag_color;

void main() {
	frag_color = texture(tex, tex_coord);
}