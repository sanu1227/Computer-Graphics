#version 330 core

in vec3 vertexColor;
out vec4 finalColor;

void main()
{
    // 꼭짓점 사이의 색상이 자동으로 보간되어 부드러운 색상 변화가 생긴다.
    finalColor = vec4(vertexColor, 1.0);
}
