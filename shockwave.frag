uniform sampler2D tex;
uniform vec2 resolution; // in pixels
uniform vec2 center; // in pixels
uniform float force;
uniform float size;
uniform float thickness;

in vec2 tex_coord;
out vec4 frag_color;

vec2 map_target_to_texture(vec2 pos) {
    return vec2(pos.x / resolution.x, pos.y / resolution.y);
}

void main() {
    // Correct aspect ratio for non-square resolutions
    float aspectRatio = resolution.y / resolution.x;
    
    // Correct UVs so that the shockwave is always circular
    vec2 uv = tex_coord;
    vec2 scaledUV = uv * vec2(1.0, aspectRatio); // Stretch the UVs to correct the circle's shape
    
    // Adjust the center based on aspect ratio
    vec2 correctedCenter = map_target_to_texture(center) * vec2(1.0, aspectRatio);
    
    // Calculate distance from the corrected center
    float dist = length(scaledUV - correctedCenter);
    
    // Create a circular mask with smooth edges
    float mask = (1.0 - smoothstep(size - 0.1, size, dist)) *
                 smoothstep(size - thickness - 0.1, size - thickness, dist);
    
    // Calculate displacement
    vec2 disp = normalize(scaledUV - correctedCenter) * force * mask;
    
    // Sample the texture with original UVs
    vec4 originalPixel = texture(tex, uv);

    // Sample the texture with displaced UVs for the shockwave effect
    vec4 displacedPixel = texture(tex, uv - disp);
    
    // Blend between the original pixel and the displaced pixel using the mask
    vec4 finalPixel = mix(originalPixel, displacedPixel, mask);
    
    // Set the final color
    frag_color = finalPixel;
}
