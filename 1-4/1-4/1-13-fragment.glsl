#version 330 core

in vec3 vertexColor;
out vec4 finalColor;

void main()
{
    // 정점 셰이더에서 받은 색상을 그대로 출력한다. 알파값 1은 불투명이다.
    finalColor = vec4(vertexColor, 1.0);
}
