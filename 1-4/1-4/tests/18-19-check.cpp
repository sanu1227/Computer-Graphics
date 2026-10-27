// 실제 OpenGL 컨텍스트에서 키 입력, 기하 조건, 화면 출력을 함께 검사한다.
#define main AssignmentMain
#if ASSIGNMENT == 18
#include "../1-18.cpp"
#else
#include "../1-19.cpp"
#endif
#undef main
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>

bool Near(float a, float b)
{
    return std::abs(a - b) < 0.0002f;
}

bool Near(glm::vec3 a, glm::vec3 b)
{
    return glm::length(a - b) < 0.0002f;
}

bool Near(glm::mat4 a, glm::mat4 b)
{
    for (int column = 0; column < 4; column++)
        for (int row = 0; row < 4; row++)
            if (!Near(a[column][row], b[column][row]))
                return false;
    return true;
}

std::vector<unsigned char> ReadFrame(GLint transform, GLint color, const std::string& name)
{
    DrawScene(transform, color);
    std::vector<unsigned char> pixels(800 * 600 * 3);
    glReadPixels(0, 0, 800, 600, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    assert(glGetError() == GL_NO_ERROR);
    assert(glIsEnabled(GL_DEPTH_TEST));
    int colored = 0;
    for (int i = 0; i < (int)pixels.size(); i += 3)
        if (pixels[i] < 220 || pixels[i + 1] < 220 || pixels[i + 2] < 220)
            colored++;
    assert(colored > 1000);
    std::ofstream image(name + ".ppm", std::ios::binary);
    image << "P6\n800 600\n255\n";
    for (int y = 599; y >= 0; y--)
        image.write((const char*)&pixels[y * 800 * 3], 800 * 3);
    return pixels;
}

#if ASSIGNMENT == 18
void Check(GLFWwindow* window, GLint transform, GLint color)
{
    auto press = [window](int key, int mods = 0) { KeyCallback(window, key, 0, GLFW_PRESS, mods); };
    ResetObjects();
    auto initial = ReadFrame(transform, color, "18-initial");
    // 행성은 각 궤도 평면 위에 있고 달은 화면과 평행한 평면 위에 있다.
    for (int i = 0; i < 3; i++)
    {
        glm::vec3 planet = GetPlanetPosition(i);
        assert(Near(glm::length(planet), orbitRadii[i]));
        assert(Near(planet.y, std::tan(glm::radians(orbitTilts[i])) * planet.x));
        glm::vec3 moonOffset = GetMoonPosition(i) - planet;
        assert(Near(glm::length(moonOffset), 0.48f));
        assert(Near((GetViewTransform() * glm::vec4(moonOffset, 0)).z, 0.0f));
        // 생성한 원의 모든 정점도 같은 반지름을 갖는다.
        for (int j = orbitFirst; j < (int)vertices.size(); j++)
        {
            glm::vec3 point = glm::vec3(GetPlanetOrbitTransform(i) * glm::vec4(vertices[j], 1));
            assert(Near(glm::length(point), orbitRadii[i]));
        }
    }
    UpdateAnimation(1.0f);
    assert(Near(planetAngles[0], 37) && Near(planetAngles[1], 112) && Near(planetAngles[2], 307));
    assert(ReadFrame(transform, color, "18-orbit") != initial);
    press(GLFW_KEY_1);
    KeyCallback(window, GLFW_KEY_1, 0, GLFW_REPEAT, 0);
    assert(largeOrbits);
    for (int i = 0; i < 3; i++)
    {
        assert(Near(glm::length(GetPlanetPosition(i)), orbitRadii[i] * 1.25f));
        assert(Near(glm::length(GetMoonPosition(i) - GetPlanetPosition(i)), 0.6f));
    }
    press(GLFW_KEY_1);
    assert(!largeOrbits);
    glm::vec3 startColor = GetSunColor();
    press(GLFW_KEY_2);
    UpdateAnimation(0.4f);
    assert(GetSunSize() > 0.55f && !Near(startColor, GetSunColor()));
    press(GLFW_KEY_2);
    float stoppedSize = GetSunSize();
    glm::vec3 stoppedColor = GetSunColor();
    UpdateAnimation(0.3f);
    assert(Near(stoppedSize, GetSunSize()) && Near(stoppedColor, GetSunColor()));
    press(GLFW_KEY_2);
    for (int i = 0; i < 100; i++)
    {
        UpdateAnimation(0.1f);
        assert(GetSunSize() >= 0.4f && GetSunSize() <= 0.7f);
    }
    press(GLFW_KEY_3);
    UpdateAnimation(1.0f);
    assert(Near(moonOrbitAngle, 45.0f));
    glm::mat4 expected = glm::rotate(glm::mat4(1), glm::radians(45.0f), glm::vec3(0, 0, 1));
    expected *= glm::mat4(glm::transpose(glm::mat3(GetViewTransform())));
    for (int i = 0; i < 3; i++)
    {
        glm::vec3 normal = glm::normalize(glm::vec3(GetMoonOrbitTransform(i)[2]));
        assert(Near(normal, glm::vec3(expected[2])));
        assert(Near(glm::length(GetMoonPosition(i) - GetPlanetPosition(i)), 0.48f));
    }
    press(GLFW_KEY_3);
    UpdateAnimation(0.2f);
    assert(Near(moonOrbitAngle, 45.0f));
    ReadFrame(transform, color, "18-animated");

    ResetObjects();
    glm::vec3 planet = GetPlanetPosition(0);
    glm::vec3 moon = GetMoonPosition(0);
    press(GLFW_KEY_D);
    assert(Near(scenePosition, glm::vec3(0.15f, 0, 0)));
    press(GLFW_KEY_W);
    assert(Near(scenePosition, glm::vec3(0.15f, 0.15f, 0)));
    press(GLFW_KEY_KP_ADD);
    glm::vec3 delta(0.15f);
    assert(Near(scenePosition, delta));
    assert(Near(GetPlanetPosition(0), planet + delta));
    assert(Near(GetMoonPosition(0), moon + delta));
    press(GLFW_KEY_A);
    assert(Near(scenePosition, glm::vec3(0, 0.15f, 0.15f)));
    press(GLFW_KEY_S);
    assert(Near(scenePosition, glm::vec3(0, 0, 0.15f)));
    press(GLFW_KEY_MINUS);
    assert(Near(scenePosition, glm::vec3(0)));
    press(GLFW_KEY_EQUAL, GLFW_MOD_SHIFT);
    press(GLFW_KEY_KP_SUBTRACT);
    assert(Near(scenePosition, glm::vec3(0)));
    press(GLFW_KEY_P);
    assert(!perspective);
    auto ortho = ReadFrame(transform, color, "18-orthographic");
    assert(ortho != initial);
    press(GLFW_KEY_P, GLFW_MOD_SHIFT);
    assert(perspective && ReadFrame(transform, color, "18-perspective") == initial);
    press(GLFW_KEY_M, GLFW_MOD_SHIFT);
    assert(wireframe && ReadFrame(transform, color, "18-wireframe") != initial);
    press(GLFW_KEY_M);
    assert(!wireframe && ReadFrame(transform, color, "18-solid") == initial);
    GLboolean mask[4];
    glGetBooleanv(GL_COLOR_WRITEMASK, mask);
    assert(mask[0] && mask[1] && mask[2] && mask[3]);
}
#else
void Check(GLFWwindow* window, GLint transform, GLint color)
{
    auto press = [window](int key, int mods = 0) { KeyCallback(window, key, 0, GLFW_PRESS, mods); };
    ResetObjects();
    auto initial = ReadFrame(transform, color, "19-initial");
    glm::mat4 parts[9];
    GetPartTransforms(parts);
    glm::mat4 originalLeft = GetUpperTransform(0);
    glm::mat4 originalRight = GetUpperTransform(1);
    press(GLFW_KEY_L);
    UpdateAnimation(1.0f);
    assert(swapping && Near(swapProgress, 0.5f));
    glm::vec3 left = glm::vec3(GetUpperTransform(0)[3]);
    glm::vec3 right = glm::vec3(GetUpperTransform(1)[3]);
    assert(glm::length(left - right) > 1.7f);
    ReadFrame(transform, color, "19-swap-mid");
    UpdateAnimation(10.0f);
    assert(!swapping && Near(GetUpperTransform(0), originalRight));
    assert(Near(GetUpperTransform(1), originalLeft));
    ReadFrame(transform, color, "19-swapped");
    press(GLFW_KEY_L);
    UpdateAnimation(2.0f);
    assert(Near(GetUpperTransform(0), originalLeft));
    assert(ReadFrame(transform, color, "19-swap-restored") == initial);

    press(GLFW_KEY_D);
    press(GLFW_KEY_T);
    press(GLFW_KEY_G);
    press(GLFW_KEY_P);
    UpdateAnimation(0.7f);
    GetPartTransforms(parts);
    assert(Near((parts[0] * glm::vec4(0, -0.5f, 0, 1)).y, 0));
    assert(Near((parts[1] * glm::vec4(0, -0.5f, 0, 1)).y, 0));
    assert(Near((parts[2] * glm::vec4(0, -0.5f, 0, 1)).y, GetBaseHeight()));
    assert(Near(centerAngle, 31.5f));
    // 자식 부품의 회전축은 몸체에 붙어 있고 두 방향은 서로 반대다.
    for (int i = 0; i < 2; i++)
    {
        float h = i == 0 ? 0.65f : 0.8f;
        float d = i == 0 ? 1.0f : 0.9f;
        glm::mat4 inverseUpper = glm::inverse(GetUpperTransform(i));
        glm::vec3 gunRoot = glm::vec3(inverseUpper * parts[5 + i] * glm::vec4(0, 0, -0.5f, 1));
        glm::vec3 poleRoot = glm::vec3(inverseUpper * parts[7 + i] * glm::vec4(0, -0.5f, 0, 1));
        assert(Near(gunRoot, glm::vec3(0, h / 2, d / 2)));
        assert(Near(poleRoot, glm::vec3(0, h, 0)));
    }
    glm::mat4 localGun0 = glm::inverse(GetUpperTransform(0)) * parts[5];
    glm::mat4 localGun1 = glm::inverse(GetUpperTransform(1)) * parts[6];
    glm::mat4 localPole0 = glm::inverse(GetUpperTransform(0)) * parts[7];
    glm::mat4 localPole1 = glm::inverse(GetUpperTransform(1)) * parts[8];
    assert(Near(localGun0[2].x, -localGun1[2].x) && std::abs(localGun0[2].x) > 0.1f);
    assert(Near(localPole0[1].z, -localPole1[1].z) && std::abs(localPole0[1].z) > 0.1f);
    auto animated = ReadFrame(transform, color, "19-animated");
    assert(animated != initial);
    for (int key : {GLFW_KEY_D, GLFW_KEY_T, GLFW_KEY_G, GLFW_KEY_P}) press(key);
    UpdateAnimation(0.4f);
    assert(ReadFrame(transform, color, "19-stopped") == animated);

    // 가장자리에서 모든 애니메이션을 진행해도 모든 부품이 바닥 안에 있다.
    for (int key : {GLFW_KEY_LEFT, GLFW_KEY_RIGHT, GLFW_KEY_UP, GLFW_KEY_DOWN})
    {
        press(GLFW_KEY_C);
        for (int i = 0; i < 100; i++) press(key);
        assert(std::abs(tankPosition.x) <= 3.5f && std::abs(tankPosition.z) <= 3.5f);
        for (int animation : {GLFW_KEY_D, GLFW_KEY_T, GLFW_KEY_G, GLFW_KEY_P, GLFW_KEY_L}) press(animation);
        for (int frame = 0; frame < 160; frame++)
        {
            UpdateAnimation(0.1f);
            assert(GetBaseHeight() >= 0.4f && GetBaseHeight() <= 0.8f);
            GetPartTransforms(parts);
            for (const glm::mat4& part : parts)
                for (int corner = 0; corner < 8; corner++)
                {
                    glm::vec3 p = glm::vec3(part * glm::vec4((corner & 1) ? 0.5f : -0.5f,
                        (corner & 2) ? 0.5f : -0.5f, (corner & 4) ? 0.5f : -0.5f, 1));
                    assert(std::abs(p.x) <= floorHalfSize && std::abs(p.z) <= floorHalfSize && p.y >= -0.0001f);
                }
        }
    }

    press(GLFW_KEY_C);
    glm::vec3 startPosition = cameraPosition;
    glm::mat4 startRotation = cameraRotation;
    press(GLFW_KEY_X);
    assert(Near(cameraPosition, startPosition + glm::vec3(0.3f, 0, 0)));
    press(GLFW_KEY_X, GLFW_MOD_SHIFT);
    press(GLFW_KEY_Z);
    assert(Near(cameraPosition, startPosition + glm::vec3(0, 0, 0.3f)));
    press(GLFW_KEY_Z, GLFW_MOD_SHIFT);
    assert(Near(cameraPosition, startPosition) && Near(cameraRotation, startRotation));
    press(GLFW_KEY_Y);
    UpdateAnimation(0.5f);
    assert(Near(cameraPosition, startPosition) && !Near(cameraRotation, startRotation));
    assert(Near(cameraRotation, glm::rotate(startRotation, glm::radians(-17.5f), glm::vec3(0, 1, 0))));
    ReadFrame(transform, color, "19-camera-spin");
    press(GLFW_KEY_Y, GLFW_MOD_SHIFT);
    UpdateAnimation(0.5f);
    assert(Near(cameraRotation, startRotation));
    press(GLFW_KEY_Y, GLFW_MOD_SHIFT);
    UpdateAnimation(0.5f);
    assert(cameraSpin == 0 && Near(cameraRotation, startRotation));
    press(GLFW_KEY_R);
    UpdateAnimation(1.0f);
    assert(Near(glm::length(cameraPosition), glm::length(startPosition)));
    assert(Near(cameraPosition.y, startPosition.y));
    glm::mat4 orbit = glm::rotate(glm::mat4(1), glm::radians(-25.0f), glm::vec3(0, 1, 0));
    assert(Near(cameraPosition, glm::vec3(orbit * glm::vec4(startPosition, 1))));
    assert(Near(cameraRotation, orbit * startRotation));
    ReadFrame(transform, color, "19-camera-orbit");
    press(GLFW_KEY_R, GLFW_MOD_SHIFT);
    UpdateAnimation(1.0f);
    assert(Near(cameraPosition, startPosition) && Near(cameraRotation, startRotation));
    press(GLFW_KEY_R, GLFW_MOD_SHIFT);
    assert(cameraOrbit == 0);
    press(GLFW_KEY_O);
    assert(ReadFrame(transform, color, "19-colors") != initial);
    press(GLFW_KEY_C);
    KeyCallback(window, GLFW_KEY_D, 0, GLFW_REPEAT, 0);
    assert(!animateBase);
    assert(ReadFrame(transform, color, "19-reset") == initial);
}
#endif

int main()
{
    assert(glfwInit());
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    GLFWwindow* window = glfwCreateWindow(800, 600, "18-19 check", nullptr, nullptr);
    assert(window);
    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    assert(glewInit() == GLEW_OK);
    while (glGetError() != GL_NO_ERROR) {}
    std::string prefix = "1-" + std::to_string(ASSIGNMENT);
    GLuint vs = CompileShader(GL_VERTEX_SHADER, ReadShaderFile((prefix + "-vertex.glsl").c_str()).c_str());
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, ReadShaderFile((prefix + "-fragment.glsl").c_str()).c_str());
    assert(vs && fs);
    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    GLint linked;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    assert(linked);
    glUseProgram(program);
    GLint transform = glGetUniformLocation(program, "transform");
    GLint color = glGetUniformLocation(program, "faceColor");
    assert(transform >= 0 && color >= 0);
#if ASSIGNMENT == 18
    lightingLocation = glGetUniformLocation(program, "sphereLighting");
    assert(lightingLocation >= 0);
#endif
    MakeVertices();
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);
    glEnableVertexAttribArray(0);
    FramebufferSizeCallback(window, 800, 600);
    Check(window, transform, color);
    FramebufferSizeCallback(window, 1000, 400);
    assert(Near(aspectRatio, 2.5f));
    FramebufferSizeCallback(window, 0, 0);
    assert(Near(aspectRatio, 2.5f) && glGetError() == GL_NO_ERROR);
    KeyCallback(window, GLFW_KEY_Q, 0, GLFW_PRESS, 0);
    assert(glfwWindowShouldClose(window));
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteShader(vs);
    glDeleteShader(fs);
    glDeleteProgram(program);
    glfwDestroyWindow(window);
    glfwTerminate();
    std::cout << "PASS: " << ASSIGNMENT << " controls, geometry, animation, boundaries, camera/projection, resize and OpenGL rendering\n";
}
