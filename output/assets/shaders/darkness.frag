uniform sampler2D tex;
uniform vec2 resolution;
uniform vec2 center;
uniform float intensity;

void main()
{
    float aspect = resolution.y / resolution.x;
    vec2 uv = gl_TexCoord[0].xy;
    vec2 corrected_uv = uv * vec2(1.0, aspect);
    vec2 corrected_center = (center / resolution) * vec2(1.0, aspect);
    float dist = length(corrected_uv - corrected_center);

    const float INNER_EDGE = 0.05;
    const float OUTER_EDGE = 0.3;
    float mask = 1.0 - smoothstep(INNER_EDGE - 0.1, OUTER_EDGE, dist);
    mask *= smoothstep(INNER_EDGE + 0.1, INNER_EDGE, dist);
    mask = clamp(mask, 0.0, 1.0);

    gl_FragColor.rgb = texture(tex, uv).rgb;
    gl_FragColor.rgb *= mix(1.0 - intensity, 1.0, mask);
    gl_FragColor.a = 1.0;
}
