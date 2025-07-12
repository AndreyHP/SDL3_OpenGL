#version 330 core



out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;


void main() {
// Sample the texture
vec4 color = texture(screenTexture, TexCoords);
color = floor( color * 32.0) / 32.0; //Simulate 5-bit per channel (15-bit color)

FragColor = color;

}
