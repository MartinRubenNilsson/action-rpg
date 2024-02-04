uniform sampler2D texture;
uniform vec2 resolution;
uniform vec2 center;
uniform float force;
uniform float size;
uniform float thickness;

void main()
{
    // Correct aspect ratio for non-square resolutions
    float aspectRatio = resolution.y / resolution.x;
    
    // Correct UVs so that the shockwave is always circular
    vec2 uv = gl_TexCoord[0].xy;
    vec2 scaledUV = uv * vec2(1.0, aspectRatio); // Stretch the UVs to correct the circle's shape

    // Adjust the center based on aspect ratio
    vec2 correctedCenter = center * vec2(1.0, aspectRatio);
    
    // Calculate distance from the corrected center
    float distance = length(scaledUV - correctedCenter);
    
    // Ensure that 'size' is larger than 'thickness' to avoid negative mask values
    //size = max(size, thickness + 0.1);

    // Create a circular mask with smooth edges, clamped to avoid negative values
    float outerEdge = size;
    float innerEdge = size - thickness;
    float mask = 1.0 - smoothstep(innerEdge - 0.1, outerEdge, distance);
    mask *= smoothstep(innerEdge + 0.1, innerEdge, distance);
    mask = clamp(mask, 0.0, 1.0); // Clamp the mask to valid [0, 1] range

    // Calculate displacement
    vec2 disp = normalize(scaledUV - correctedCenter) * force * mask;
    
    // Apply the displacement to UV coordinates and get the pixel color
    vec4 pixel = texture2D(texture, uv - disp);
    
    // Set the final color, now applying the mask to the alpha channel as well
    gl_FragColor = vec4(pixel.rgb, mask);
}
