#version 330 core

layout (location = 0) in vec3 position;

// 도형에는 투영 * 좌표계 기울기 * 객체 변환을 받는다.
// 좌표축에는 투영 * 좌표계 기울기만 받는다.
uniform mat4 transform;

void main()
{
    gl_Position = transform * vec4(position, 1.0);
}
