#include <GL/glew.h>
#include <GL/glfw3.h>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

struct Color { float r, g, b; };
struct Triangle { float x, y, size, direction; Color color; bool visible; };
struct Vertex { float x, y, r, g, b; };

Triangle triangles[4];
int windowWidth = 800;
int windowHeight = 600;
bool filled = true;
GLuint shaderProgram, vao, vbo;

std::string ReadShaderFile(const char* fileName)
{
    std::ifstream file(fileName);
    std::stringstream source;
    source << file.rdbuf();
    return source.str();
}

GLuint CreateShaderProgram()
{
    std::string vertexSource = ReadShaderFile("vertex.glsl");
    std::string fragmentSource = ReadShaderFile("fragment.glsl");
    const char* vertexSourcePointer = vertexSource.c_str();
    const char* fragmentSourcePointer = fragmentSource.c_str();
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(vertexShader, 1, &vertexSourcePointer, nullptr);
    glCompileShader(vertexShader);
    glShaderSource(fragmentShader, 1, &fragmentSourcePointer, nullptr);
    glCompileShader(fragmentShader);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return program;
}

float RandomValue()
{
    return static_cast<float>(rand()) / RAND_MAX;
}

int GetQuadrant(float x, float y)
{
    bool right = x >= windowWidth / 2.0f;
    bool bottom = y >= windowHeight / 2.0f;

    if (!right && !bottom) return 0;
    if (right && !bottom) return 1;
    if (!right && bottom) return 2;
    return 3;
}

void CreateTriangle(int quadrant, float x, float y)
{
    triangles[quadrant].x = x;
    triangles[quadrant].y = y;
    triangles[quadrant].size = 40.0f + rand() % 61;
    triangles[quadrant].direction = 1.0f;
    triangles[quadrant].color = { RandomValue(), RandomValue(), RandomValue() };
    triangles[quadrant].visible = true;
}

void ResetTriangles()
{
    for (int i = 0; i < 4; ++i)
    {
        float x = (i % 2 == 0 ? windowWidth * 0.25f : windowWidth * 0.75f);
        float y = (i < 2 ? windowHeight * 0.25f : windowHeight * 0.75f);
        CreateTriangle(i, x, y);
    }
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int)
{
    if (action != GLFW_PRESS)
        return;

    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    int quadrant = GetQuadrant(static_cast<float>(mouseX), static_cast<float>(mouseY));

    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        CreateTriangle(quadrant, static_cast<float>(mouseX), static_cast<float>(mouseY));
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT &&
        triangles[quadrant].visible)
    {
        Triangle& triangle = triangles[quadrant];
        triangle.size += triangle.direction * 10.0f;

        if (triangle.size >= 120.0f)
        {
            triangle.size = 120.0f;
            triangle.direction = -1.0f;
        }
        else if (triangle.size <= 30.0f)
        {
            triangle.size = 30.0f;
            triangle.direction = 1.0f;
        }
    }
}

void KeyCallback(GLFWwindow* window, int key, int, int action, int)
{
    if (action != GLFW_PRESS)
        return;

    if (key == GLFW_KEY_A)
        filled = true;
    else if (key == GLFW_KEY_B)
        filled = false;
    else if (key == GLFW_KEY_C)
        ResetTriangles();
    else if (key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE)
        glfwSetWindowShouldClose(window, true);
}

void AddVertex(std::vector<Vertex>& vertices, float x, float y, Color color)
{
    vertices.push_back({ x, y, color.r, color.g, color.b });
}

void DrawTriangle(const Triangle& triangle)
{
    if (!triangle.visible)
        return;

    float halfSize = triangle.size / 2.0f;
    float top = triangle.y - halfSize;
    float left = triangle.x - halfSize;
    float right = triangle.x + halfSize;
    float bottom = triangle.y + halfSize;

    std::vector<Vertex> vertices = {
        { triangle.x, top, triangle.color.r, triangle.color.g, triangle.color.b },
        { left, bottom, triangle.color.r, triangle.color.g, triangle.color.b },
        { right, bottom, triangle.color.r, triangle.color.g, triangle.color.b }
    };

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
        vertices.data(), GL_DYNAMIC_DRAW);

    glDrawArrays(filled ? GL_TRIANGLES : GL_LINE_LOOP, 0, 3);
}

void DrawAxes()
{
    Color axisColor = { 0.25f, 0.25f, 0.25f };
    std::vector<Vertex> vertices = {
        { windowWidth / 2.0f, 0, axisColor.r, axisColor.g, axisColor.b },
        { windowWidth / 2.0f, static_cast<float>(windowHeight), axisColor.r, axisColor.g, axisColor.b },
        { 0, windowHeight / 2.0f, axisColor.r, axisColor.g, axisColor.b },
        { static_cast<float>(windowWidth), windowHeight / 2.0f, axisColor.r, axisColor.g, axisColor.b }
    };

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
        vertices.data(), GL_DYNAMIC_DRAW);
    glDrawArrays(GL_LINES, 0, 4);
}

void DrawScene()
{
    glUseProgram(shaderProgram);
    glUniform2f(glGetUniformLocation(shaderProgram, "windowSize"),
        static_cast<float>(windowWidth), static_cast<float>(windowHeight));

    glBindVertexArray(vao);
    DrawAxes();

    for (int i = 0; i < 4; ++i)
        DrawTriangle(triangles[i]);
}

int main()
{
    srand(static_cast<unsigned int>(time(nullptr)));

    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(
        windowWidth, windowHeight, "1-8 Shader Triangles", nullptr, nullptr);

    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;

    if (glewInit() != GLEW_OK)
        return -1;

    shaderProgram = CreateShaderProgram();
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
        sizeof(Vertex), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
        sizeof(Vertex), reinterpret_cast<void*>(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetKeyCallback(window, KeyCallback);
    ResetTriangles();

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.95f, 0.95f, 0.95f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        DrawScene();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
