#version 330 core
uniform vec3 faceColor;
uniform bool sphereLighting;
in vec3 sphereNormal;
out vec4 finalColor;
void main()
{
    float brightness = 1.0;
    if (sphereLighting)
        brightness = 0.45 + 0.55 * max(dot(normalize(sphereNormal), normalize(vec3(-1.0, 2.0, 3.0))), 0.0);
    finalColor = vec4(faceColor * brightness, 1.0);
}
