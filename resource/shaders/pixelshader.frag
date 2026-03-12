#version 310 es

precision mediump float;

layout (location = 0) in vec4 inColor;
 
layout (location = 0) out vec4 outColor;

void main() 
{
    // Draw the entire triangle yellow.
    outColor = vec4 (1.0f, 1.0f, 0.0f, 1.0f);
}
