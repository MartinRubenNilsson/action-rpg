#version 430

out vec2 tex_coord;

void main()
{
	const vec2 vertices[4] = vec2[4](
		vec2(-1.0, -1.0),  // bottom-left
		vec2( 1.0, -1.0),  // bottom-right
		vec2(-1.0,  1.0),  // top-left
		vec2( 1.0,  1.0)); // top-right
	const vec2 tex_coords[4] = vec2[4](
		vec2(0.0, 0.0),  // bottom-left
		vec2(1.0, 0.0),  // bottom-right
		vec2(0.0, 1.0),  // top-left
		vec2(1.0, 1.0)); // top-right
	gl_Position = vec4(vertices[gl_VertexID], 0.0, 1.0);
	tex_coord = tex_coords[gl_VertexID];
}