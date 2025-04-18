static const float2 _23[3] = { (-1.0f).xx, float2(3.0f, -1.0f), float2(-1.0f, 3.0f) };
static const float2 _45[3] = { 0.0f.xx, float2(2.0f, 0.0f), float2(0.0f, 2.0f) };

static float4 gl_Position;
static int gl_VertexID;
static float2 tex_coord;

struct SPIRV_Cross_Input
{
    uint gl_VertexID : SV_VertexID;
};

struct SPIRV_Cross_Output
{
    float2 tex_coord : TEXCOORD0;
    float4 gl_Position : SV_Position;
};

void vert_main()
{
    gl_Position = float4(_23[gl_VertexID], 0.0f, 1.0f);
    tex_coord = _45[gl_VertexID];
    gl_Position.y = -gl_Position.y;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    gl_VertexID = int(stage_input.gl_VertexID);
    vert_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    stage_output.tex_coord = tex_coord;
    return stage_output;
}
