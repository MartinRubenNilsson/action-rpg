uniform sampler2D tex;
uniform float time;
uniform vec2 position; // world position (not screen position) of the tile in pixels

in vec4 color;
in vec2 tex_coord;
out vec4 frag_color;

void main()
{
    const float frequency = 5.0;
    const float speed = 4.0;
    const float amplitude = 0.015; // Adjust for a stronger effect if needed
    const float amplitude_modulation_frequency = 1.2; // Frequency of amplitude change
    const float amplitude_modulation_intensity = 0.75; // Degree of change to amplitude

    vec2 uv = tex_coord;

    // Linearly map uv.y from 0 (bottom) to 1 (top) to get a scaling factor
    float height_factor = pow(uv.y, 2.0);  // Example using a power function

    // Time offset for asynchronous swaying
    float time_offset = position.x * 0.03 + position.y * 0.01; 
    float offsetted_time = time + time_offset;

    float time_factor = 0.5 + 0.5 * sin(offsetted_time * amplitude_modulation_frequency) * amplitude_modulation_intensity; 
    float dynamic_amplitude = amplitude * height_factor * time_factor; 

    // Frequency variation
    float frequency_variation = sin(offsetted_time) * 0.5; 
    float adjusted_frequency = frequency + frequency_variation;
    
    // Updated calculation
    uv.x += sin(uv.y * adjusted_frequency + offsetted_time * speed) * dynamic_amplitude;

    vec4 pixel = texture(tex, uv);
    frag_color = color * pixel;
}