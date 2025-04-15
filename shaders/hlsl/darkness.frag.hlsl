cbuffer DarknessUniformBlock : register(b1)
{
    float2 _13_resolution : packoffset(c0);
    float2 _13_center : packoffset(c0.z);
    float _13_intensity : packoffset(c1);
};

Texture2D<float4> tex : register(t0);
SamplerState _tex_sampler : register(s0);

static float2 tex_coord;
static float4 frag_color;

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
    float aspect = _13_resolution.y / _13_resolution.x;
    float2 uv = tex_coord;
    float2 corrected_uv = uv * float2(1.0f, aspect);
    float2 corrected_center = (_13_center / _13_resolution) * float2(1.0f, aspect);
    float dist = length(corrected_uv - corrected_center);
    float mask = 1.0f - smoothstep(-0.0500000007450580596923828125f, 0.300000011920928955078125f, dist);
    mask *= smoothstep(0.1500000059604644775390625f, 0.0500000007450580596923828125f, dist);
    mask = clamp(mask, 0.0f, 1.0f);
    float3 _78 = tex.Sample(_tex_sampler, uv).xyz;
    frag_color.x = _78.x;
    frag_color.y = _78.y;
    frag_color.z = _78.z;
    float4 _93 = frag_color;
    float3 _95 = _93.xyz * lerp(1.0f - _13_intensity, 1.0f, mask);
    frag_color.x = _95.x;
    frag_color.y = _95.y;
    frag_color.z = _95.z;
    frag_color.w = 1.0f;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    tex_coord = stage_input.tex_coord;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.frag_color = frag_color;
    return stage_output;
}
