#version 460

out gl_PerVertex {
	vec4 gl_Position;
};

layout(location = 0) out vec2 tex_coord;

void main(){ 
	// We draw a fullscreen-covering triangle that looks like this:
	//  ______
	// |  |  /
	// |  | /
	// |__|/
	// |  /
	// | /
	// |/
	// 
	// PITFALL: We use an unusual clip space coordinate system where y is down.
	// This makes it easier to handle some differences between OpenGL and D3D11.
	// Moreover, it means the shader coordinate axes point the same as the game world.
	//
	const vec2 vertices[3] = vec2[3](
		vec2(-1.0, -1.0), // top left
		vec2(3.0, -1.0), // top right
		vec2(-1.0, 3.0) // bottom
	);
	const vec2 tex_coords[3] = vec2[3](
		vec2(0.0, 0.0),
		vec2(2.0, 0.0),
		vec2(0.0, 2.0)
	);
	gl_Position = vec4(vertices[gl_VertexID], 0.0, 1.0);
	tex_coord = tex_coords[gl_VertexID];
}