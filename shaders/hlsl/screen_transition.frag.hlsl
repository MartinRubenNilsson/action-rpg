cbuffer ScreenTransitionUniformBlock : register(b1)
{
    float _93_progress : packoffset(c0);
};

Texture2D<float4> tex : register(t0);
SamplerState _tex_sampler : register(s0);

static float4 gl_FragCoord;
static float4 frag_color;
static float2 tex_coord;

struct SPIRV_Cross_Input
{
    float2 tex_coord : TEXCOORD0;
    float4 gl_FragCoord : SV_Position;
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

void frag_main()
{
    uint _20_dummy_parameter;
    float2 tex_size = float2(int2(spvTextureSize(tex, uint(0), _20_dummy_parameter)));
    float2 pixel = floor(float2(gl_FragCoord.x, tex_size.y - gl_FragCoord.y));
    float2 pixel_fractions = frac(pixel / 20.0f.xx);
    float2 pixel_distances = abs(pixel_fractions - 0.5f.xx);
    float2 tex_size_in_diamonds = tex_size / 20.0f.xx;
    float max_diamond_distance = tex_size_in_diamonds.x + tex_size_in_diamonds.y;
    float2 diamond = floor(pixel / 20.0f.xx);
    float2 diamond_distances = diamond / max_diamond_distance.xx;
    float pixel_progress = pixel_distances.x + pixel_distances.y;
    float diamond_progress = diamond_distances.x + diamond_distances.y;
    float total_progress = (pixel_progress + diamond_progress) / 2.0f;
    bool _98 = _93_progress < 0.0f;
    bool _107;
    if (_98)
    {
        _107 = (1.0f + _93_progress) < total_progress;
    }
    else
    {
        _107 = _98;
    }
    if (_107)
    {
        frag_color = float4(0.0f, 0.0f, 0.0f, 1.0f);
    }
    else
    {
        bool _116 = _93_progress > 0.0f;
        bool _123;
        if (_116)
        {
            _123 = total_progress < _93_progress;
        }
        else
        {
            _123 = _116;
        }
        if (_123)
        {
            frag_color = float4(0.0f, 0.0f, 0.0f, 1.0f);
        }
        else
        {
            frag_color = tex.Sample(_tex_sampler, gl_FragCoord.xy / tex_size);
        }
    }
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    gl_FragCoord = stage_input.gl_FragCoord;
    gl_FragCoord.w = 1.0 / gl_FragCoord.w;
    tex_coord = stage_input.tex_coord;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.frag_color = frag_color;
    return stage_output;
}
