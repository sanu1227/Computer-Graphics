#version 330 core

// 면 하나를 그리기 전에 C++에서 지정하는 색상이다.
uniform vec3 faceColor;
out vec4 finalColor;

void main()
{
    finalColor = vec4(faceColor, 1.0);
}
