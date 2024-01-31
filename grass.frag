uniform sampler2D texture;
uniform float time;
uniform float time_delta;
uniform vec2 position; // world position (not screen position) of the tile in pixels
const float frequency = 5.0;
const float speed = 2.0;
const float amplitude = 0.01;

void main()
{
    vec2 uv = gl_TexCoord[0].xy;
    uv.x += sin(uv.y * frequency + time * speed) * amplitude;
    vec4 pixel = texture2D(texture, uv);
    gl_FragColor = gl_Color * pixel;
}