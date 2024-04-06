// https://www.rastergrid.com/blog/2010/09/efficient-gaussian-blur-with-linear-sampling/

uniform sampler2D tex;
uniform vec2 tex_size;
const float offsets[3] = { 0.0, 1.3846153846, 3.2307692308 };
const float weights[3] = { 0.2270270270, 0.3162162162, 0.0702702703 };
 
void main()
{
    gl_FragColor = texture2D(tex, gl_FragCoord.xy / tex_size) * weights[0];
    for (int i = 1; i < 3; ++i) {
        gl_FragColor += texture2D(tex, (gl_FragCoord.xy + vec2(0.0, offsets[i])) / tex_size) * weights[i];
        gl_FragColor += texture2D(tex, (gl_FragCoord.xy - vec2(0.0, offsets[i])) / tex_size) * weights[i];
    }
}