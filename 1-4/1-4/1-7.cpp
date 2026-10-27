#include <GL/glew.h>
#include <GL/glfw3.h>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

const int MAX_SHAPES = 50;
const float MOVE_DISTANCE = 10.0f;

enum ShapeType { POINT, LINE, TRIANGLE, RECTANGLE };
struct Color { float r, g, b; };
struct Shape { ShapeType type; float x, y, width, height; Color color; };

struct Vertex { float x, y, r, g, b; };
Shape shapes[MAX_SHAPES];
int shapeCount = 0;
int selectedShape = -1;
int windowWidth = 800;
int windowHeight = 600;
GLuint shaderProgram;
GLuint vao;
GLuint vbo;

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

void AddShape(ShapeType type)
{
    if (shapeCount >= MAX_SHAPES)
        return;

    Shape& shape = shapes[shapeCount++];
    shape.type = type;
    shape.color = { RandomValue(), RandomValue(), RandomValue() };
    shape.width = 40.0f + static_cast<float>(rand() % 61);
    shape.height = 40.0f + static_cast<float>(rand() % 61);

    if (type == POINT)
        shape.width = shape.height = 10.0f;

    shape.x = shape.width / 2.0f + rand() % (windowWidth - static_cast<int>(shape.width) + 1);
    shape.y = shape.height / 2.0f + rand() % (windowHeight - static_cast<int>(shape.height) + 1);
}

void KeepInside(Shape& shape)
{
    shape.x = std::max(shape.width / 2.0f,
        std::min(shape.x, windowWidth - shape.width / 2.0f));
    shape.y = std::max(shape.height / 2.0f,
        std::min(shape.y, windowHeight - shape.height / 2.0f));
}

void MoveSelected(float x, float y)
{
    if (selectedShape < 0)
        return;

    shapes[selectedShape].x += x;
    shapes[selectedShape].y += y;
    KeepInside(shapes[selectedShape]);
}

void MoveAll(float x, float y)
{
    for (int i = 0; i < shapeCount; ++i)
    {
        shapes[i].x += x;
        shapes[i].y += y;
        KeepInside(shapes[i]);
    }
}

bool IsInside(const Shape& shape, float x, float y)
{
    return x >= shape.x - shape.width / 2.0f &&
        x <= shape.x + shape.width / 2.0f &&
        y >= shape.y - shape.height / 2.0f &&
        y <= shape.y + shape.height / 2.0f;
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int)
{
    if (button != GLFW_MOUSE_BUTTON_LEFT || action != GLFW_PRESS)
        return;

    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    selectedShape = -1;

    for (int i = shapeCount - 1; i >= 0; --i)
    {
        if (IsInside(shapes[i], static_cast<float>(mouseX), static_cast<float>(mouseY)))
        {
            selectedShape = i;
            break;
        }
    }
}

void KeyCallback(GLFWwindow* window, int key, int, int action, int)
{
    if (action != GLFW_PRESS && action != GLFW_REPEAT)
        return;

    if (action == GLFW_PRESS && key == GLFW_KEY_P) AddShape(POINT);
    else if (action == GLFW_PRESS && key == GLFW_KEY_E) AddShape(LINE);
    else if (action == GLFW_PRESS && key == GLFW_KEY_T) AddShape(TRIANGLE);
    else if (action == GLFW_PRESS && key == GLFW_KEY_R) AddShape(RECTANGLE);
    else if (key == GLFW_KEY_W) MoveSelected(0.0f, -MOVE_DISTANCE);
    else if (key == GLFW_KEY_A) MoveSelected(-MOVE_DISTANCE, 0.0f);
    else if (key == GLFW_KEY_S) MoveSelected(0.0f, MOVE_DISTANCE);
    else if (key == GLFW_KEY_D) MoveSelected(MOVE_DISTANCE, 0.0f);
    else if (key == GLFW_KEY_1) MoveAll(0.0f, -MOVE_DISTANCE);
    else if (key == GLFW_KEY_2) MoveAll(-MOVE_DISTANCE, 0.0f);
    else if (key == GLFW_KEY_3) MoveAll(0.0f, MOVE_DISTANCE);
    else if (key == GLFW_KEY_4) MoveAll(MOVE_DISTANCE, 0.0f);
    else if (key == GLFW_KEY_I) MoveSelected(-MOVE_DISTANCE, -MOVE_DISTANCE);
    else if (key == GLFW_KEY_J) MoveSelected(MOVE_DISTANCE, -MOVE_DISTANCE);
    else if (key == GLFW_KEY_K) MoveSelected(-MOVE_DISTANCE, MOVE_DISTANCE);
    else if (key == GLFW_KEY_L) MoveSelected(MOVE_DISTANCE, MOVE_DISTANCE);
    else if (action == GLFW_PRESS && key == GLFW_KEY_C) { shapeCount = 0; selectedShape = -1; }
    else if (key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE)
        glfwSetWindowShouldClose(window, true);
}

void AddVertex(std::vector<Vertex>& vertices, float x, float y, Color color)
{
    vertices.push_back({ x, y, color.r, color.g, color.b });
}

void DrawShape(const Shape& shape)
{
    std::vector<Vertex> vertices;
    float left = shape.x - shape.width / 2.0f;
    float right = shape.x + shape.width / 2.0f;
    float top = shape.y - shape.height / 2.0f;
    float bottom = shape.y + shape.height / 2.0f;

    if (shape.type == LINE)
    {
        AddVertex(vertices, left, top, shape.color);
        AddVertex(vertices, right, bottom, shape.color);
    }
    else if (shape.type == TRIANGLE)
    {
        AddVertex(vertices, shape.x, top, shape.color);
        AddVertex(vertices, left, bottom, shape.color);
        AddVertex(vertices, right, bottom, shape.color);
    }
    else
    {
        AddVertex(vertices, left, top, shape.color);
        AddVertex(vertices, right, top, shape.color);
        AddVertex(vertices, right, bottom, shape.color);
        AddVertex(vertices, left, top, shape.color);
        AddVertex(vertices, right, bottom, shape.color);
        AddVertex(vertices, left, bottom, shape.color);
    }

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
        vertices.data(), GL_DYNAMIC_DRAW);
    glDrawArrays(shape.type == LINE ? GL_LINES :
        shape.type == TRIANGLE ? GL_TRIANGLES : GL_TRIANGLES,
        0, static_cast<GLsizei>(vertices.size()));
}

void DrawScene()
{
    glUseProgram(shaderProgram);
    glUniform2f(glGetUniformLocation(shaderProgram, "windowSize"),
        static_cast<float>(windowWidth), static_cast<float>(windowHeight));

    glBindVertexArray(vao);

    for (int i = 0; i < shapeCount; ++i)
        DrawShape(shapes[i]);

    if (selectedShape >= 0)
    {
        Shape box = shapes[selectedShape];
        box.color = { 0.0f, 0.0f, 0.0f };
        std::vector<Vertex> outline = {
            { box.x - box.width / 2 - 5, box.y - box.height / 2 - 5, 0, 0, 0 },
            { box.x + box.width / 2 + 5, box.y - box.height / 2 - 5, 0, 0, 0 },
            { box.x + box.width / 2 + 5, box.y + box.height / 2 + 5, 0, 0, 0 },
            { box.x - box.width / 2 - 5, box.y + box.height / 2 + 5, 0, 0, 0 }
        };
        glBufferData(GL_ARRAY_BUFFER, outline.size() * sizeof(Vertex),
            outline.data(), GL_DYNAMIC_DRAW);
        glDrawArrays(GL_LINE_LOOP, 0, 4);
    }
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
        windowWidth, windowHeight, "1-7 Shader Shapes", nullptr, nullptr);

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

    glfwSetKeyCallback(window, KeyCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.92f, 0.92f, 0.92f, 1.0f);
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
