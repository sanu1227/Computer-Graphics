#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

// C++에서 만든 이동과 회전 행렬을 받는다.
uniform mat4 transform;
out vec3 vertexColor;

void main()
{
    gl_Position = transform * vec4(position, 1.0);
    vertexColor = color;
}
