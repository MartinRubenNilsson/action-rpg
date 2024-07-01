#version 430

uniform sampler2D tex;
uniform vec2 resolution;
uniform vec2 center;
uniform float intensity;

in vec2 tex_coord;
out vec4 frag_color;

void main()
{
    float aspect = resolution.y / resolution.x;
    vec2 uv = tex_coord;
    vec2 corrected_uv = uv * vec2(1.0, aspect);
    vec2 corrected_center = (center / resolution) * vec2(1.0, aspect);
    float dist = length(corrected_uv - corrected_center);

    const float INNER_EDGE = 0.05;
    const float OUTER_EDGE = 0.3;
    float mask = 1.0 - smoothstep(INNER_EDGE - 0.1, OUTER_EDGE, dist);
    mask *= smoothstep(INNER_EDGE + 0.1, INNER_EDGE, dist);
    mask = clamp(mask, 0.0, 1.0);

    frag_color.rgb = texture(tex, uv).rgb;
    frag_color.rgb *= mix(1.0 - intensity, 1.0, mask);
    frag_color.a = 1.0;
}
