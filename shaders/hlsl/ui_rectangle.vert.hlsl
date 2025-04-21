cbuffer FrameUniformBlock : register(b0)
{
    float _28_app_time : packoffset(c0);
    float _28_game_time : packoffset(c0.y);
    float _28_window_framebuffer_width : packoffset(c0.z);
    float _28_window_framebuffer_height : packoffset(c0.w);
    row_major float4x4 _28_view_proj_matrix : packoffset(c1);
};


static float4 gl_Position;
static float2 vertex_position;
static float4 color;
static float4 vertex_color;
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
    float4 gl_Position : SV_Position;
};

void vert_main()
{
    gl_Position = float4(vertex_position, 0.0f, 1.0f);
    gl_Position.x /= _28_window_framebuffer_width;
    gl_Position.y /= _28_window_framebuffer_height;
    gl_Position.x = (gl_Position.x * 2.0f) - 1.0f;
    gl_Position.y = (gl_Position.y * 2.0f) - 1.0f;
    color = vertex_color;
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
    return stage_output;
}
