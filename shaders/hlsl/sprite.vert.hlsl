cbuffer FrameUniformBlock : register(b0)
{
    row_major float4x4 _17_view_proj_matrix : packoffset(c0);
    float _17_app_time : packoffset(c4);
    float _17_game_time : packoffset(c4.y);
};


static float4 gl_Position;
static float2 vertex_position;
static float4 color;
static float4 vertex_color;
static float2 tex_coord;
static float2 vertex_tex_coord;

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
    gl_Position = mul(float4(vertex_position, 0.0f, 1.0f), _17_view_proj_matrix);
    color = vertex_color;
    tex_coord = vertex_tex_coord;
    gl_Position.y = -gl_Position.y;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    vertex_position = stage_input.vertex_position;
    vertex_color = stage_input.vertex_color;
    vertex_tex_coord = stage_input.vertex_tex_coord;
    vert_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    stage_output.color = color;
    stage_output.tex_coord = tex_coord;
    return stage_output;
}
