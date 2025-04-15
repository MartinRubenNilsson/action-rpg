cbuffer PlayerOutfitUniformBlock : register(b1)
{
    int _302_lut1_type : packoffset(c0);
    int _302_lut1_y : packoffset(c0.y);
    int _302_lut2_type : packoffset(c0.z);
    int _302_lut2_y : packoffset(c0.w);
};

Texture2D<float4> tex : register(t0);
SamplerState _tex_sampler : register(s0);
Texture2D<float4> lut1 : register(t1);
SamplerState _lut1_sampler : register(s1);
Texture2D<float4> lut2 : register(t2);
SamplerState _lut2_sampler : register(s2);

static float4 frag_color;
static float2 tex_coord;

struct SPIRV_Cross_Input
{
    float2 tex_coord : TEXCOORD0;
};

struct SPIRV_Cross_Output
{
    float4 frag_color : SV_Target0;
};

uint2 spvTextureSize(Texture2D<float4> Tex, uint Level, out uint Param)
{
    uint2 ret;
    Tex.GetDimensions(Level, ret.x, ret.y, Param);
    return ret;
}

int skin_base_color_to_lut_x(int3 rgb)
{
    if (all(bool3(rgb.x == int3(248, 240, 224).x, rgb.y == int3(248, 240, 224).y, rgb.z == int3(248, 240, 224).z)))
    {
        return 0;
    }
    if (all(bool3(rgb.x == int3(248, 216, 184).x, rgb.y == int3(248, 216, 184).y, rgb.z == int3(248, 216, 184).z)))
    {
        return 1;
    }
    if (all(bool3(rgb.x == int3(216, 152, 120).x, rgb.y == int3(216, 152, 120).y, rgb.z == int3(216, 152, 120).z)))
    {
        return 2;
    }
    if (all(bool3(rgb.x == int3(136, 88, 72).x, rgb.y == int3(136, 88, 72).y, rgb.z == int3(136, 88, 72).z)))
    {
        return 3;
    }
    if (all(bool3(rgb.x == int3(24, 24, 24).x, rgb.y == int3(24, 24, 24).y, rgb.z == int3(24, 24, 24).z)))
    {
        return 4;
    }
    return -1;
}

int hair_base_color_to_lut_x(int3 rgb)
{
    if (all(bool3(rgb.x == int3(248, 240, 224).x, rgb.y == int3(248, 240, 224).y, rgb.z == int3(248, 240, 224).z)))
    {
        return 0;
    }
    if (all(bool3(rgb.x == int3(200, 200, 184).x, rgb.y == int3(200, 200, 184).y, rgb.z == int3(200, 200, 184).z)))
    {
        return 1;
    }
    if (all(bool3(rgb.x == int3(136, 152, 160).x, rgb.y == int3(136, 152, 160).y, rgb.z == int3(136, 152, 160).z)))
    {
        return 2;
    }
    if (all(bool3(rgb.x == int3(104, 104, 120).x, rgb.y == int3(104, 104, 120).y, rgb.z == int3(104, 104, 120).z)))
    {
        return 3;
    }
    if (all(bool3(rgb.x == int3(80, 56, 80).x, rgb.y == int3(80, 56, 80).y, rgb.z == int3(80, 56, 80).z)))
    {
        return 4;
    }
    if (all(bool3(rgb.x == int3(24, 24, 24).x, rgb.y == int3(24, 24, 24).y, rgb.z == int3(24, 24, 24).z)))
    {
        return 5;
    }
    return -1;
}

int c3_base_color_to_lut_x(int3 rgb)
{
    if (all(bool3(rgb.x == int3(88, 224, 160).x, rgb.y == int3(88, 224, 160).y, rgb.z == int3(88, 224, 160).z)))
    {
        return 0;
    }
    if (all(bool3(rgb.x == int3(40, 152, 96).x, rgb.y == int3(40, 152, 96).y, rgb.z == int3(40, 152, 96).z)))
    {
        return 1;
    }
    if (all(bool3(rgb.x == int3(32, 80, 64).x, rgb.y == int3(32, 80, 64).y, rgb.z == int3(32, 80, 64).z)))
    {
        return 2;
    }
    if (all(bool3(rgb.x == int3(24, 24, 24).x, rgb.y == int3(24, 24, 24).y, rgb.z == int3(24, 24, 24).z)))
    {
        return 3;
    }
    return -1;
}

int c4_base_color_to_lut_x(int3 rgb)
{
    if (all(bool3(rgb.x == int3(250, 182, 193).x, rgb.y == int3(250, 182, 193).y, rgb.z == int3(250, 182, 193).z)))
    {
        return 0;
    }
    if (all(bool3(rgb.x == int3(215, 110, 162).x, rgb.y == int3(215, 110, 162).y, rgb.z == int3(215, 110, 162).z)))
    {
        return 1;
    }
    if (all(bool3(rgb.x == int3(183, 67, 123).x, rgb.y == int3(183, 67, 123).y, rgb.z == int3(183, 67, 123).z)))
    {
        return 2;
    }
    if (all(bool3(rgb.x == int3(91, 42, 84).x, rgb.y == int3(91, 42, 84).y, rgb.z == int3(91, 42, 84).z)))
    {
        return 3;
    }
    if (all(bool3(rgb.x == int3(24, 24, 24).x, rgb.y == int3(24, 24, 24).y, rgb.z == int3(24, 24, 24).z)))
    {
        return 4;
    }
    return -1;
}

bool lookup_color(Texture2D<float4> lut, SamplerState _lut_sampler, int lut_type, int lut_y, inout float3 color)
{
    if (lut_type == (-1))
    {
        return false;
    }
    if (lut_y == (-1))
    {
        return false;
    }
    int lut_x = -1;
    switch (lut_type)
    {
        case 0:
        {
            int3 param = int3(color * 255.0f);
            lut_x = skin_base_color_to_lut_x(param);
            break;
        }
        case 1:
        {
            int3 param_1 = int3(color * 255.0f);
            lut_x = hair_base_color_to_lut_x(param_1);
            break;
        }
        case 2:
        {
            int3 param_2 = int3(color * 255.0f);
            lut_x = c3_base_color_to_lut_x(param_2);
            break;
        }
        case 3:
        {
            int3 param_3 = int3(color * 255.0f);
            lut_x = c4_base_color_to_lut_x(param_3);
            break;
        }
    }
    if (lut_x == (-1))
    {
        return false;
    }
    uint _276_dummy_parameter;
    int2 tex_size = int2(spvTextureSize(lut, uint(0), _276_dummy_parameter));
    color = lut.Load(int3(int2(lut_x, lut_y) * int2(2, 2), 0)).xyz;
    return true;
}

void frag_main()
{
    frag_color = tex.Sample(_tex_sampler, tex_coord);
    int param = _302_lut1_type;
    int param_1 = _302_lut1_y;
    float3 param_2 = frag_color.xyz;
    bool _313 = lookup_color(lut1, _lut1_sampler, param, param_1, param_2);
    frag_color.x = param_2.x;
    frag_color.y = param_2.y;
    frag_color.z = param_2.z;
    if (_313)
    {
        return;
    }
    int param_3 = _302_lut2_type;
    int param_4 = _302_lut2_y;
    float3 param_5 = frag_color.xyz;
    bool _339 = lookup_color(lut2, _lut2_sampler, param_3, param_4, param_5);
    frag_color.x = param_5.x;
    frag_color.y = param_5.y;
    frag_color.z = param_5.z;
    if (_339)
    {
        return;
    }
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    tex_coord = stage_input.tex_coord;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.frag_color = frag_color;
    return stage_output;
}
