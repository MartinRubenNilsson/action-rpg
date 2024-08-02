#version 430

layout(binding = 0) uniform sampler2D tex;
layout(binding = 1) uniform sampler2D lut1;
layout(binding = 2) uniform sampler2D lut2;

uniform int lut1_type;
uniform int lut1_y;
uniform int lut2_type;
uniform int lut2_y;

in vec2 tex_coord;
out vec4 frag_color;

#define LUT_SKIN 0
#define LUT_HAIR 1
#define LUT_C3 2
#define LUT_C4 3

int skin_base_color_to_lut_x(ivec3 rgb)
{
	if (rgb == ivec3(248, 240, 224)) return 0;
    if (rgb == ivec3(248, 216, 184)) return 1;
    if (rgb == ivec3(216, 152, 120)) return 2;
    if (rgb == ivec3(136, 88, 72)) return 3;
    if (rgb == ivec3(24, 24, 24)) return 4;
    return -1;
}

int hair_base_color_to_lut_x(ivec3 rgb)
{
	if (rgb == ivec3(248, 240, 224)) return 0;
    if (rgb == ivec3(200, 200, 184)) return 1;
    if (rgb == ivec3(136, 152, 160)) return 2;
    if (rgb == ivec3(104, 104, 120)) return 3;
    if (rgb == ivec3(80, 56, 80)) return 4;
    if (rgb == ivec3(24, 24, 24)) return 5;
    return -1;
}

int c3_base_color_to_lut_x(ivec3 rgb)
{
	if (rgb == ivec3(88, 224, 160)) return 0;
    if (rgb == ivec3(40, 152, 96)) return 1;
    if (rgb == ivec3(32, 80, 64)) return 2;
    if (rgb == ivec3(24, 24, 24)) return 3;
    return -1;
}

int c4_base_color_to_lut_x(ivec3 rgb)
{
	if (rgb == ivec3(250, 182, 193)) return 0;
    if (rgb == ivec3(215, 110, 162)) return 1;
    if (rgb == ivec3(183, 67, 123)) return 2;
    if (rgb == ivec3(91, 42, 84)) return 3;
    if (rgb == ivec3(24, 24, 24)) return 4;
    return -1;
}

bool lookup_color(sampler2D lut, int lut_type, int lut_y, inout vec3 color)
{
    if (lut_type == -1) return false;
    if (lut_y == -1) return false;
    int lut_x = -1;
    switch (lut_type) {
    case LUT_SKIN:
        lut_x = skin_base_color_to_lut_x(ivec3(color * 255));
	    break;
    case LUT_HAIR:
        lut_x = hair_base_color_to_lut_x(ivec3(color * 255));
	    break;
    case LUT_C3:
		lut_x = c3_base_color_to_lut_x(ivec3(color * 255));
        break;
    case LUT_C4:
		lut_x = c4_base_color_to_lut_x(ivec3(color * 255));
		break;
    }
    if (lut_x == -1) return false;
    // The lookup textures has 2x2 pixels for some reason, hence the * 2
    ivec2 tex_size = textureSize(lut, 0);
    color = texelFetch(lut, ivec2(lut_x, lut_y) * 2, 0).xyz;
    return true;
}

void main()
{
    frag_color = texture(tex, tex_coord);
    if (lookup_color(lut1, lut1_type, lut1_y, frag_color.xyz)) return;
    if (lookup_color(lut2, lut2_type, lut2_y, frag_color.xyz)) return;
}