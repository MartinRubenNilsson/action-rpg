#version 460

layout(location = 0) in vec4 color;

layout(location = 0) out vec4 frag_color;

void main() {
	//TODO: rounded corners
	frag_color = color;
}