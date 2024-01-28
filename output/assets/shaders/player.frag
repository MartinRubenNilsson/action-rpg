uniform sampler2D texture;
uniform float time;
uniform float time_delta;
uniform vec2 position; // world position (not screen position) of the tile in pixels

void main()
{
    vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);
    gl_FragColor = gl_Color * pixel;
    gl_FragColor.r = 0.5 + 0.5 * sin(position.x * 0.1);
}