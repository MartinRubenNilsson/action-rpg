uniform sampler2D texture;
uniform vec2 resolution;
uniform float time;
uniform vec2 center;

void main()
{
    vec2 uv = gl_TexCoord[0].xy;
    vec4 pixel = texture2D(texture, uv);
    if (length(gl_FragCoord.xy - center) < 10.0) {
		pixel = vec4(1.0, 0.0, 0.0, 1.0);
	}
    gl_FragColor = gl_Color * pixel;
}