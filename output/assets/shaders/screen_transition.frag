uniform sampler2D tex;
uniform float pixel_scale;
uniform float progress;

void main()
{
    // https://ddrkirby.com/articles/shader-based-transitions/shader-based-transitions.html

    const float DIAMOND_SIZE_IN_PIXELS = 20.0;

    vec2 tex_size = textureSize(tex, 0);
    vec2 pixel = floor(vec2(gl_FragCoord.x, tex_size.y - gl_FragCoord.y) / pixel_scale);
    vec2 pixel_fractions = fract(pixel / DIAMOND_SIZE_IN_PIXELS);
    vec2 pixel_distances = abs(pixel_fractions - 0.5);
    vec2 tex_size_in_pixels = tex_size / pixel_scale;
    vec2 tex_size_in_diamonds = tex_size_in_pixels / DIAMOND_SIZE_IN_PIXELS;
    float max_diamond_distance = tex_size_in_diamonds.x + tex_size_in_diamonds.y;
    vec2 diamond = floor(pixel / DIAMOND_SIZE_IN_PIXELS);
    vec2 diamond_distances = diamond / max_diamond_distance;

    float pixel_progress = pixel_distances.x + pixel_distances.y; // <= 1.0
    float diamond_progress = diamond_distances.x + diamond_distances.y; // <= 1.0
    float total_progress = (pixel_progress + diamond_progress) / 2.0; // <= 1.0

    if (progress < 0.0 && 1.0 + progress < total_progress) {
        gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    } else if (progress > 0.0 && total_progress < progress) {
        gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    } else {
        gl_FragColor = texture(tex, gl_FragCoord.xy / tex_size);
	}
}