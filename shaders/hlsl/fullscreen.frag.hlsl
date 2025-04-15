Texture2D<float4> tex : register(t0);
SamplerState _tex_sampler : register(s0);

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

void frag_main()
{
    frag_color = tex.Sample(_tex_sampler, tex_coord);
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    tex_coord = stage_input.tex_coord;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.frag_color = frag_color;
    return stage_output;
}
