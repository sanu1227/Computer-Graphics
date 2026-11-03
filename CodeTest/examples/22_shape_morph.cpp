// 시험용 공통 바탕. 이 파일 하나만 빌드한다. C++17 이상, x64.
// 링크: opengl32.lib glew32.lib glfw3.lib
// 1. FEATURE_BLOCK에 필요한 blocks/*.inc 내용을 붙인다.
// 2. HOOK_BLOCK에서 그 기능의 Reset/Key/Mouse/Cursor/Update/Draw를 호출한다.
#include <GL/glew.h>
#include <GL/glfw3.h>
#include <GL/glm/glm.hpp>
#include <GL/glm/gtc/matrix_transform.hpp>
#include <GL/glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

// ===== 공통 바탕: 여러 기능을 붙여도 아래 OpenGL 자원은 하나만 사용한다 =====
// 먼저 기능 블록의 [수정] 값을 바꾼다. 창·투영·그리기 방식을 바꿀 때만 이 영역을 고친다.
GLFWwindow *window = nullptr;
GLuint program = 0, vao = 0, vbo = 0;
GLint transformLocation = -1, colorLocation = -1;
int framebufferWidth = 960, framebufferHeight = 720;
// 2D 논리 해상도다. 화면 크기와 다르다. 바꾸면 블록의 경계·배치 좌표도 맞춰야 한다.
constexpr float CanvasWidth = 800, CanvasHeight = 600;
float mouseX = 0, mouseY = 0; // 2D 논리 좌표. 왼쪽 위 (0,0), 오른쪽 아래 (800,600).
bool use2D = false, perspective = true, wireframe = false;
glm::vec3 background(0.08f, 0.10f, 0.15f);
// eye=카메라 위치, target=바라보는 점, cameraUp=화면 위 방향. 15번 Camera가 이 값을 바꾼다.
glm::vec3 eye(6, 5, 9), target(0), cameraUp(0, 1, 0);
glm::mat4 viewProjection(1);
const glm::vec3 Red(0.95f, 0.25f, 0.25f), Green(0.25f, 0.85f, 0.45f);
const glm::vec3 Blue(0.25f, 0.55f, 1), Yellow(1, 0.8f, 0.2f);

void Require(bool condition, const char *message)
{
    if (!condition)
        throw std::runtime_error(message);
}
bool Near(float a, float b)
{
    return std::abs(a - b) < 0.001f;
}
// 이동: p의 x/y/z가 각 축 이동량이다. 3D에서는 +Y가 위쪽이다.
glm::mat4 T(glm::vec3 p)
{
    return glm::translate(glm::mat4(1), p);
}
// 회전: degrees는 도 단위. {0,1,0}은 Y축. sin/cos에 넣는 라디안과 구분한다.
glm::mat4 R(float degrees, glm::vec3 axis)
{
    return glm::rotate(glm::mat4(1), glm::radians(degrees), axis);
}
// 크기 배율: {2,1,1}이면 X방향만 두 배. Box의 기본 한 변은 1, Sphere의 기본 반지름은 1.
glm::mat4 S(glm::vec3 size)
{
    return glm::scale(glm::mat4(1), size);
}

// position과 size는 중심과 전체 크기이다. halfWidth와 혼동하지 않는다.
bool HitRect(float x, float y, glm::vec2 center, glm::vec2 size)
{
    return std::abs(x - center.x) <= size.x / 2 && std::abs(y - center.y) <= size.y / 2;
}
bool OverlapRect(glm::vec2 a, glm::vec2 sizeA, glm::vec2 b, glm::vec2 sizeB)
{
    return std::abs(a.x - b.x) <= (sizeA.x + sizeB.x) / 2 &&
           std::abs(a.y - b.y) <= (sizeA.y + sizeB.y) / 2;
}
void Primitive(GLenum mode, const std::vector<glm::vec3> &vertices, glm::mat4 model,
               glm::vec3 color)
{
    if (vertices.empty())
        return;
    glUseProgram(program);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(),
                 GL_STREAM_DRAW);
    glm::mat4 transform = viewProjection * model;
    glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(transform));
    glUniform3fv(colorLocation, 1, glm::value_ptr(color));
    glDrawArrays(mode, 0, static_cast<GLsizei>(vertices.size()));
}
void Box(glm::mat4 model, glm::vec3 color)
{
    // model은 위치·회전·크기, color는 RGB(0~1)다. 같은 정점을 model만 바꿔 재사용한다.
    static const glm::vec3 p[] = {{-.5f, -.5f, .5f}, {.5f, -.5f, .5f},   {.5f, .5f, .5f},
                                  {-.5f, .5f, .5f},  {-.5f, -.5f, -.5f}, {.5f, -.5f, -.5f},
                                  {.5f, .5f, -.5f},  {-.5f, .5f, -.5f}};
    static const int faces[6][4] = {{0, 1, 2, 3}, {1, 5, 6, 2}, {5, 4, 7, 6},
                                    {4, 0, 3, 7}, {3, 2, 6, 7}, {4, 5, 1, 0}};
    for (int f = 0; f < 6; ++f)
        Primitive(GL_TRIANGLE_FAN, {p[faces[f][0]], p[faces[f][1]], p[faces[f][2]], p[faces[f][3]]},
                  model, color * (1.0f - 0.08f * f));
}
void Rect(glm::vec2 center, glm::vec2 size, glm::vec3 color, bool filled = true)
{
    Primitive(filled ? GL_TRIANGLE_FAN : GL_LINE_LOOP,
              {{-.5f, -.5f, 0}, {.5f, -.5f, 0}, {.5f, .5f, 0}, {-.5f, .5f, 0}},
              T({center.x, center.y, 0}) * S({size.x, size.y, 1}), color);
}
void Circle(glm::vec2 center, float radius, glm::vec3 color, bool filled = true)
{
    std::vector<glm::vec3> points;
    if (filled)
        points.push_back({center.x, center.y, 0});
    for (int i = 0; i <= (filled ? 48 : 47); ++i)
    {
        float a = glm::radians(360.0f * i / 48);
        points.push_back({center.x + radius * std::cos(a), center.y + radius * std::sin(a), 0});
    }
    Primitive(filled ? GL_TRIANGLE_FAN : GL_LINE_LOOP, points, glm::mat4(1), color);
}
void Sphere(glm::mat4 model, glm::vec3 color)
{
    static const std::vector<glm::vec3> points = [] {
        std::vector<glm::vec3> result;
        auto point = [](int row, int col) {
            float a = glm::radians(-90.0f + 180.0f * row / 12), b = glm::radians(360.0f * col / 24);
            return glm::vec3(std::cos(a) * std::cos(b), std::sin(a), std::cos(a) * std::sin(b));
        };
        for (int row = 0; row < 12; ++row)
            for (int col = 0; col < 24; ++col)
            {
                auto a = point(row, col), b = point(row, col + 1), c = point(row + 1, col + 1),
                     d = point(row + 1, col);
                if (row != 0)
                    result.insert(result.end(), {a, b, c});
                if (row != 11)
                    result.insert(result.end(), {a, c, d});
            }
        return result;
    }();
    Primitive(GL_TRIANGLES, points, model, color); // 원점 중심, 반지름 1.
}
void Ground()
{
    Box(T({0, -.15f, 0}) * S({10, .2f, 10}), {.3f, .35f, .4f});
}
void SetView(int x, int y, int width, int height, glm::vec3 from, glm::vec3 at, glm::vec3 up,
             bool usePerspective)
{
    if (width <= 0 || height <= 0)
        return;
    glViewport(x, y, width, height);
    float aspect = float(width) / height;
    glm::mat4 projection =
        usePerspective ? glm::perspective(glm::radians(45.0f), aspect, .1f, 100.0f)
                       : glm::ortho(-6.0f * aspect, 6.0f * aspect, -6.0f, 6.0f, .1f, 100.0f);
    viewProjection = projection * glm::lookAt(from, at, up);
}
void BeginFrame()
{
    glDisable(GL_SCISSOR_TEST);
    glViewport(0, 0, framebufferWidth, framebufferHeight);
    glClearColor(background.r, background.g, background.b, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
    if (use2D)
    {
        glDisable(GL_DEPTH_TEST);
        viewProjection = glm::ortho(0.0f, CanvasWidth, CanvasHeight, 0.0f, -10.0f, 10.0f);
    }
    else
    {
        glEnable(GL_DEPTH_TEST);
        SetView(0, 0, framebufferWidth, framebufferHeight, eye, target, cameraUp, perspective);
    }
}

// FEATURE_BLOCK_BEGIN
// [복붙 위치] base.cpp의 FEATURE_BLOCK_END 바로 앞에 namespace 전체를 붙인다.
// [연결] 이 파일만 붙이면 실행되지 않는다. 참고서의 호출 등록부를 기존 On 함수 안에 합친다.
namespace ShapeMorph
{
// ===== 1. 시험 조건에 맞게 바꿀 설정값 =====
constexpr float ShapeHeight =
    2.0f; // [수정] 꼭대기 높이. 위쪽 네 정점과 사각뿔 꼭짓점에 같이 적용한다.
constexpr float Duration = 1.0f; // [수정] 모양 변환 시간(초). 2로 바꾸면 두 배 느리게 변한다.

// ===== 2. 실행 중 바뀌는 상태와 자료 =====
// progress=0이면 위쪽 네 정점이 한 꼭짓점에 모여 사각뿔, 1이면 분리되어 육면체다. Top(i)는 진행률에
// 따른 위쪽 정점을 돌려준다.

float progress = 0;
bool cube = false;
// [초기화] 시작/F5에서 호출한다. 설정값을 현재 상태로 복사한다. 조합할 때 use2D는 한 좌표계로
// 통일한다.
void Reset()
{
    use2D = false;
    progress = 0;
    cube = false;
}
// [키 입력] 실행 플래그나 상태를 바꾼다. 두 기능이 같은 키를 쓰면 등록부에서 키를 분배한다.
void Key(int key, int action, int)
{
    if (key == GLFW_KEY_M && action == GLFW_PRESS)
        cube = !cube;
}
// [시간 갱신] dt는 지난 프레임부터 흐른 초다. 초당 속도에는 dt를 곱한다. 그리기 전에 한 번
// 호출한다.
void Update(float dt)
{
    progress += std::clamp((cube ? 1.0f : 0.0f) - progress, -dt / Duration, dt / Duration);
}
glm::vec3 Top(int i)
{
    const glm::vec3 corners[] = {
        {-1, ShapeHeight, 1}, {1, ShapeHeight, 1}, {1, ShapeHeight, -1}, {-1, ShapeHeight, -1}};
    return glm::mix(glm::vec3(0, ShapeHeight, 0), corners[i], progress);
}
// [그리기] 상태를 화면에 표현한다. 같은 물체에 다른 기능을 합치면 이 함수에서 그 기능의 계산 결과를
// 사용한다.
void Draw()
{
    const glm::vec3 bottom[] = {{-1, 0, 1}, {1, 0, 1}, {1, 0, -1}, {-1, 0, -1}};
    for (int i = 0; i < 4; ++i)
    {
        int j = (i + 1) % 4;
        Primitive(GL_TRIANGLE_FAN, {bottom[i], bottom[j], Top(j), Top(i)}, glm::mat4(1),
                  i % 2 ? Green : Blue);
    }
    Primitive(GL_TRIANGLE_FAN, {Top(0), Top(1), Top(2), Top(3)}, glm::mat4(1), Yellow);
    Primitive(GL_TRIANGLE_FAN, {bottom[3], bottom[2], bottom[1], bottom[0]}, glm::mat4(1), Red);
}
// [자동 검사] 기본 설정값 기준의 검사다. 시험 조건으로 설정을 바꾸면 기대값도 바꿔야 한다. 일반
// 실행에서는 호출되지 않는다.
void Check()
{
    Reset();
    Require(glm::length(Top(0) - Top(1)) < .001f, "pyramid apex");
    cube = true;
    Update(1);
    Require(Near(Top(0).x, -1) && Near(Top(1).x, 1), "cube corners");
}
} // namespace ShapeMorph

// FEATURE_BLOCK_END

// HOOK_BLOCK_BEGIN
// 초기 상태 설정. 시작과 F5에서 호출한다.
void OnReset()
{
    ShapeMorph::Reset();
}

// 키 전달. 키가 겹치면 if/else로 한 기능에만 전달한다.
void OnKey(int key,int action,int mods)
{
    ShapeMorph::Key(key,action,mods);
}

// 마우스 버튼 눌림/놓임 전달.
void OnMouse(int button,int action)
{
    // 이 예제에서는 연결할 기능이 없다.
}

// 마우스를 움직일 때 호출한다.
void OnCursor()
{
    // 이 예제에서는 연결할 기능이 없다.
}

// dt는 초 단위. 각 기능을 프레임당 한 번 갱신한다.
void OnUpdate(float dt)
{
    ShapeMorph::Update(dt);
}

// 장면을 그린다. 보조 기능의 예제 물체가 필요 없으면 해당 Draw를 빼면 된다.
void OnDraw()
{
    ShapeMorph::Draw();
}

// --check 실행에서만 사용하는 기본 설정 검사.
void OnCheck()
{
    ShapeMorph::Check();
}
// HOOK_BLOCK_END

void UpdateMouse()
{
    int width, height;
    double x, y;
    glfwGetWindowSize(window, &width, &height);
    glfwGetCursorPos(window, &x, &y);
    if (width > 0 && height > 0)
    {
        mouseX = float(x) * CanvasWidth / width;
        mouseY = float(y) * CanvasHeight / height;
    }
}
GLuint Compile(GLenum type, const char *source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        char log[2048];
        glGetShaderInfoLog(shader, 2048, nullptr, log);
        throw std::runtime_error(log);
    }
    return shader;
}
int main(int argc, char **argv)
{
    bool check = argc > 1 && std::string(argv[1]) == "--check";
    try
    {
        Require(glfwInit() != 0, "GLFW init failed");
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        if (check)
            glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        window = glfwCreateWindow(960, 720, "OpenGL Exam Kit", nullptr, nullptr);
        Require(window != nullptr, "Window creation failed");
        glfwMakeContextCurrent(window);
        glewExperimental = GL_TRUE;
        Require(glewInit() == GLEW_OK, "GLEW init failed");
        while (glGetError() != GL_NO_ERROR)
        {
        }
        const char *vs = R"GLSL(#version 330 core
layout(location=0) in vec3 position;
uniform mat4 transform;
void main(){gl_Position=transform*vec4(position,1.0);}
)GLSL";
        const char *fs = R"GLSL(#version 330 core
uniform vec3 color;
out vec4 outColor;
void main(){outColor=vec4(color,1.0);}
)GLSL";
        GLuint vertex = Compile(GL_VERTEX_SHADER, vs), fragment = Compile(GL_FRAGMENT_SHADER, fs);
        program = glCreateProgram();
        glAttachShader(program, vertex);
        glAttachShader(program, fragment);
        glLinkProgram(program);
        GLint linked = 0;
        glGetProgramiv(program, GL_LINK_STATUS, &linked);
        if (!linked)
        {
            char log[2048];
            glGetProgramInfoLog(program, 2048, nullptr, log);
            throw std::runtime_error(log);
        }
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        transformLocation = glGetUniformLocation(program, "transform");
        colorLocation = glGetUniformLocation(program, "color");
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);
        glEnableVertexAttribArray(0);
        glfwSetKeyCallback(window, [](GLFWwindow *, int key, int, int action, int mods) {
            if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
                glfwSetWindowShouldClose(window, true);
            if (key == GLFW_KEY_F5 && action == GLFW_PRESS)
                OnReset();
            OnKey(key, action, mods);
        });
        glfwSetMouseButtonCallback(window, [](GLFWwindow *, int button, int action, int) {
            UpdateMouse();
            OnMouse(button, action);
        });
        glfwSetCursorPosCallback(window, [](GLFWwindow *, double, double) {
            UpdateMouse();
            OnCursor();
        });
        glfwSwapInterval(check ? 0 : 1);
        OnReset();
        if (check)
        {
            OnCheck();
            OnReset();
        }
        double previous = glfwGetTime();
        int frames = 0;
        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();
            double now = glfwGetTime();
            float dt = check ? 1.0f / 60 : std::min(float(now - previous), 0.05f);
            previous = now;
            OnUpdate(dt);
            glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
            if (framebufferWidth > 0 && framebufferHeight > 0)
            {
                BeginFrame();
                OnDraw();
                if (check)
                    Require(glGetError() == GL_NO_ERROR, "OpenGL error");
                if (check && frames == 7)
                {
                    std::vector<unsigned char> pixels(framebufferWidth * framebufferHeight * 3);
                    glPixelStorei(GL_PACK_ALIGNMENT, 1);
                    glReadPixels(0, 0, framebufferWidth, framebufferHeight, GL_RGB,
                                 GL_UNSIGNED_BYTE, pixels.data());
                    Require(glGetError() == GL_NO_ERROR, "Pixel read error");
                    int changed = 0;
                    for (size_t i = 0; i < pixels.size(); i += 3)
                    {
                        bool different = false;
                        for (int c = 0; c < 3; ++c)
                            different = different ||
                                        std::abs(int(pixels[i + c]) - int(background[c] * 255)) > 5;
                        if (different)
                            ++changed;
                    }
                    Require(changed > 50, "No visible geometry in framebuffer");
                }
                glfwSwapBuffers(window);
            }
            if (check && ++frames >= 8)
                break;
        }
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
        glDeleteProgram(program);
        glfwDestroyWindow(window);
        glfwTerminate();
        if (check)
            std::cout << "PASS: logic checks, 8 hidden OpenGL frames, non-background pixels\n";
        return 0;
    }
    catch (const std::exception &error)
    {
        std::cerr << error.what() << '\n';
        if (window)
            glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }
}
