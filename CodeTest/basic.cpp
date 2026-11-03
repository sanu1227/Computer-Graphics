// OpenGL 기본 코드: 이 .cpp 하나만 빌드한다. 시작하면 배경만 보이는 것이 정상이다.
// 준비: x64 / C++17 이상 / GLFW·GLEW·GLM / opengl32.lib glew32.lib glfw3.lib
// 셰이더는 코드에 포함되어 있다. 인터넷이나 외부 GLSL 파일은 필요 없다.
// Esc: 종료. F5: OnReset 호출. 문제별 코드는 OnKey/OnUpdate/OnDraw 등에 작성한다.
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
bool wireframe = false; // true이면 면을 선으로 표시한다.
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

// 매 프레임 화면과 깊이 버퍼를 초기화한다.
void BeginFrame()
{
    glDisable(GL_SCISSOR_TEST);
    glViewport(0,0,framebufferWidth,framebufferHeight);
    glClearColor(background.r,background.g,background.b,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPolygonMode(GL_FRONT_AND_BACK,wireframe ? GL_LINE : GL_FILL);
}

// ===== 여기부터 문제에 필요한 변수와 동작을 작성한다 =====

void OnReset()
{
    // 처음 실행할 때와 F5를 눌렀을 때 복원할 상태.
}

void OnKey(int key, int action, int mods)
{
    // key: 키 종류, action: PRESS/RELEASE/REPEAT, mods: Shift/Ctrl 등.
    // 토글은 action == GLFW_PRESS일 때만 처리한다.
}

void OnMouse(int button, int action)
{
    // 마우스 버튼 입력. 현재 위치는 mouseX, mouseY.
}

void OnCursor()
{
    // 마우스를 움직였을 때 처리할 내용.
}

void OnUpdate(float dt)
{
    // dt: 지난 프레임 이후 흐른 시간(초). 이동·회전 등의 상태를 갱신한다.
}

void OnDraw()
{
    // 투영·카메라 설정과 도형 그리기를 작성한다. 기본 상태에서는 배경만 보인다.
}

// ===== 아래는 공통 실행부 =====

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
int main()
{
    try
    {
        Require(glfwInit() != 0, "GLFW init failed");
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
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
        glfwSwapInterval(1);
        OnReset();
        double previous = glfwGetTime();
        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();
            double now = glfwGetTime();
            float dt = std::min(float(now - previous), 0.05f); // 중단 후 큰 시간 간격 제한.
            previous = now;
            OnUpdate(dt);
            glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
            if (framebufferWidth > 0 && framebufferHeight > 0) // 최소화 중 0 높이 방지.
            {
                BeginFrame();
                OnDraw();
                glfwSwapBuffers(window);
            }
        }
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
        glDeleteProgram(program);
        glfwDestroyWindow(window);
        glfwTerminate();
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
