#version 330 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec3 color;

uniform vec2 windowSize;
out vec3 vertexColor;

void main()
{
    vec2 normalizedPosition = position / windowSize * 2.0 - 1.0;
    normalizedPosition.y = -normalizedPosition.y;
    gl_Position = vec4(normalizedPosition, 0.0, 1.0);
    vertexColor = color;
}
