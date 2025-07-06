#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform sampler2D ditherTex; // Bayer matrix texture
uniform float ditherScale = 4.0; // Add a uniform to control dither size

vec2 screenPos = gl_FragCoord.xy;
float dither = texture2D(ditherTex, screenPos / ditherScale).r;

void main() {
// Sample the texture
vec4 color = texture(screenTexture, TexCoords);
color = floor(( color + dither * 0.3) * 32.0) / 32.0; // Apply dithering and Simulate 5-bit per channel (15-bit color)

FragColor = color;

}
