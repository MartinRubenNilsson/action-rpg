uniform sampler2D texture;
uniform float time;

void main()
{
    vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);
    //vec2 t = 30.0 * gl_TexCoord[0].xy + 2 * time;
    gl_FragColor = gl_Color * pixel;
    //gl_FragColor.r *= 0.5 + 0.5 * sin(t.x);
    //gl_FragColor.g *= 0.5 + 0.5 * sin(t.y * 2);
    //gl_FragColor.b *= 0.5 + 0.5 * sin(length(t) * 3);
}