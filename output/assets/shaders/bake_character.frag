uniform sampler2D texture;
uniform sampler2D lut;
uniform int palette;
uniform int color;

#define PALETTE_SKIN 0
#define PALETTE_HAIR 1
#define PALETTE_A 2
#define PALETTE_B 3
#define PALETTE_C 4
#define PALETTE_D 5

int skin_base_color_to_lut_row(ivec3 color)
{
	if (color == ivec3(248, 240, 224)) return 0;
    if (color == ivec3(248, 216, 184)) return 1;
    if (color == ivec3(216, 152, 120)) return 2;
    if (color == ivec3(136, 88, 72)) return 3;
    if (color == ivec3(24, 24, 24)) return 4;
    return -1;
}

int hair_base_color_to_lut_row(ivec3 color)
{
	if (color == ivec3(248, 240, 224)) return 0;
    if (color == ivec3(200, 200, 184)) return 1;
    if (color == ivec3(136, 152, 160)) return 2;
    if (color == ivec3(104, 104, 120)) return 3;
    if (color == ivec3(80, 56, 80)) return 4;
    if (color == ivec3(24, 24, 24)) return 5;
    return -1;
}

vec3 sample_lut(int row)
{
    ivec2 fetch_pos = ivec2(row, color) * 2; // lookup texture has 2x2 pixels for some reason
	return texelFetch(lut, fetch_pos, 0).rgb;
}

void main()
{
	ivec2 texture_size = textureSize(texture, 0);
    ivec2 fetch_pos = ivec2(gl_FragCoord.x, texture_size.y - gl_FragCoord.y);
    vec4 pixel = texelFetch(texture, fetch_pos, 0);
    ivec3 color = ivec3(pixel.rgb * 255.0);

    int offset = -1;

    switch (palette) {
    case PALETTE_SKIN:
        offset = skin_base_color_to_lut_row(color);
	    break;
    case PALETTE_HAIR:
        offset = hair_base_color_to_lut_row(color);
	    break;
    case PALETTE_A:
    case PALETTE_B:
    case PALETTE_C:
    case PALETTE_D:
        break;
    }

    if (offset != -1)
		pixel.rgb = sample_lut(offset);

    gl_FragColor = pixel;
}