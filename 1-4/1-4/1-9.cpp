#include <GL/glew.h>
#include <GL/glfw3.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

const int TRIANGLE_COUNT = 4;
const float MIN_TRIANGLE_SIZE = 30.0f;
const float MAX_TRIANGLE_SIZE = 120.0f;
const float SIZE_CHANGE = 10.0f;
const float PI = 3.14159265f;
const int MAX_PATH_POINTS = 256;

struct Color { float red, green, blue; };
struct PathPoint { float x, y; };

struct Triangle
{
    float x, y, size, sizeDirection, rotation;
    Color color;
    bool visible;
    float speed, directionX, directionY;
    bool movingHorizontally;
    int zigzagPhase;
    float stepRemaining, stepLength;
    float spiralCenterX, spiralCenterY;
    float spiralAngle, spiralRadius, spiralRadiusDirection;
    float angularSpeed, radialSpeed;
    PathPoint path[MAX_PATH_POINTS];
    int pathCount;
};

struct Vertex { float x, y, red, green, blue; };

Triangle triangles[TRIANGLE_COUNT];
int windowWidth = 800;
int windowHeight = 600;
int movementMode = 1;
bool drawFilled = true;
GLuint shaderProgram = 0, vao = 0, vbo = 0;

std::string ReadShaderFile(const char* fileName)
{
    std::ifstream file(fileName);
    std::stringstream source;
    source << file.rdbuf();
    return source.str();
}

GLuint CompileShader(GLenum shaderType, const std::string& source)
{
    const char* sourcePointer = source.c_str();
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &sourcePointer, nullptr);
    glCompileShader(shader);

    GLint success = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char log[1024] = {};
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        std::cerr << "Shader compilation failed: " << log << std::endl;
    }
    return shader;
}

GLuint CreateShaderProgram()
{
    GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, ReadShaderFile("vertex.glsl"));
    GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, ReadShaderFile("fragment.glsl"));
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    GLint success = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        char log[1024] = {};
        glGetProgramInfoLog(program, sizeof(log), nullptr, log);
        std::cerr << "Shader linking failed: " << log << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return program;
}

float RandomFloat(float minimum, float maximum)
{
    return minimum + static_cast<float>(rand()) / RAND_MAX * (maximum - minimum);
}

float RandomDirection() { return rand() % 2 == 0 ? -1.0f : 1.0f; }

Color MakeRandomColor()
{
    return { RandomFloat(0.15f, 1.0f), RandomFloat(0.15f, 1.0f), RandomFloat(0.15f, 1.0f) };
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

void ClearPath(Triangle& triangle)
{
    triangle.pathCount = 0;
    triangle.path[triangle.pathCount++] = { triangle.x, triangle.y };
}

void ResetTrianglePath(Triangle& triangle) { ClearPath(triangle); }

void SetRandomTriangle(Triangle& triangle, float x, float y)
{
    triangle.x = x;
    triangle.y = y;
    triangle.size = RandomFloat(50.0f, 90.0f);
    triangle.sizeDirection = 1.0f;
    triangle.rotation = 0.0f;
    triangle.color = MakeRandomColor();
    triangle.visible = true;
    triangle.speed = RandomFloat(70.0f, 150.0f);
    triangle.directionX = RandomDirection();
    triangle.directionY = RandomDirection();
    // 2번은 가로 이동부터, 3번은 세로 이동부터 시작한다.
    triangle.movingHorizontally = movementMode != 3;
    triangle.zigzagPhase = movementMode == 2 ? 0 : 1;
    triangle.stepLength = RandomFloat(35.0f, 70.0f);
    triangle.stepRemaining = triangle.stepLength;
    triangle.spiralCenterX = x;
    triangle.spiralCenterY = y;
    triangle.spiralAngle = RandomFloat(0.0f, 2.0f * PI);
    triangle.spiralRadius = 0.0f;
    triangle.spiralRadiusDirection = 1.0f;
    // 회전 성분을 크게 두어 원형 궤도가 먼저 보이게 한다.
    triangle.angularSpeed = RandomDirection() * RandomFloat(3.5f, 5.0f);
    triangle.radialSpeed = RandomFloat(10.0f, 18.0f);
    ClearPath(triangle);
}

void CreateTriangleInsideQuadrant(int quadrant)
{
    float halfWidth = windowWidth / 2.0f;
    float halfHeight = windowHeight / 2.0f;
    float margin = MAX_TRIANGLE_SIZE / 2.0f;
    float minX = quadrant % 2 == 0 ? margin : halfWidth + margin;
    float maxX = quadrant % 2 == 0 ? halfWidth - margin : windowWidth - margin;
    float minY = quadrant < 2 ? margin : halfHeight + margin;
    float maxY = quadrant < 2 ? halfHeight - margin : windowHeight - margin;
    SetRandomTriangle(triangles[quadrant], RandomFloat(minX, maxX), RandomFloat(minY, maxY));
}

void ResetTriangles()
{
    for (int i = 0; i < TRIANGLE_COUNT; ++i)
        CreateTriangleInsideQuadrant(i);
}

void KeepTriangleInsideWindow(Triangle& triangle)
{
    float halfSize = triangle.size / 2.0f;
    if (triangle.x < halfSize) { triangle.x = halfSize; triangle.directionX = 1.0f; }
    if (triangle.x > windowWidth - halfSize) { triangle.x = windowWidth - halfSize; triangle.directionX = -1.0f; }
    if (triangle.y < halfSize) { triangle.y = halfSize; triangle.directionY = 1.0f; }
    if (triangle.y > windowHeight - halfSize) { triangle.y = windowHeight - halfSize; triangle.directionY = -1.0f; }
}

void MoveBounce(Triangle& triangle, float deltaTime)
{
    triangle.x += triangle.directionX * triangle.speed * deltaTime;
    triangle.y += triangle.directionY * triangle.speed * deltaTime;
    KeepTriangleInsideWindow(triangle);
}

void MoveHorizontalZigzag(Triangle& triangle, float deltaTime)
{
    float distance = triangle.speed * deltaTime;
    if (triangle.zigzagPhase == 0)
    {
        triangle.x += triangle.directionX * distance;
        if (triangle.x <= triangle.size / 2.0f || triangle.x >= windowWidth - triangle.size / 2.0f)
        {
            KeepTriangleInsideWindow(triangle);
            triangle.zigzagPhase = 1;
            triangle.stepRemaining = triangle.stepLength;
        }
    }
    else
    {
        // ㄹ의 세로 연결 구간
        float moveDistance = std::min(distance, triangle.stepRemaining);
        triangle.y += triangle.directionY * moveDistance;
        triangle.stepRemaining -= moveDistance;
        if (triangle.stepRemaining <= 0.0f)
        {
            triangle.stepRemaining = triangle.stepLength;
            triangle.zigzagPhase = 0;
        }
        KeepTriangleInsideWindow(triangle);
    }
}

void MoveVerticalZigzag(Triangle& triangle, float deltaTime)
{
    float verticalDistance = triangle.speed * deltaTime;
    float horizontalDistance = verticalDistance * 0.65f;
    float halfSize = triangle.size / 2.0f;

    // 삼각 스파이크: 위·아래 끝점을 대각선으로 번갈아 연결한다.
    triangle.x += triangle.directionX * horizontalDistance;
    triangle.y += triangle.directionY * verticalDistance;

    if (triangle.y <= halfSize || triangle.y >= windowHeight - halfSize)
    {
        if (triangle.y <= halfSize)
        {
            triangle.y = halfSize;
            triangle.directionY = 1.0f;
        }
        else
        {
            triangle.y = windowHeight - halfSize;
            triangle.directionY = -1.0f;
        }
    }

    if (triangle.x <= halfSize || triangle.x >= windowWidth - halfSize)
    {
        if (triangle.x <= halfSize)
        {
            triangle.x = halfSize;
            triangle.directionX = 1.0f;
        }
        else
        {
            triangle.x = windowWidth - halfSize;
            triangle.directionX = -1.0f;
        }
    }
}

void MoveSpiral(Triangle& triangle, float deltaTime)
{
    triangle.spiralAngle += triangle.angularSpeed * deltaTime;
    triangle.spiralRadius += triangle.spiralRadiusDirection * triangle.radialSpeed * deltaTime;
    if (triangle.spiralRadius < 0.0f)
    {
        triangle.spiralRadius = 0.0f;
        triangle.spiralRadiusDirection = 1.0f;
    }
    triangle.x = triangle.spiralCenterX + cosf(triangle.spiralAngle) * triangle.spiralRadius;
    triangle.y = triangle.spiralCenterY + sinf(triangle.spiralAngle) * triangle.spiralRadius;
    if (triangle.x < triangle.size / 2.0f || triangle.x > windowWidth - triangle.size / 2.0f ||
        triangle.y < triangle.size / 2.0f || triangle.y > windowHeight - triangle.size / 2.0f)
    {
        triangle.spiralRadiusDirection = -1.0f;
        triangle.spiralRadius = std::max(0.0f, triangle.spiralRadius - triangle.radialSpeed * deltaTime * 2.0f);
        triangle.x = triangle.spiralCenterX + cosf(triangle.spiralAngle) * triangle.spiralRadius;
        triangle.y = triangle.spiralCenterY + sinf(triangle.spiralAngle) * triangle.spiralRadius;
        KeepTriangleInsideWindow(triangle);
    }
}

void UpdateMovementDirection(Triangle& triangle, float previousX, float previousY)
{
    float dx = triangle.x - previousX;
    float dy = triangle.y - previousY;
    float length = sqrtf(dx * dx + dy * dy);
    if (length > 0.001f)
    {
        triangle.directionX = dx / length;
        triangle.directionY = dy / length;
    }
}

float GetMovementAngle(const Triangle& triangle)
{
    return atan2f(triangle.directionY, triangle.directionX) + PI / 2.0f;
}

void UpdateTriangleRotation(Triangle& triangle, float previousX, float previousY)
{
    float dx = triangle.x - previousX;
    float dy = triangle.y - previousY;
    if (fabsf(dx) > 0.001f || fabsf(dy) > 0.001f)
        triangle.rotation = atan2f(dy, dx) + PI / 2.0f;
}

void RecordPathPoint(Triangle& triangle)
{
    if (triangle.pathCount > 0)
    {
        PathPoint last = triangle.path[triangle.pathCount - 1];
        float dx = triangle.x - last.x;
        float dy = triangle.y - last.y;
        if (dx * dx + dy * dy < 9.0f) return;
    }
    if (triangle.pathCount == MAX_PATH_POINTS)
    {
        for (int i = 1; i < MAX_PATH_POINTS; ++i)
            triangle.path[i - 1] = triangle.path[i];
        --triangle.pathCount;
    }
    triangle.path[triangle.pathCount++] = { triangle.x, triangle.y };
}

void UpdateTriangle(Triangle& triangle, float deltaTime)
{
    if (!triangle.visible) return;
    float previousX = triangle.x;
    float previousY = triangle.y;
    if (movementMode == 1) MoveBounce(triangle, deltaTime);
    else if (movementMode == 2) MoveHorizontalZigzag(triangle, deltaTime);
    else if (movementMode == 3) MoveVerticalZigzag(triangle, deltaTime);
    else MoveSpiral(triangle, deltaTime);
    // 이동 함수가 경계 충돌 후 갱신한 방향을 유지한다.
    // 실제 위치 변화는 회전각 계산에만 사용한다.
    UpdateTriangleRotation(triangle, previousX, previousY);
    RecordPathPoint(triangle);
}

void UpdateTriangles(float deltaTime)
{
    for (Triangle& triangle : triangles) UpdateTriangle(triangle, deltaTime);
}

void ChangeMovementMode(int newMode)
{
    movementMode = newMode;
    for (Triangle& triangle : triangles)
    {
        // 2번: 가로 직선 후 세로 칸 이동
        // 3번: 세로 직선 후 가로 칸 이동
        triangle.movingHorizontally = movementMode == 2;
        triangle.zigzagPhase = movementMode == 2 ? 0 : 1;
        triangle.stepRemaining = triangle.stepLength;
        if (movementMode == 4)
        {
            triangle.spiralCenterX = triangle.x;
            triangle.spiralCenterY = triangle.y;
            triangle.spiralRadius = 0.0f;
            triangle.spiralRadiusDirection = 1.0f;
        }
    }
}

void ChangeTriangleSize(int quadrant)
{
    Triangle& triangle = triangles[quadrant];
    triangle.size += triangle.sizeDirection * SIZE_CHANGE;
    if (triangle.size >= MAX_TRIANGLE_SIZE) { triangle.size = MAX_TRIANGLE_SIZE; triangle.sizeDirection = -1.0f; }
    if (triangle.size <= MIN_TRIANGLE_SIZE) { triangle.size = MIN_TRIANGLE_SIZE; triangle.sizeDirection = 1.0f; }
    KeepTriangleInsideWindow(triangle);
    ClearPath(triangle);
}

void AddVertex(std::vector<Vertex>& vertices, float x, float y, Color color)
{
    vertices.push_back({ x, y, color.red, color.green, color.blue });
}

void UploadVertices(const std::vector<Vertex>& vertices)
{
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_DYNAMIC_DRAW);
}

void DrawAxes()
{
    Color color = { 0.25f, 0.25f, 0.25f };
    std::vector<Vertex> vertices = {
        { windowWidth / 2.0f, 0, color.red, color.green, color.blue },
        { windowWidth / 2.0f, static_cast<float>(windowHeight), color.red, color.green, color.blue },
        { 0, windowHeight / 2.0f, color.red, color.green, color.blue },
        { static_cast<float>(windowWidth), windowHeight / 2.0f, color.red, color.green, color.blue }
    };
    UploadVertices(vertices);
    glDrawArrays(GL_LINES, 0, 4);
}

void DrawPath(const Triangle& triangle)
{
    if (!triangle.visible || triangle.pathCount < 2) return;
    Color pathColor = { triangle.color.red * 0.55f, triangle.color.green * 0.55f, triangle.color.blue * 0.55f };
    std::vector<Vertex> vertices;
    for (int i = 0; i < triangle.pathCount; ++i)
        AddVertex(vertices, triangle.path[i].x, triangle.path[i].y, pathColor);
    UploadVertices(vertices);
    glLineWidth(2.0f);
    glDrawArrays(GL_LINE_STRIP, 0, static_cast<GLsizei>(vertices.size()));
    glLineWidth(1.0f);
}

void DrawTriangle(const Triangle& triangle)
{
    if (!triangle.visible) return;
    float half = triangle.size / 2.0f;
    float cosAngle = cosf(triangle.rotation);
    float sinAngle = sinf(triangle.rotation);
    auto rotate = [&](float localX, float localY) {
        return PathPoint{ triangle.x + localX * cosAngle - localY * sinAngle,
                          triangle.y + localX * sinAngle + localY * cosAngle };
    };
    PathPoint top = rotate(0, -half);
    PathPoint left = rotate(-half, half);
    PathPoint right = rotate(half, half);
    std::vector<Vertex> vertices;
    AddVertex(vertices, top.x, top.y, triangle.color);
    AddVertex(vertices, left.x, left.y, triangle.color);
    AddVertex(vertices, right.x, right.y, triangle.color);
    UploadVertices(vertices);
    glDrawArrays(drawFilled ? GL_TRIANGLES : GL_LINE_LOOP, 0, 3);
}

void DrawScene()
{
    glUseProgram(shaderProgram);
    GLint windowSize = glGetUniformLocation(shaderProgram, "windowSize");
    glUniform2f(windowSize, static_cast<float>(windowWidth), static_cast<float>(windowHeight));
    glBindVertexArray(vao);
    DrawAxes();
    for (const Triangle& triangle : triangles) DrawPath(triangle);
    for (const Triangle& triangle : triangles) DrawTriangle(triangle);
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int)
{
    if (action != GLFW_PRESS) return;
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    int quadrant = GetQuadrant(static_cast<float>(mouseX), static_cast<float>(mouseY));
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        SetRandomTriangle(triangles[quadrant], static_cast<float>(mouseX), static_cast<float>(mouseY));
        KeepTriangleInsideWindow(triangles[quadrant]);
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT)
        ChangeTriangleSize(quadrant);
}

void KeyCallback(GLFWwindow* window, int key, int, int action, int)
{
    if (action != GLFW_PRESS) return;
    if (key >= GLFW_KEY_1 && key <= GLFW_KEY_4) ChangeMovementMode(key - GLFW_KEY_0);
    else if (key == GLFW_KEY_A) drawFilled = true;
    else if (key == GLFW_KEY_B) drawFilled = false;
    else if (key == GLFW_KEY_C) ResetTriangles();
    else if (key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void WindowSizeCallback(GLFWwindow*, int width, int height)
{
    windowWidth = width;
    windowHeight = height;
    for (Triangle& triangle : triangles) KeepTriangleInsideWindow(triangle);
}

void FramebufferSizeCallback(GLFWwindow*, int width, int height) { glViewport(0, 0, width, height); }

int main()
{
    srand(static_cast<unsigned int>(time(nullptr)));
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "1-9 Moving Triangles", nullptr, nullptr);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetWindowSizeCallback(window, WindowSizeCallback);
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) { glfwDestroyWindow(window); glfwTerminate(); return -1; }
    int framebufferWidth, framebufferHeight;
    glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
    glViewport(0, 0, framebufferWidth, framebufferHeight);

    shaderProgram = CreateShaderProgram();
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    ResetTriangles();

    double previousTime = glfwGetTime();
    while (!glfwWindowShouldClose(window))
    {
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - previousTime);
        previousTime = currentTime;
        if (deltaTime > 0.05f) deltaTime = 0.05f;
        UpdateTriangles(deltaTime);
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
