uniform sampler2D texture;
uniform float time;
uniform float time_delta;
uniform vec2 position; // world position (not screen position) of the tile in pixels

void main()
{
    vec2 uv = gl_TexCoord[0].xy;
    // TODO: Offset uv so it looks like the grass is swaying in the wind.
    // Let's offset u by different amounts along v.
    // We'll use the sine function to make it look like a wave.
    vec4 pixel = texture2D(texture, uv);
    gl_FragColor = gl_Color * pixel;
}