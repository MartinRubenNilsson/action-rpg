cbuffer ShockwaveUniformBlock : register(b1)
{
    float2 _21_resolution : packoffset(c0);
    float2 _21_center : packoffset(c0.z);
    float _21_force : packoffset(c1);
    float _21_size : packoffset(c1.y);
    float _21_thickness : packoffset(c1.z);
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

float2 map_target_to_texture(float2 pos)
{
    return float2(pos.x / _21_resolution.x, pos.y / _21_resolution.y);
}

void frag_main()
{
    float aspectRatio = _21_resolution.y / _21_resolution.x;
    float2 uv = tex_coord;
    float2 scaledUV = uv * float2(1.0f, aspectRatio);
    float2 param = _21_center;
    float2 correctedCenter = map_target_to_texture(param) * float2(1.0f, aspectRatio);
    float dist = length(scaledUV - correctedCenter);
    float mask = (1.0f - smoothstep(_21_size - 0.100000001490116119384765625f, _21_size, dist)) * smoothstep((_21_size - _21_thickness) - 0.100000001490116119384765625f, _21_size - _21_thickness, dist);
    float2 disp = (normalize(scaledUV - correctedCenter) * _21_force) * mask;
    float4 originalPixel = tex.Sample(_tex_sampler, uv);
    float4 displacedPixel = tex.Sample(_tex_sampler, uv - disp);
    float4 finalPixel = lerp(originalPixel, displacedPixel, mask.xxxx);
    frag_color = finalPixel;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    tex_coord = stage_input.tex_coord;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.frag_color = frag_color;
    return stage_output;
}
