#version 460

out vec4 gl_Position;
out vec2 tex_coord;

void main()
{
	const vec2 vertices[3] = vec2[3](
		vec2(-1.0, -1.0),
		vec2(3.0, -1.0),
		vec2(-1.0, 3.0));
	const vec2 tex_coords[3] = vec2[3](
		vec2(0.0, 0.0),
		vec2(2.0, 0.0),
		vec2(0.0, 2.0));
	gl_Position = vec4(vertices[gl_VertexID], 0.0, 1.0);
	tex_coord = tex_coords[gl_VertexID];
}