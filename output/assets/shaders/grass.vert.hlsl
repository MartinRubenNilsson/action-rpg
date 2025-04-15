cbuffer GrassUniformBlock : register(b1)
{
    float2 _16_position : packoffset(c0);
    float2 _16_tex_min : packoffset(c0.z);
    float2 _16_tex_max : packoffset(c1);
};

cbuffer FrameUniformBlock : register(b0)
{
    row_major float4x4 _46_view_proj_matrix : packoffset(c0);
    float _46_app_time : packoffset(c4);
    float _46_game_time : packoffset(c4.y);
};


static float4 gl_Position;
static float2 vertex_tex_coord;
static float2 vertex_position;
static float4 color;
static float4 vertex_color;
static float2 tex_coord;

struct SPIRV_Cross_Input
{
    float2 vertex_position : TEXCOORD0;
    float4 vertex_color : TEXCOORD1;
    float2 vertex_tex_coord : TEXCOORD2;
};

struct SPIRV_Cross_Output
{
    float4 color : TEXCOORD0;
    float2 tex_coord : TEXCOORD1;
    float4 gl_Position : SV_Position;
};

void vert_main()
{
    float2 local_tex_coord = (vertex_tex_coord - _16_tex_min) / (_16_tex_max - _16_tex_min);
    float sway_intensity = clamp(1.0f - local_tex_coord.y, 0.0f, 1.0f) * (0.5f + (0.300000011920928955078125f * sin(0.300000011920928955078125f * _46_game_time)));
    float sway_time_offset = (local_tex_coord.x * 2.0f) + (0.008000000379979610443115234375f * dot(_16_position, float2(1.0f, 2.0f)));
    float sway_x = (1.2999999523162841796875f * sway_intensity) * sin(3.854000091552734375f * (_46_game_time - sway_time_offset));
    float sway_y = (0.800000011920928955078125f * sway_intensity) * sin(2.5680000782012939453125f * (_46_game_time - sway_time_offset));
    gl_Position = mul(float4(vertex_position + float2(sway_x, sway_y), 0.0f, 1.0f), _46_view_proj_matrix);
    color = vertex_color;
    tex_coord = vertex_tex_coord;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    vertex_tex_coord = stage_input.vertex_tex_coord;
    vertex_position = stage_input.vertex_position;
    vertex_color = stage_input.vertex_color;
    vert_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    stage_output.color = color;
    stage_output.tex_coord = tex_coord;
    return stage_output;
}
