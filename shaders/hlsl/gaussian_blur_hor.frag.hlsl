static const float _56[3] = { 0.0f, 1.384615421295166015625f, 3.23076915740966796875f };
static const float _71[3] = { 0.2270270287990570068359375f, 0.3162162303924560546875f, 0.0702702701091766357421875f };

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
    uint _19_dummy_parameter;
    int2 tex_size = int2(spvTextureSize(tex, uint(0), _19_dummy_parameter));
    frag_color = tex.Sample(_tex_sampler, gl_FragCoord.xy / float2(tex_size)) * 0.2270270287990570068359375f;
    for (int i = 1; i < 3; i++)
    {
        frag_color += (tex.Sample(_tex_sampler, (gl_FragCoord.xy + float2(_56[i], 0.0f)) / float2(tex_size)) * _71[i]);
        frag_color += (tex.Sample(_tex_sampler, (gl_FragCoord.xy - float2(_56[i], 0.0f)) / float2(tex_size)) * _71[i]);
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
