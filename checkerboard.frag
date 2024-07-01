#version 430

uniform ivec2 tile_size;
uniform vec4 color1;
uniform vec4 color2;

void main()
{
	vec2 tile = floor(gl_FragCoord.xy / tile_size);
	gl_FragColor = mod(tile.x + tile.y, 2.0) < 0.5 ? color1 : color2;
}