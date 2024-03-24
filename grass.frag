uniform sampler2D texture;
uniform float time;
uniform vec2 position; // world position (not screen position) of the tile in pixels
const float frequency = 5.0;
const float speed = 2.0;
const float amplitude = 0.01;
const float amplitude_scale = 1.5; // Adjust for a stronger effect if needed
const float amplitude_modulation_frequency = 0.5; // Frequency of amplitude change
const float amplitude_modulation_intensity = 0.75; // Degree of change to amplitude
uniform vec2 tile_offset; // Offset from a common origin

void main()
{
   vec2 uv = gl_TexCoord[0].xy;

    // Linearly map uv.y from 0 (bottom) to 1 (top) to get a scaling factor
    float height_factor = pow(1 - uv.y, 2.0);  // Example using a power function

    // Time offset for asynchronous swaying
    float time_offset = tile_offset.y + tile_offset.x * 5.0; 

    float offseted_time = time + time_offset;

    float time_factor = 0.5 + 0.5 * sin(offseted_time * amplitude_modulation_frequency) * amplitude_modulation_intensity; 
    float dynamic_amplitude = amplitude * height_factor * amplitude_scale * time_factor; 

    // Introduce horizontal offset based on tile position
    float horizontal_offset = sin(position.x + tile_offset.x) * 0.2; // Adjust for offset amount


    // Frequency variation
    float frequency_variation = tile_offset.x * 0.5; 
    float adjusted_frequency = frequency + frequency_variation;
    
    // Updated calculation
    uv.x += sin(uv.y * adjusted_frequency + offseted_time * speed + horizontal_offset) * dynamic_amplitude;

    vec4 pixel = texture2D(texture, uv);
    gl_FragColor = gl_Color * pixel;
}