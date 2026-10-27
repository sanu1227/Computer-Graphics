#version 330 core

layout (location = 0) in vec3 position;

// 도형에는 투영 * 전체 도형 변환 * 각 면의 변환을 받는다.
// 화면 좌표축에는 단위 행렬을 받는다.
uniform mat4 transform;

void main()
{
    gl_Position = transform * vec4(position, 1.0);
}
