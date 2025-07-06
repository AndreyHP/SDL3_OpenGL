#version 330 core
out vec4 FragColor;

noperspective in vec2 TexCoords;

vec4 fog;
float near = 0.1;
float far  = 20.0;

uniform sampler2D texture_diffuse1;
vec2 screenPos = gl_FragCoord.xy;


float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // back to NDC
    return (2.0 * near * far) / (far + near - z * (far - near));
}


void main()
{
    float depth = LinearizeDepth(gl_FragCoord.z) / far;

    vec4 baseColor = texture(texture_diffuse1, TexCoords);
    baseColor.rgb = floor(baseColor.rgb * 16.0) / 16.0; // Reduce to 16 colors
    baseColor = floor( baseColor * 32.0) / 32.0; //Simulate 5-bit per channel (15-bit color)
    fog = vec4(vec3(depth), 1.0);

    FragColor = mix(baseColor, vec4(1.0), fog);
}
