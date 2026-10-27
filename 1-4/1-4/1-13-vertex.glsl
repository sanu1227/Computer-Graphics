#version 330 core

// C++에서 보낸 위치와 색상.
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

// C++에서 만든 회전 행렬.
uniform mat4 transform;
out vec3 vertexColor;

void main()
{
    gl_Position = transform * vec4(position, 1.0);
    vertexColor = color;
}
