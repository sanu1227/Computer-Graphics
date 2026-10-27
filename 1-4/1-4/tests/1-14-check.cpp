// 실행: PowerShell에서 ./1-14-check.ps1
// 실제 OpenGL 컨텍스트에서 콜백, 애니메이션, 픽셀 결과를 확인한다.
#define main AssignmentMain
#include "../1-14.cpp"
#undef main
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <cmath>
#include <vector>

const int testWidth = 800;
const int testHeight = 600;

std::vector<unsigned char> ReadFrame(GLint location, const char* imageName)
{
    DrawScene(location);
    assert((glIsEnabled(GL_DEPTH_TEST) == GL_TRUE) == hiddenSurface);
    std::vector<unsigned char> pixels(testWidth * testHeight * 3);
    glReadPixels(0, 0, testWidth, testHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    assert(glGetError() == GL_NO_ERROR);
    // PPM은 별도 이미지 라이브러리 없이 저장할 수 있는 이미지 형식이다.
    std::ofstream image(imageName, std::ios::binary);
    image << "P6\n" << testWidth << " " << testHeight << "\n255\n";
    for (int y = testHeight - 1; y >= 0; y--)
        image.write((const char*)&pixels[y * testWidth * 3], testWidth * 3);
    return pixels;
}

int CountColoredPixels(const std::vector<unsigned char>& pixels)
{
    int count = 0;
    for (int i = 0; i < (int)pixels.size(); i += 3)
    {
        if (pixels[i] < 250 || pixels[i + 1] < 250 || pixels[i + 2] < 250)
            count++;
    }
    return count;
}

// 선의 픽셀 위치는 드라이버에 따라 한 픽셀 차이 날 수 있어 주변도 확인한다.
int FindAxisPixel(const std::vector<unsigned char>& pixels, int x, int y, int channel)
{
    for (int dy = -2; dy <= 2; dy++)
    {
        for (int dx = -2; dx <= 2; dx++)
        {
            int pixel = ((y + dy) * testWidth + x + dx) * 3;
            if (pixels[pixel + channel] > 240 &&
                pixels[pixel + (channel + 1) % 3] < 10 &&
                pixels[pixel + (channel + 2) % 3] < 10)
                return pixel;
        }
    }
    assert(false && "PDF axis direction or color mismatch");
    return -1;
}

int main()
{
    assert(glfwInit());
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    GLFWwindow* window = glfwCreateWindow(testWidth, testHeight, "1-14 check", nullptr, nullptr);
    assert(window);
    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    assert(glewInit() == GLEW_OK);
    // GLEW 초기화 중 생긴 오류를 비운 뒤 과제의 렌더링 오류만 확인한다.
    while (glGetError() != GL_NO_ERROR) {}

    std::string vertex = ReadShaderFile("1-14-vertex.glsl");
    std::string fragment = ReadShaderFile("1-14-fragment.glsl");
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
    GLint location = glGetUniformLocation(program, "transform");
    assert(location >= 0);
    CreateVertices();
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    FramebufferSizeCallback(window, testWidth, testHeight);

    auto cube = ReadFrame(location, "cube-solid.ppm");
    assert(CountColoredPixels(cube) > 20000);
    KeyCallback(window, GLFW_KEY_H, 0, GLFW_PRESS, 0);
    assert(!hiddenSurface);
    auto cubeNoDepth = ReadFrame(location, "cube-no-depth.ppm");
    assert(cube != cubeNoDepth);
    // PDF처럼 x축은 오른쪽 위, y축은 수직, z축은 오른쪽 아래로 향한다.
    int axisPixels[6] = {
        FindAxisPixel(cubeNoDepth, 200, 240, 0), FindAxisPixel(cubeNoDepth, 600, 360, 0),
        FindAxisPixel(cubeNoDepth, 400, 40, 1), FindAxisPixel(cubeNoDepth, 400, 560, 1),
        FindAxisPixel(cubeNoDepth, 265, 423, 2), FindAxisPixel(cubeNoDepth, 535, 177, 2)
    };
    KeyCallback(window, GLFW_KEY_H, 0, GLFW_REPEAT, 0);
    assert(!hiddenSurface);  // 키 반복으로 토글을 되풀이하지 않는다.
    KeyCallback(window, GLFW_KEY_W, 0, GLFW_PRESS, 0);
    assert(wireframe);
    auto wireAll = ReadFrame(location, "cube-wire-all.ppm");
    KeyCallback(window, GLFW_KEY_H, 0, GLFW_PRESS, 0);
    auto wireHidden = ReadFrame(location, "cube-wire-hidden.ppm");
    // 선만 그리므로 면 전체가 뒤쪽 모서리를 가리는 처리는 하지 않는다.
    assert(CountColoredPixels(wireHidden) > CountColoredPixels(wireAll) * 0.95);
    assert(CountColoredPixels(wireAll) < CountColoredPixels(cube) / 4);
    KeyCallback(window, GLFW_KEY_W, 0, GLFW_PRESS, GLFW_MOD_SHIFT);
    assert(!wireframe);

    KeyCallback(window, GLFW_KEY_P, 0, GLFW_PRESS, 0);
    assert(objectType == 1);
    auto pyramid = ReadFrame(location, "pyramid-solid.ppm");
    assert(pyramid != cube && CountColoredPixels(pyramid) > 8000);
    // 사각뿔도 h 설정에 따라 깊이 검사를 켜거나 끈다.
    KeyCallback(window, GLFW_KEY_H, 0, GLFW_PRESS, 0);
    auto pyramidNoDepth = ReadFrame(location, "pyramid-no-depth.ppm");
    assert(pyramid != pyramidNoDepth);
    KeyCallback(window, GLFW_KEY_W, 0, GLFW_PRESS, 0);
    auto pyramidWireAll = ReadFrame(location, "pyramid-wire-all.ppm");
    KeyCallback(window, GLFW_KEY_H, 0, GLFW_PRESS, 0);
    auto pyramidWireHidden = ReadFrame(location, "pyramid-wire-hidden.ppm");
    assert(CountColoredPixels(pyramidWireHidden) > CountColoredPixels(pyramidWireAll) * 0.95);
    KeyCallback(window, GLFW_KEY_W, 0, GLFW_PRESS, GLFW_MOD_SHIFT);
    KeyCallback(window, GLFW_KEY_C, 0, GLFW_PRESS, 0);
    assert(objectType == 0);

    KeyCallback(window, GLFW_KEY_RIGHT, 0, GLFW_PRESS, 0);
    KeyCallback(window, GLFW_KEY_UP, 0, GLFW_REPEAT, 0);
    assert(std::abs(positionX - moveStep) < 0.0001f);
    assert(std::abs(positionY - moveStep) < 0.0001f);
    auto moved = ReadFrame(location, "cube-moved.ppm");
    assert(moved != cube);
    // 축 고정 여부는 도형에 가려지는 영향을 제외하고 확인한다.
    hiddenSurface = false;
    auto movedNoDepth = ReadFrame(location, "cube-moved-no-depth.ppm");
    hiddenSurface = true;
    for (int pixel : axisPixels)
        for (int color = 0; color < 3; color++)
            assert(movedNoDepth[pixel + color] == cubeNoDepth[pixel + color]);
    KeyCallback(window, GLFW_KEY_LEFT, 0, GLFW_PRESS, 0);
    KeyCallback(window, GLFW_KEY_DOWN, 0, GLFW_PRESS, 0);
    assert(std::abs(positionX) < 0.0001f && std::abs(positionY) < 0.0001f);

    // 양/음 방향 모두 새 축을 선택하면 이전 축의 회전이 멈춰야 한다.
    for (int mods : { 0, GLFW_MOD_SHIFT })
    {
        ResetObject();
        int direction = mods == 0 ? 1 : -1;
        KeyCallback(window, GLFW_KEY_X, 0, GLFW_PRESS, mods);
        UpdateAnimation(0.1f);
        float stoppedAngleX = angleX;
        KeyCallback(window, GLFW_KEY_Y, 0, GLFW_PRESS, mods);
        assert(rotationX == 0 && rotationY == direction);
        UpdateAnimation(0.1f);
        assert(angleX == stoppedAngleX);
        assert(std::abs(angleY - (startAngleY + direction * 6.0f)) < 0.0001f);
        float stoppedAngleY = angleY;
        KeyCallback(window, GLFW_KEY_X, 0, GLFW_PRESS, mods);
        assert(rotationY == 0 && rotationX == direction);
        UpdateAnimation(0.1f);
        assert(angleY == stoppedAngleY);
        assert(std::abs(angleX - (stoppedAngleX + direction * 6.0f)) < 0.0001f);
    }
    ResetObject();
    KeyCallback(window, GLFW_KEY_X, 0, GLFW_PRESS, 0);
    UpdateAnimation(0.1f);
    assert(std::abs(angleX - 36.0f) < 0.0001f);
    KeyCallback(window, GLFW_KEY_X, 0, GLFW_PRESS, GLFW_MOD_SHIFT);
    UpdateAnimation(0.1f);
    assert(std::abs(angleX - 30.0f) < 0.0001f);
    KeyCallback(window, GLFW_KEY_S, 0, GLFW_PRESS, 0);
    KeyCallback(window, GLFW_KEY_Y, 0, GLFW_PRESS, 0);
    UpdateAnimation(0.1f);
    assert(std::abs(angleY - 36.0f) < 0.0001f);
    KeyCallback(window, GLFW_KEY_Y, 0, GLFW_PRESS, GLFW_MOD_SHIFT);
    UpdateAnimation(0.1f);
    assert(std::abs(angleY - 30.0f) < 0.0001f);
    UpdateAnimation(0.1f);
    auto rotated = ReadFrame(location, "cube-rotated.ppm");
    assert(rotated != cube);  // 회전은 렌더링 결과에도 반영되어야 한다.
    hiddenSurface = false;
    auto rotatedNoDepth = ReadFrame(location, "cube-rotated-no-depth.ppm");
    hiddenSurface = true;
    for (int pixel : axisPixels)
        for (int color = 0; color < 3; color++)
            assert(rotatedNoDepth[pixel + color] == cubeNoDepth[pixel + color]);
    angleX = 359.0f;
    rotationX = 1;
    UpdateAnimation(0.1f);
    assert(std::abs(angleX - 5.0f) < 0.0001f);
    angleY = 1.0f;
    rotationY = -1;
    UpdateAnimation(0.1f);
    assert(std::abs(angleY - 355.0f) < 0.0001f);

    KeyCallback(window, GLFW_KEY_S, 0, GLFW_PRESS, 0);
    UpdateAnimation(0.1f);
    assert(positionX == 0.0f && positionY == 0.0f);
    assert(angleX == startAngleX && angleY == startAngleY);
    assert(rotationX == 0 && rotationY == 0);
    auto reset = ReadFrame(location, "cube-reset.ppm");
    assert(reset == cube);
    FramebufferSizeCallback(window, 1000, 400);
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    assert(viewport[0] == 300 && viewport[1] == 0);
    assert(viewport[2] == 400 && viewport[3] == 400);
    FramebufferSizeCallback(window, 0, 0);
    assert(glGetError() == GL_NO_ERROR);
    KeyCallback(window, GLFW_KEY_ESCAPE, 0, GLFW_PRESS, 0);
    assert(glfwWindowShouldClose(window));
    std::cout << "PASS: shader, shape, depth, wireframe, movement, fixed axes, rotation, reset\n";
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteShader(vs);
    glDeleteShader(fs);
    glDeleteProgram(program);
    glfwDestroyWindow(window);
    glfwTerminate();
}
