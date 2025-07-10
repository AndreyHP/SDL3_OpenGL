#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

noperspective out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


void main()
{
    TexCoords = aTexCoords;

    // Transform vertex to clip space
    vec4 clipPos = projection * view * model * vec4(aPos, 1.0);

    // Convert to NDC (Normalized Device Coordinates)
    vec3 ndc = clipPos.xyz / clipPos.w;

    // Simulate PS1 screen resolution snapping (e.g., 512x240 grid)
    float snapX = 256.0; // Adjust to control horizontal snap strength
    float snapY = 120.0; // Adjust to control vertical snap strength
    ndc.x = round(ndc.x * snapX) / snapX;
    ndc.y = round(ndc.y * snapY) / snapY;

    // Convert back to clip space
    clipPos = vec4(ndc * clipPos.w, clipPos.w);

    gl_Position = clipPos;
}
