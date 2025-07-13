#version 410 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;


const float dither[16] = float[16](
     0.0,  8.0,   2.0,   10.0,
     12.0, 4.0,   14.0,  6.0,
     3.0,  11.0,  1.0,   9.0,
     15.0, 7.0,   13.0,  5.0
);

void main() {

// Sample the texture
vec4 color = texture(screenTexture, TexCoords);
//color = floor( color * 32.0) / 32.0; //Simulate 5-bit per channel (15-bit color)

// Calculate Dither
int x = int(gl_FragCoord.x) % 4;
int y = int(gl_FragCoord.y) % 4;

float ditherVal = dither[(x * y) / 8];


vec4 DitherColor = color;

// Avarege the brightness
float brightness = (DitherColor.r + DitherColor.g + DitherColor.b) / 3;

//Apply dither
if (brightness < ditherVal){
    DitherColor.rgb = vec3(0.0);
}else{
    DitherColor.rgb =  color.rgb;
}


//color = mix(color, DitherColor, 0.3);
color = DitherColor;


FragColor = color;

}
