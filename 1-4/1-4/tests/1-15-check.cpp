// 실행: PowerShell에서 ./1-15-check.ps1
#define main AssignmentMain
#include "../1-15.cpp"
#undef main
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <vector>

bool Near(float a, float b)
{
    return std::abs(a - b) < 0.0001f;
}

bool Near(glm::vec3 a, glm::vec3 b)
{
    return glm::length(a - b) < 0.0001f;
}

glm::vec3 TransformPoint(int face, glm::vec3 point)
{
    return glm::vec3(GetFaceTransform(face) * glm::vec4(point, 1.0f));
}

void CheckOriginalFace(int face)
{
    for (int i = face * 6; i < face * 6 + 6; i++)
    {
        glm::vec3 point(vertices[i][0], vertices[i][1], vertices[i][2]);
        assert(Near(TransformPoint(face, point), point));
    }
}

void CheckAnimation(GLFWwindow* window)
{
    auto press = [window](int key) { KeyCallback(window, key, 0, GLFW_PRESS, 0); };
    ResetObject();
    press(GLFW_KEY_Y);
    UpdateAnimation(0.5f);
    assert(Near(objectAngle, 30.0f));
    press(GLFW_KEY_Y);
    UpdateAnimation(1.0f);
    assert(Near(objectAngle, 30.0f));

    press(GLFW_KEY_T);
    UpdateAnimation(0.5f);
    assert(Near(topAngle, 30.0f));
    assert(Near(TransformPoint(4, faceCenters[4]), faceCenters[4]));
    assert(!Near(TransformPoint(4, glm::vec3(0.4f, 0.4f, 0.4f)), glm::vec3(0.4f, 0.4f, 0.4f)));
    press(GLFW_KEY_T);
    UpdateAnimation(1.0f);
    assert(Near(topAngle, 30.0f));

    press(GLFW_KEY_S);
    UpdateAnimation(0.5f);
    assert(Near(sideAngle, 30.0f));
    assert(Near(TransformPoint(1, faceCenters[1]), faceCenters[1]));
    press(GLFW_KEY_S);
    UpdateAnimation(1.0f);
    assert(Near(sideAngle, 30.0f));

    press(GLFW_KEY_F);
    UpdateAnimation(2.0f);
    assert(Near(frontAngle, 90.0f));
    for (float x : { -0.4f, 0.4f })
    {
        glm::vec3 hinge(x, 0.4f, 0.4f);
        assert(Near(TransformPoint(0, hinge), hinge));
        assert(Near(TransformPoint(0, glm::vec3(x, -0.4f, 0.4f)), glm::vec3(x, 0.4f, 1.2f)));
    }
    press(GLFW_KEY_F);
    UpdateAnimation(0.5f);
    assert(Near(frontAngle, 60.0f));
    press(GLFW_KEY_F);
    UpdateAnimation(0.25f);
    assert(Near(frontAngle, 75.0f));
    press(GLFW_KEY_F);
    UpdateAnimation(10.0f);
    assert(Near(frontAngle, 0.0f));

    press(GLFW_KEY_B);
    UpdateAnimation(0.5f);
    assert(Near(backScale, 0.5f));
    assert(Near(TransformPoint(2, faceCenters[2]), faceCenters[2]));
    UpdateAnimation(3.0f);
    assert(Near(backScale, 0.0f));
    press(GLFW_KEY_B);
    UpdateAnimation(3.0f);
    assert(Near(backScale, 1.0f));

    // 3번과 0번을 포함한 모든 이웃 관계에서 공통 모서리와 자리 교체를 검사한다.
    std::srand(15);
    for (int first = 0; first < 4; first++)
    {
        press(GLFW_KEY_1);
        assert(firstFace >= 0 && firstFace < 4 && secondFace == (firstFace + 1) % 4);
        firstFace = first;
        secondFace = (first + 1) % 4;
        UpdateAnimation(0.75f);
        glm::vec3 edge = faceCenters[firstFace] + faceCenters[secondFace];
        for (float y : { -0.4f, 0.4f })
        {
            glm::vec3 point(edge.x, y, edge.z);
            assert(Near(TransformPoint(firstFace, point), point));
            assert(Near(TransformPoint(secondFace, point), point));
        }
        for (int face = 0; face < 6; face++)
            if (face != firstFace && face != secondFace)
                CheckOriginalFace(face);
        UpdateAnimation(6.0f);
        assert(twoFaceMode == 0 && Near(twoFaceAngle, 0.0f));
        for (int face = 0; face < 6; face++)
            CheckOriginalFace(face);

        press(GLFW_KEY_2);
        assert(secondFace == (firstFace + 1) % 4);
        firstFace = first;
        secondFace = (first + 1) % 4;
        UpdateAnimation(2.0f);
        assert(Near(twoFaceAngle, 90.0f));
        assert(Near(TransformPoint(firstFace, faceCenters[firstFace]), faceCenters[secondFace]));
        assert(Near(TransformPoint(secondFace, faceCenters[secondFace]), faceCenters[firstFace]));
        UpdateAnimation(5.0f);
        assert(Near(twoFaceAngle, 90.0f));
    }

    press(GLFW_KEY_4);
    assert(twoFaceMode == 0 && fourFaceMode == 4);
    UpdateAnimation(1.5f);
    for (int face = 0; face < 4; face++)
        assert(Near(TransformPoint(face, faceCenters[face]), faceCenters[(face + 1) % 4]));
    CheckOriginalFace(4);
    CheckOriginalFace(5);
    press(GLFW_KEY_5);
    assert(Near(fourFaceAngle, 0.0f));
    UpdateAnimation(1.5f);
    int destination[6] = { 5, 1, 4, 3, 0, 2 };
    for (int face = 0; face < 6; face++)
        assert(Near(TransformPoint(face, faceCenters[face]), faceCenters[destination[face]]));
    CheckOriginalFace(1);
    CheckOriginalFace(3);

    press(GLFW_KEY_T);
    assert(twoFaceMode == 0 && fourFaceMode == 0 && rotateTop);
    UpdateAnimation(0.5f);
    press(GLFW_KEY_P);
    press(GLFW_KEY_Y);
    float savedTop = topAngle;
    float savedObject = objectAngle;
    UpdateAnimation(0.5f);
    assert(Near(topAngle, savedTop) && !Near(objectAngle, savedObject));
    press(GLFW_KEY_T);  // 사각뿔에서 육면체 면 명령은 무시한다.
    assert(rotateTop);
    KeyCallback(window, GLFW_KEY_P, 0, GLFW_REPEAT, 0);
    assert(objectType == 1);
    press(GLFW_KEY_P);
    UpdateAnimation(0.5f);
    assert(Near(topAngle, savedTop + 30.0f));
    press(GLFW_KEY_H);
    assert(!hiddenSurface);
    press(GLFW_KEY_R);
    assert(objectType == 0 && hiddenSurface && !rotateObject && !rotateTop && !rotateSide);
    assert(!openFront && !hideBack && twoFaceMode == 0 && fourFaceMode == 0);
    assert(Near(objectAngle, 0.0f) && Near(backScale, 1.0f));
    for (int face = 0; face < 6; face++)
        CheckOriginalFace(face);
}

std::vector<unsigned char> ReadFrame(GLint transform, GLint color, const std::string& name)
{
    DrawScene(transform, color);
    assert((glIsEnabled(GL_DEPTH_TEST) == GL_TRUE) == hiddenSurface);
    std::vector<unsigned char> pixels(800 * 600 * 3);
    glReadPixels(0, 0, 800, 600, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    assert(glGetError() == GL_NO_ERROR);
    // 가림이 없는 상태에서 세 축 양끝의 색과 고정된 화면 위치를 확인한다.
    if (!hiddenSurface)
    {
        int samples[6][3] = {
            {200, 240, 0}, {600, 360, 0}, {400, 40, 1},
            {400, 560, 1}, {265, 423, 2}, {535, 177, 2}
        };
        for (auto& sample : samples)
        {
            bool found = false;
            for (int dy = -2; dy <= 2; dy++)
                for (int dx = -2; dx <= 2; dx++)
                {
                    int pixel = ((sample[1] + dy) * 800 + sample[0] + dx) * 3;
                    int channel = sample[2];
                    if (pixels[pixel + channel] > 240 &&
                        pixels[pixel + (channel + 1) % 3] < 10 &&
                        pixels[pixel + (channel + 2) % 3] < 10)
                        found = true;
                }
            assert(found && "Fixed axis position or color mismatch");
        }
    }
    std::ofstream image(name + ".ppm", std::ios::binary);
    image << "P6\n800 600\n255\n";
    for (int y = 599; y >= 0; y--)
        image.write((const char*)&pixels[y * 800 * 3], 800 * 3);
    return pixels;
}

int main()
{
    assert(glfwInit());
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    GLFWwindow* window = glfwCreateWindow(800, 600, "1-15 check", nullptr, nullptr);
    assert(window);
    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    assert(glewInit() == GLEW_OK);
    while (glGetError() != GL_NO_ERROR) {}
    CheckAnimation(window);

    std::string vertex = ReadShaderFile("1-15-vertex.glsl");
    std::string fragment = ReadShaderFile("1-15-fragment.glsl");
    GLuint vs = CompileShader(GL_VERTEX_SHADER, vertex.c_str());
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fragment.c_str());
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
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    FramebufferSizeCallback(window, 800, 600);

    auto initial = ReadFrame(transform, color, "initial");
    int colored = 0;
    for (int i = 0; i < (int)initial.size(); i += 3)
        if (initial[i] < 250 || initial[i + 1] < 250 || initial[i + 2] < 250)
            colored++;
    assert(colored > 5000);
    hiddenSurface = false;
    assert(ReadFrame(transform, color, "depth-off") != initial);
    for (int key : { GLFW_KEY_Y, GLFW_KEY_T, GLFW_KEY_F, GLFW_KEY_S, GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_4, GLFW_KEY_5 })
    {
        ResetObject();
        KeyCallback(window, key, 0, GLFW_PRESS, 0);
        UpdateAnimation(0.75f);
        assert(ReadFrame(transform, color, "key-" + std::to_string(key)) != initial);
        hiddenSurface = false;
        ReadFrame(transform, color, "axes-key-" + std::to_string(key));
    }
    ResetObject();
    objectAngle = 180.0f;  // 뒤쪽을 향하게 돌려 뒷면 축소가 보이는지 검사한다.
    auto back = ReadFrame(transform, color, "back-full");
    KeyCallback(window, GLFW_KEY_B, 0, GLFW_PRESS, 0);
    UpdateAnimation(0.5f);
    assert(ReadFrame(transform, color, "back-half") != back);
    UpdateAnimation(1.0f);
    assert(ReadFrame(transform, color, "back-hidden") != back);
    KeyCallback(window, GLFW_KEY_B, 0, GLFW_PRESS, 0);
    UpdateAnimation(1.0f);
    assert(ReadFrame(transform, color, "back-restored") == back);
    ResetObject();
    KeyCallback(window, GLFW_KEY_P, 0, GLFW_PRESS, 0);
    auto pyramid = ReadFrame(transform, color, "pyramid");
    assert(pyramid != initial);
    KeyCallback(window, GLFW_KEY_Y, 0, GLFW_PRESS, 0);
    UpdateAnimation(0.5f);
    assert(ReadFrame(transform, color, "pyramid-rotated") != pyramid);
    hiddenSurface = false;
    ReadFrame(transform, color, "axes-pyramid-rotated");
    ResetObject();
    assert(ReadFrame(transform, color, "reset") == initial);
    FramebufferSizeCallback(window, 1000, 400);
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    assert(viewport[0] == 300 && viewport[1] == 0 && viewport[2] == 400 && viewport[3] == 400);
    FramebufferSizeCallback(window, 0, 0);
    assert(glGetError() == GL_NO_ERROR);
    KeyCallback(window, GLFW_KEY_Q, 0, GLFW_PRESS, 0);
    assert(glfwWindowShouldClose(window));
    std::cout << "PASS: all keys, pivots, all adjacent face pairs, swap endpoints, limits, mode changes, fixed axes, depth, GPU rendering, reset\n";
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteShader(vs);
    glDeleteShader(fs);
    glDeleteProgram(program);
    glfwDestroyWindow(window);
    glfwTerminate();
}
