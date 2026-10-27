#version 330 core
uniform vec3 faceColor;
out vec4 finalColor;
void main()
{
    finalColor = vec4(faceColor, 1.0);
}
