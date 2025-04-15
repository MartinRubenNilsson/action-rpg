Texture2D<float4> tex : register(t0);
SamplerState _tex_sampler : register(s0);

static float2 tex_coord;
static float4 frag_color;
static float4 color;

struct SPIRV_Cross_Input
{
    float4 color : TEXCOORD0;
    float2 tex_coord : TEXCOORD1;
};

struct SPIRV_Cross_Output
{
    float4 frag_color : SV_Target0;
};

void frag_main()
{
    float alpha = tex.Sample(_tex_sampler, tex_coord).x;
    frag_color.x = color.xyz.x;
    frag_color.y = color.xyz.y;
    frag_color.z = color.xyz.z;
    frag_color.w = color.w * alpha;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    tex_coord = stage_input.tex_coord;
    color = stage_input.color;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.frag_color = frag_color;
    return stage_output;
}
