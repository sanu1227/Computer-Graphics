// 실행: ./16-17-check.ps1. 실제 OpenGL 컨텍스트에서 입력, 행렬, 렌더링을 검사한다.
#define main AssignmentMain
#if ASSIGNMENT == 16
#include "../1-16.cpp"
#else
#include "../1-17.cpp"
#endif
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
    int colored = 0;
    for (int i = 0; i < (int)pixels.size(); i += 3)
        if (pixels[i] < 240 || pixels[i + 1] < 240 || pixels[i + 2] < 240)
            colored++;
    assert(colored > 1000);
    std::ofstream image(name + ".ppm", std::ios::binary);
    image << "P6\n800 600\n255\n";
    for (int y = 599; y >= 0; y--)
        image.write((const char*)&pixels[y * 800 * 3], 800 * 3);
    return pixels;
}

#if ASSIGNMENT == 16
glm::vec3 Point(int face, glm::vec3 point)
{
    return glm::vec3(GetPyramidTransform(face) * glm::vec4(point, 1.0f));
}

void Check(GLFWwindow* window, GLint transform, GLint color)
{
    auto press = [window](int key) { KeyCallback(window, key, 0, GLFW_PRESS, 0); };
    ResetObject();
    assert(objectType == 1);
    auto initial = ReadFrame(transform, color, "16-initial");
    press(GLFW_KEY_6);
    UpdateAnimation(0.75f);
    assert(ReadFrame(transform, color, "16-all-mid") != initial);
    for (int face = 0; face < 4; face++)
    {
        // 아래 모서리 양끝이 움직이지 않아야 진짜 경첩 회전이다.
        for (int corner = 0; corner < 2; corner++)
        {
            int index = 36 + face * 3 + corner;
            glm::vec3 p(vertices[index][0], vertices[index][1], vertices[index][2]);
            assert(Near(Point(face, p), p));
        }
    }
    UpdateAnimation(10.0f);
    assert(Near(pyramidAngle, 180.0f));
    for (int face = 0; face < 4; face++)
        assert(Near(Point(face, glm::vec3(0, 0.4f, 0)), glm::vec3(0, -1.2f, 0)));
    assert(Near(GetPyramidTransform(4), glm::mat4(1.0f)));
    ReadFrame(transform, color, "16-all-open");
    press(GLFW_KEY_6);
    UpdateAnimation(0.5f);
    assert(Near(pyramidAngle, 150.0f));
    press(GLFW_KEY_6);
    UpdateAnimation(0.25f);
    assert(Near(pyramidAngle, 165.0f));
    press(GLFW_KEY_6);
    UpdateAnimation(10.0f);
    assert(ReadFrame(transform, color, "16-all-closed") == initial);

    press(GLFW_KEY_7);
    UpdateAnimation(1.5f);
    assert(Near(Point(0, glm::vec3(0, 0.4f, 0)).y, -0.4f));
    for (int face = 1; face < 4; face++)
        assert(Near(GetPyramidTransform(face), glm::mat4(1.0f)));
    ReadFrame(transform, color, "16-first-open");
    for (int face = 1; face < 4; face++)
    {
        UpdateAnimation(1.5f);
        assert(Near(Point(face, glm::vec3(0, 0.4f, 0)).y, -0.4f));
    }
    ReadFrame(transform, color, "16-flat");
    UpdateAnimation(100.0f);
    assert(Near(sequentialAngle, 360.0f));
    press(GLFW_KEY_7);
    UpdateAnimation(0.5f);
    assert(Near(sequentialAngle, 330.0f));
    assert(Near(Point(0, glm::vec3(0, 0.4f, 0)).y, -0.4f));
    UpdateAnimation(10.0f);
    assert(ReadFrame(transform, color, "16-sequence-closed") == initial);

    press(GLFW_KEY_8);
    UpdateAnimation(0.75f);
    for (int face : {0, 2})
        assert(Near(Point(face, glm::vec3(0, 0.4f, 0)), glm::vec3(0, 0.4f, 0)));
    assert(Near(GetPyramidTransform(1), glm::mat4(1.0f)));
    assert(Near(GetPyramidTransform(3), glm::mat4(1.0f)));
    ReadFrame(transform, color, "16-swap-mid");
    UpdateAnimation(10.0f);
    for (int face : {0, 2})
    {
        for (int corner = 0; corner < 3; corner++)
        {
            int source = 36 + face * 3 + corner;
            int target = 36 + (2 - face) * 3 + corner;
            glm::vec3 a(vertices[source][0], vertices[source][1], vertices[source][2]);
            glm::vec3 b(vertices[target][0], vertices[target][1], vertices[target][2]);
            assert(Near(Point(face, a), b));
        }
    }
    ReadFrame(transform, color, "16-swapped");
    press(GLFW_KEY_8);
    UpdateAnimation(10.0f);
    assert(ReadFrame(transform, color, "16-swap-restored") == initial);

    press(GLFW_KEY_9);
    UpdateAnimation(0.75f);
    for (int face = 0; face < 4; face++)
    {
        assert(Near(Point(face, glm::vec3(0, 0.4f, 0)), glm::vec3(0, 0.4f, 0)));
        glm::vec3 bottom = Point(face, pyramidHinges[face]);
        assert(glm::length(glm::vec2(bottom.x, bottom.z)) > 0.4f);
    }
    ReadFrame(transform, color, "16-apex-mid");
    UpdateAnimation(10.0f);
    assert(Near(pyramidAngle, 90.0f));
    ReadFrame(transform, color, "16-apex-open");
    press(GLFW_KEY_9);
    UpdateAnimation(10.0f);
    assert(ReadFrame(transform, color, "16-apex-closed") == initial);
    press(GLFW_KEY_6);
    UpdateAnimation(0.5f);
    press(GLFW_KEY_7);
    assert(Near(pyramidAngle, 0.0f) && Near(sequentialAngle, 0.0f));
    KeyCallback(window, GLFW_KEY_7, 0, GLFW_REPEAT, 0);
    assert(pyramidOpen);
    press(GLFW_KEY_Y);
    UpdateAnimation(0.5f);
    assert(Near(objectAngle, 30.0f));
    press(GLFW_KEY_H);
    ReadFrame(transform, color, "16-depth-off");
    assert(glIsEnabled(GL_DEPTH_TEST) == GL_FALSE);
    press(GLFW_KEY_R);
    assert(ReadFrame(transform, color, "16-reset") == initial);
    press(GLFW_KEY_P);
    press(GLFW_KEY_F);
    UpdateAnimation(2.0f);
    assert(objectType == 0 && Near(frontAngle, 90.0f));
    ReadFrame(transform, color, "16-cube");
    press(GLFW_KEY_9);
    assert(objectType == 1 && pyramidMode == 9);
}
#else
void Check(GLFWwindow* window, GLint transform, GLint color)
{
    auto press = [window](int key, int mods = 0) { KeyCallback(window, key, 0, GLFW_PRESS, mods); };
    ResetObjects();
    auto initial = ReadFrame(transform, color, "17-initial");
    glm::mat4 expectedScene = glm::rotate(glm::mat4(1), glm::radians(30.0f), glm::vec3(1, 0, 0));
    expectedScene = glm::rotate(expectedScene, glm::radians(30.0f), glm::vec3(0, 1, 0));
    assert(Near(GetSceneTransform(), expectedScene));

    for (int selection = 1; selection <= 3; selection++)
    {
        ResetObjects();
        press(GLFW_KEY_0 + selection);
        press(GLFW_KEY_D);
        for (int i = 0; i < 2; i++)
        {
            float x = i == 0 ? -0.85f : 0.85f;
            assert(Near(objects[i].position.x, x - ((selection == 3 || selection == i + 1) ? 0.1f : 0.0f)));
        }
        press(GLFW_KEY_D, GLFW_MOD_SHIFT);
        press(GLFW_KEY_E);
        press(GLFW_KEY_E, GLFW_MOD_SHIFT);
        assert(Near(objects[0].position, glm::vec3(-0.85f, 0, 0)));
        assert(Near(objects[1].position, glm::vec3(0.85f, 0, 0)));
    }

    ResetObjects();
    press(GLFW_KEY_1);
    glm::mat4 right = GetObjectTransform(1);
    press(GLFW_KEY_A);
    assert(Near(objects[0].size, 1.1f) && Near(objects[0].position.x, -0.85f));
    assert(Near(GetObjectTransform(1), right));
    press(GLFW_KEY_A, GLFW_MOD_SHIFT);
    press(GLFW_KEY_B);
    assert(Near(objects[0].size, 1.1f) && Near(objects[0].position.x, -0.935f));
    press(GLFW_KEY_B, GLFW_MOD_SHIFT);
    assert(Near(objects[0].size, 1.0f) && Near(objects[0].position.x, -0.85f));
    for (int i = 0; i < 100; i++) press(GLFW_KEY_B, GLFW_MOD_SHIFT);
    assert(Near(objects[0].size, 0.1f) && Near(objects[0].position.x, -0.085f));
    for (int i = 0; i < 100; i++) press(GLFW_KEY_B);
    assert(Near(objects[0].size, 4.0f) && Near(objects[0].position.x, -3.4f));

    for (int key : {GLFW_KEY_X, GLFW_KEY_Y, GLFW_KEY_R})
    {
        ResetObjects();
        press(GLFW_KEY_1);
        press(key);
        UpdateAnimation(1.5f);
        assert(Near(GetObjectTransform(1), right));
        glm::vec3 axis = key == GLFW_KEY_X ? glm::vec3(1, 0, 0) : glm::vec3(0, 1, 0);
        assert(Near(objects[0].rotation, glm::rotate(glm::mat4(1), glm::radians(90.0f), axis)));
        if (key == GLFW_KEY_R)
            assert(Near(objects[0].position, glm::vec3(0, 0, 0.85f)));
        else
            assert(Near(objects[0].position, glm::vec3(-0.85f, 0, 0)));
        ReadFrame(transform, color, "17-key-" + std::to_string(key));
        press(key, GLFW_MOD_SHIFT);
        UpdateAnimation(1.5f);
        assert(Near(objects[0].rotation, glm::mat4(1)));
        assert(Near(objects[0].position, glm::vec3(-0.85f, 0, 0)));
        press(key, GLFW_MOD_SHIFT);
        glm::mat4 stopped = GetObjectTransform(0);
        UpdateAnimation(1.0f);
        assert(Near(GetObjectTransform(0), stopped));
    }

    for (int key : {GLFW_KEY_T, GLFW_KEY_U})
    {
        ResetObjects();
        press(GLFW_KEY_1);
        press(key);
        UpdateAnimation(1.0f);
        if (key == GLFW_KEY_T)
        {
            assert(Near(objects[0].position, glm::vec3(0)));
            assert(Near(objects[1].position, glm::vec3(0)));
        }
        else
        {
            assert(Near(objects[0].position, glm::vec3(0, 0.7f, 0)));
            assert(Near(objects[1].position, glm::vec3(0, -0.7f, 0)));
        }
        ReadFrame(transform, color, "17-mid-" + std::to_string(key));
        UpdateAnimation(10.0f);
        assert(animationMode == 0);
        assert(Near(objects[0].position, glm::vec3(0.85f, 0, 0)));
        assert(Near(objects[1].position, glm::vec3(-0.85f, 0, 0)));
        ReadFrame(transform, color, "17-swapped-" + std::to_string(key));
    }
    // 이미 이동한 비대칭 위치에서도 t는 원점을 거쳐 정확히 교환한다.
    objects[0].position = glm::vec3(-0.4f, 0.2f, 0.3f);
    objects[1].position = glm::vec3(0.7f, -0.1f, -0.2f);
    press(GLFW_KEY_T);
    UpdateAnimation(1.0f);
    assert(Near(objects[0].position, glm::vec3(0)) && Near(objects[1].position, glm::vec3(0)));
    UpdateAnimation(1.0f);
    assert(Near(objects[0].position, glm::vec3(0.7f, -0.1f, -0.2f)));
    assert(Near(objects[1].position, glm::vec3(-0.4f, 0.2f, 0.3f)));

    ResetObjects();
    press(GLFW_KEY_2);
    press(GLFW_KEY_V);
    UpdateAnimation(0.5f);
    assert(objects[0].size > 1.0f && objects[1].size < 1.0f);
    assert(Near(glm::length(objects[0].position), 0.85f));
    assert(Near(objects[0].position, -objects[1].position));
    assert(!Near(objects[0].rotation, glm::mat4(1)));
    ReadFrame(transform, color, "17-combined");
    press(GLFW_KEY_C);
    UpdateAnimation(0.1f);
    assert(objects[0].shape == 2 && objects[1].shape == 3);
    ReadFrame(transform, color, "17-shapes");
    press(GLFW_KEY_W);
    ReadFrame(transform, color, "17-solid");
    press(GLFW_KEY_V);
    glm::mat4 stopped[2] = {GetObjectTransform(0), GetObjectTransform(1)};
    UpdateAnimation(1.0f);
    assert(Near(GetObjectTransform(0), stopped[0]) && Near(GetObjectTransform(1), stopped[1]));
    press(GLFW_KEY_U);
    UpdateAnimation(0.4f);
    press(GLFW_KEY_A);
    assert(animationMode == 0);
    press(GLFW_KEY_S);
    KeyCallback(window, GLFW_KEY_X, 0, GLFW_REPEAT, 0);
    assert(objects[0].spinX == 0 && objects[1].spinX == 0);
    assert(ReadFrame(transform, color, "17-reset") == initial);
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
    GLFWwindow* window = glfwCreateWindow(800, 600, "16-17 check", nullptr, nullptr);
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
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
#if ASSIGNMENT == 16
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
#else
    MakeVertices();
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);
#endif
    glEnableVertexAttribArray(0);
    FramebufferSizeCallback(window, 800, 600);
    Check(window, transform, color);
    FramebufferSizeCallback(window, 1000, 400);
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    assert(viewport[0] == 300 && viewport[1] == 0 && viewport[2] == 400 && viewport[3] == 400);
    FramebufferSizeCallback(window, 0, 0);
    assert(glGetError() == GL_NO_ERROR);
    KeyCallback(window, GLFW_KEY_Q, 0, GLFW_PRESS, 0);
    assert(glfwWindowShouldClose(window));
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteShader(vs);
    glDeleteShader(fs);
    glDeleteProgram(program);
    glfwDestroyWindow(window);
    glfwTerminate();
    std::cout << "PASS: " << ASSIGNMENT << " input, transforms, animation endpoints, reset, resize, shaders and OpenGL rendering\n";
}
