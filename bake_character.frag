uniform sampler2D texture;
uniform sampler2D lut;
uniform int lut_type;
uniform int lut_row;

#define LUT_SKIN 0
#define LUT_HAIR 1
#define LUT_C3 2

int skin_base_color_to_lut_column(ivec3 rgb)
{
	if (rgb == ivec3(248, 240, 224)) return 0;
    if (rgb == ivec3(248, 216, 184)) return 1;
    if (rgb == ivec3(216, 152, 120)) return 2;
    if (rgb == ivec3(136, 88, 72)) return 3;
    if (rgb == ivec3(24, 24, 24)) return 4;
    return -1;
}

int hair_base_color_to_lut_column(ivec3 rgb)
{
	if (rgb == ivec3(248, 240, 224)) return 0;
    if (rgb == ivec3(200, 200, 184)) return 1;
    if (rgb == ivec3(136, 152, 160)) return 2;
    if (rgb == ivec3(104, 104, 120)) return 3;
    if (rgb == ivec3(80, 56, 80)) return 4;
    if (rgb == ivec3(24, 24, 24)) return 5;
    return -1;
}

int c3_base_color_to_lut_column(ivec3 rgb)
{
	if (rgb == ivec3(88, 224, 160)) return 0;
    if (rgb == ivec3(40, 152, 96)) return 1;
    if (rgb == ivec3(32, 80, 64)) return 2;
    if (rgb == ivec3(24, 24, 24)) return 3;
    return -1;
}

vec3 sample_lut(int column, int row)
{
    // The lookup textures has 2x2 pixels for some reason, hence the * 2
	return texelFetch(lut, ivec2(column, row) * 2, 0).rgb;
}

void main()
{
	ivec2 texture_size = textureSize(texture, 0);
    ivec2 fetch_pos = ivec2(gl_FragCoord.x, texture_size.y - gl_FragCoord.y);
    vec4 pixel = texelFetch(texture, fetch_pos, 0);

    int lut_column = -1;

    switch (lut_type) {
    case LUT_SKIN:
        lut_column = skin_base_color_to_lut_column(ivec3(pixel.rgb * 255));
	    break;
    case LUT_HAIR:
        lut_column = hair_base_color_to_lut_column(ivec3(pixel.rgb * 255));
	    break;
    case LUT_C3:
		lut_column = c3_base_color_to_lut_column(ivec3(pixel.rgb * 255));
        break;
    }

    if (lut_column != -1 && lut_row != -1)
		pixel.rgb = sample_lut(lut_column, lut_row);

    gl_FragColor = pixel;
}