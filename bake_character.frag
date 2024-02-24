uniform sampler2D texture;
uniform sampler2D ramps_texture;
uniform int palette;
uniform int color;

#define PALETTE_SKIN 0
#define PALETTE_HAIR 1
#define PALETTE_A 2
#define PALETTE_B 3
#define PALETTE_C 4
#define PALETTE_D 5

int palette_skin_to_offset(ivec3 color)
{
	if (color == ivec3(248, 240, 224)) return 0;
    if (color == ivec3(248, 216, 184)) return 1;
    if (color == ivec3(216, 152, 120)) return 2;
    if (color == ivec3(136, 88, 72)) return 3;
    if (color == ivec3(24, 24, 24)) return 4;
    return -1;
}

vec3 fetch_ramp_color(int offset)
{
    ivec2 fetch_pos = ivec2(offset, color) * 2; // lookup texture has 2x2 pixels for some reason
	return texelFetch(ramps_texture, fetch_pos, 0).rgb;
}

void main()
{
	ivec2 texture_size = textureSize(texture, 0);
    ivec2 fetch_pos = ivec2(gl_FragCoord.x, texture_size.y - gl_FragCoord.y);
    vec4 pixel = texelFetch(texture, fetch_pos, 0);
    ivec3 color = ivec3(pixel.rgb * 255.0);

    switch (palette) {
    case PALETTE_SKIN:
        int offset = palette_skin_to_offset(color);
        if (offset != -1) {
			pixel.rgb = fetch_ramp_color(offset);
		}
		break;
    case PALETTE_HAIR:
    case PALETTE_A:
    case PALETTE_B:
    case PALETTE_C:
    case PALETTE_D:
        break;
    }

    gl_FragColor = pixel;
}