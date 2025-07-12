#version 410 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;


const float dither[16] = float[16](
    -4.0,  0.0,  -3.0,   1.0,
     2.0, -2.0,   3.0,  -1.0,
    -3.0,  1.0,  -4.0,   0.0,
     3.0, -1.0,   2.0,  -2.0
);

void main() {

// Sample the texture
vec4 color = texture(screenTexture, TexCoords);
color = floor( color * 32.0) / 32.0; //Simulate 5-bit per channel (15-bit color)

// Calculate Dither
int x = int(gl_FragCoord.x) % 4;
int y = int(gl_FragCoord.y) % 4;

float ditherVal = dither[y * 4 + x];


vec4 DitherColor = color;

// Avarege the brightness
float BR = DitherColor.r / 3.0;
float BG = DitherColor.g / 3.0;
float BB = DitherColor.b / 3.0;
float brightness = (BR + BG + BB);

//Apply dither
if (brightness < ditherVal){
    DitherColor.rgb = color.rgb;
}else{
    DitherColor.rgb =  vec3(1.0);
}


color = mix(color, DitherColor, 0.5);
//color = DitherColor;


FragColor = color;

}
