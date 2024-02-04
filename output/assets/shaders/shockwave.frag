uniform sampler2D texture;
uniform vec2 resolution;
uniform vec2 center;
uniform float force;
uniform float size;
uniform float thickness;

void main() {
    // Correct aspect ratio for non-square resolutions
    float aspectRatio = resolution.y / resolution.x;
    
    // Correct UVs so that the shockwave is always circular
    vec2 uv = gl_TexCoord[0].xy;
    vec2 scaledUV = uv * vec2(1.0, aspectRatio); // Stretch the UVs to correct the circle's shape
    
    // Adjust the center based on aspect ratio
    vec2 correctedCenter = center * vec2(1.0, aspectRatio);
    
    // Calculate distance from the corrected center
    float distance = length(scaledUV - correctedCenter);
    
    // Create a circular mask with smooth edges
    float mask = (1.0 - smoothstep(size - 0.1, size, distance)) *
                 smoothstep(size - thickness - 0.1, size - thickness, distance);
    
    // Calculate displacement
    vec2 disp = normalize(scaledUV - correctedCenter) * force * mask;
    
    // Sample the texture with original UVs
    vec4 originalPixel = texture2D(texture, uv);

    // Sample the texture with displaced UVs for the shockwave effect
    vec4 displacedPixel = texture2D(texture, uv - disp);
    
    // Blend between the original pixel and the displaced pixel using the mask
    vec4 finalPixel = mix(originalPixel, displacedPixel, mask);
    
    // Set the final color
    gl_FragColor = finalPixel;
    // debug 
    gl_FragColor = vec4(mask, mask, mask, 1.0);
}
