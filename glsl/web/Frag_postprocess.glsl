#version 300 es

precision mediump float;


out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;


const float dither[16] = float[16](
    0.0,  2.0,  0.5,  2.5,
    3.0,  1.0,  3.5,  1.5,
    0.75, 2.75,  0.25, 2.25,
    3.75, 1.75,  3.25, 1.25
);


void main() {

vec2 resulution = vec2(1280,720);

// Sample the texture
vec4 color = texture(screenTexture, TexCoords);
//color = floor( color * 32.0) / 32.0; //Simulate 5-bit per channel (15-bit color)

vec2 pixelPos = TexCoords * resulution;
// Calculate Dither
int x = int(mod(pixelPos.x,4.0));
int y = int(mod(pixelPos.y,4.0));

float ditherVal = float(dither[x + y]) / 16.0;


//vec4 DitherColor = color;



//Apply dither
if (color.r < ditherVal){
    color.r = 0.0;
}else{
    color.r =  color.r;
}
if (color.g < ditherVal){
    color.g = 0.0;
}else{
    color.g =  color.g;
}
if (color.b < ditherVal){
    color.b = 0.0;
}else{
    color.b =  color.b;
}


//color = mix(color, DitherColor, 0.3);
//color = DitherColor;


FragColor = color;

}
