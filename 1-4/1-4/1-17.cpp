#include <GL/glew.h>
#include <GL/glfw3.h>
#include <GL/glm/glm.hpp>
#include <GL/glm/gtc/matrix_transform.hpp>
#include <GL/glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

// 작업 폴더에서 찾고, 없으면 소스 파일 옆에서 찾는다.
// 프로젝트의 /FC 옵션으로 __FILE__에 전체 경로를 저장한다.
std::string ReadShaderFile(const char* fileName)
{
    std::ifstream file(fileName);
    if (!file.is_open())
    {
        std::filesystem::path sourceFolder = std::filesystem::path(__FILE__).parent_path();
        file.clear();
        file.open(sourceFolder / fileName);
    }
    if (!file.is_open())
    {
        std::cerr << "셰이더 파일을 열 수 없습니다: " << fileName << std::endl;
        return "";
    }

    std::string source;
    std::string line;
    while (std::getline(file, line))
    {
        source += line + "\n";
    }
    if (file.bad() || source.empty())
    {
        std::cerr << "셰이더 파일 읽기 실패: " << fileName << std::endl;
        return "";
    }
    return source;
}

GLuint CompileShader(GLenum type, const char* source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE)
    {
        char errorLog[1024];
        glGetShaderInfoLog(shader, 1024, nullptr, errorLog);
        std::cerr << "셰이더 컴파일 실패: " << errorLog << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

// 객체의 위치, 자세, 크기는 각각 따로 저장한다.
struct Object
{
    glm::vec3 position = glm::vec3(0.0f);
    glm::mat4 rotation = glm::mat4(1.0f);
    float size = 1.0f;
    int shape = 0;                  // 0: 육면체, 1: 원뿔, 2: 구, 3: 사각뿔
    int spinX = 0;
    int spinY = 0;
    int orbit = 0;
};

Object objects[2];
Object animationStart[2];
int selectedObject = 3;             // 1: 처음 왼쪽 객체, 2: 처음 오른쪽 객체, 3: 둘 다
int animationMode = 0;              // 0: 정지, 1: t, 2: u, 3: v
float animationTime = 0.0f;
const float animationDuration = 2.0f;
const float rotationSpeed = 60.0f;
bool wireframe = true;

// 한 번 만든 정점을 VBO에 넣고, 각 도형의 시작 위치와 개수를 기록한다.
std::vector<glm::vec3> vertices;
int shapeFirst[4];
int shapeCount[4];
int axesFirst = 0;
float faceColors[6][3] = {
    {0.9f, 0.15f, 0.15f}, {0.0f, 0.65f, 0.2f}, {0.1f, 0.25f, 0.95f},
    {0.8f, 0.65f, 0.0f}, {0.95f, 0.4f, 0.05f}, {0.65f, 0.1f, 0.8f}
};

void AddTriangle(glm::vec3 a, glm::vec3 b, glm::vec3 c)
{
    vertices.push_back(a);
    vertices.push_back(b);
    vertices.push_back(c);
}

glm::vec3 SpherePoint(float latitude, float longitude)
{
    return 0.32f * glm::vec3(std::cos(latitude) * std::cos(longitude),
                             std::sin(latitude),
                             std::cos(latitude) * std::sin(longitude));
}

void MakeVertices()
{
    vertices.clear();
    glm::vec3 points[8] = {
        {-0.28f, -0.28f, 0.28f}, {0.28f, -0.28f, 0.28f},
        {0.28f, 0.28f, 0.28f}, {-0.28f, 0.28f, 0.28f},
        {-0.28f, -0.28f, -0.28f}, {0.28f, -0.28f, -0.28f},
        {0.28f, 0.28f, -0.28f}, {-0.28f, 0.28f, -0.28f}
    };
    int faces[6][4] = {
        {0, 1, 2, 3}, {1, 5, 6, 2}, {5, 4, 7, 6},
        {4, 0, 3, 7}, {3, 2, 6, 7}, {4, 5, 1, 0}
    };
    shapeFirst[0] = (int)vertices.size();
    for (int face = 0; face < 6; face++)
    {
        // 네 꼭짓점을 함께 저장하면 테두리에 대각선이 생기지 않는다.
        for (int corner = 0; corner < 4; corner++)
            vertices.push_back(points[faces[face][corner]]);
    }
    shapeCount[0] = (int)vertices.size() - shapeFirst[0];

    const int segments = 24;
    const float pi = glm::radians(180.0f);
    shapeFirst[1] = (int)vertices.size();
    for (int i = 0; i < segments; i++)
    {
        float angle1 = 2.0f * pi * i / segments;
        float angle2 = 2.0f * pi * (i + 1) / segments;
        glm::vec3 a(0.3f * std::cos(angle1), -0.3f, 0.3f * std::sin(angle1));
        glm::vec3 b(0.3f * std::cos(angle2), -0.3f, 0.3f * std::sin(angle2));
        AddTriangle(a, b, glm::vec3(0.0f, 0.3f, 0.0f));
        AddTriangle(b, a, glm::vec3(0.0f, -0.3f, 0.0f));
    }
    shapeCount[1] = (int)vertices.size() - shapeFirst[1];

    shapeFirst[2] = (int)vertices.size();
    const int rings = 12;
    for (int row = 0; row < rings; row++)
    {
        float latitude1 = -pi / 2.0f + pi * row / rings;
        float latitude2 = -pi / 2.0f + pi * (row + 1) / rings;
        for (int column = 0; column < segments; column++)
        {
            float longitude1 = 2.0f * pi * column / segments;
            float longitude2 = 2.0f * pi * (column + 1) / segments;
            glm::vec3 a = SpherePoint(latitude1, longitude1);
            glm::vec3 b = SpherePoint(latitude1, longitude2);
            glm::vec3 c = SpherePoint(latitude2, longitude2);
            glm::vec3 d = SpherePoint(latitude2, longitude1);
            if (row != 0)
                AddTriangle(a, b, c);
            if (row != rings - 1)
                AddTriangle(a, c, d);
        }
    }
    shapeCount[2] = (int)vertices.size() - shapeFirst[2];

    shapeFirst[3] = (int)vertices.size();
    int base[4] = {0, 1, 5, 4};
    for (int face = 0; face < 4; face++)
        AddTriangle(points[base[face]], points[base[(face + 1) % 4]], glm::vec3(0.0f, 0.35f, 0.0f));
    AddTriangle(points[0], points[4], points[5]);
    AddTriangle(points[0], points[5], points[1]);
    shapeCount[3] = (int)vertices.size() - shapeFirst[3];

    axesFirst = (int)vertices.size();
    vertices.insert(vertices.end(), {
        {-1.8f, 0.0f, 0.0f}, {1.8f, 0.0f, 0.0f},
        {0.0f, -1.8f, 0.0f}, {0.0f, 1.8f, 0.0f},
        {0.0f, 0.0f, -1.8f}, {0.0f, 0.0f, 1.8f}
    });
}

void StopAnimation()
{
    animationMode = 0;
    animationTime = 0.0f;
    for (int i = 0; i < 2; i++)
    {
        objects[i].spinX = 0;
        objects[i].spinY = 0;
        objects[i].orbit = 0;
    }
}

void ResetObjects()
{
    objects[0] = Object();
    objects[1] = Object();
    objects[0].position.x = -0.85f;
    objects[1].position.x = 0.85f;
    objects[0].shape = 1;
    objects[1].shape = 0;
    selectedObject = 3;
    wireframe = true;
    StopAnimation();
}

void StartAnimation(int mode)
{
    bool stop = animationMode == mode;
    StopAnimation();
    if (stop)
        return;
    animationMode = mode;
    for (int i = 0; i < 2; i++)
        animationStart[i] = objects[i];
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS && action != GLFW_REPEAT)
        return;
    // 이동과 크기 변경은 길게 눌러도 된다. 나머지는 한 번만 처리한다.
    bool stepKey = key == GLFW_KEY_A || key == GLFW_KEY_B || key == GLFW_KEY_D || key == GLFW_KEY_E;
    if (action == GLFW_REPEAT && !stepKey)
        return;
    if (key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE)
        glfwSetWindowShouldClose(window, true);
    else if (key >= GLFW_KEY_1 && key <= GLFW_KEY_3)
        selectedObject = key - GLFW_KEY_0;
    else if (key == GLFW_KEY_S)
        ResetObjects();
    else if (key == GLFW_KEY_C)
    {
        // 원뿔/육면체와 구/사각뿔을 전환한다. 선택 번호와 무관하게 둘 다 바뀐다.
        objects[0].shape = objects[0].shape == 1 ? 2 : 1;
        objects[1].shape = objects[1].shape == 0 ? 3 : 0;
    }
    else if (key == GLFW_KEY_W)
        wireframe = !wireframe;
    else if (key == GLFW_KEY_T || key == GLFW_KEY_U || key == GLFW_KEY_V)
        StartAnimation(key == GLFW_KEY_T ? 1 : key == GLFW_KEY_U ? 2 : 3);
    else if (key == GLFW_KEY_X || key == GLFW_KEY_Y || key == GLFW_KEY_R || stepKey)
    {
        if (animationMode != 0)
            StopAnimation();
        int direction = (mods & GLFW_MOD_SHIFT) ? -1 : 1;
        for (int i = 0; i < 2; i++)
        {
            if (selectedObject != 3 && selectedObject != i + 1)
                continue;
            Object& object = objects[i];
            if (key == GLFW_KEY_X)
                object.spinX = object.spinX == direction ? 0 : direction;
            else if (key == GLFW_KEY_Y)
                object.spinY = object.spinY == direction ? 0 : direction;
            else if (key == GLFW_KEY_R)
                object.orbit = object.orbit == direction ? 0 : direction;
            else if (key == GLFW_KEY_D)
                object.position.x -= 0.1f * direction;
            else if (key == GLFW_KEY_E)
                object.position.y += 0.1f * direction;
            else if (key == GLFW_KEY_A || key == GLFW_KEY_B)
            {
                float factor = direction == 1 ? 1.1f : 1.0f / 1.1f;
                float newSize = std::clamp(object.size * factor, 0.1f, 4.0f);
                if (key == GLFW_KEY_B)
                    object.position *= newSize / object.size;
                object.size = newSize;
            }
        }
    }
}

void UpdateAnimation(float deltaTime)
{
    if (animationMode != 0)
    {
        animationTime += deltaTime;
        if (animationMode == 1 || animationMode == 2)
        {
            float progress = std::min(animationTime / animationDuration, 1.0f);
            for (int i = 0; i < 2; i++)
            {
                glm::vec3 start = animationStart[i].position;
                glm::vec3 end = animationStart[1 - i].position;
                if (animationMode == 1)
                {
                    // 이동 후 시작해도 반드시 중앙 원점을 거쳐 상대 위치로 간다.
                    if (progress < 0.5f)
                        objects[i].position = start * (1.0f - progress * 2.0f);
                    else
                        objects[i].position = end * (progress * 2.0f - 1.0f);
                }
                else
                {
                    objects[i].position = glm::mix(start, end, progress);
                    float height = 0.7f * std::sin(glm::radians(180.0f) * progress);
                    objects[i].position.y += i == 0 ? height : -height;
                }
            }
            if (progress == 1.0f)
            {
                // 삼각함수의 작은 오차도 남기지 않고 정확한 상대 위치에서 끝낸다.
                for (int i = 0; i < 2; i++)
                    objects[i].position = animationStart[1 - i].position;
                animationMode = 0;
            }
        }
        else
        {
            glm::mat4 orbit = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f * animationTime), glm::vec3(0, 1, 0));
            glm::mat4 spin = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f * animationTime), glm::vec3(0, 1, 0));
            for (int i = 0; i < 2; i++)
            {
                objects[i].position = glm::vec3(orbit * glm::vec4(animationStart[i].position, 1.0f));
                objects[i].rotation = orbit * animationStart[i].rotation * spin;
                float change = 0.4f * std::sin(animationTime * 2.0f);
                float factor = i == 0 ? 1.0f + change : 1.0f - change;
                objects[i].size = std::clamp(animationStart[i].size * factor, 0.1f, 4.0f);
            }
        }
        return;
    }

    for (int i = 0; i < 2; i++)
    {
        Object& object = objects[i];
        float angle = glm::radians(rotationSpeed * deltaTime);
        object.rotation = glm::rotate(object.rotation, angle * object.spinX, glm::vec3(1, 0, 0));
        object.rotation = glm::rotate(object.rotation, angle * object.spinY, glm::vec3(0, 1, 0));
        glm::mat4 orbit = glm::rotate(glm::mat4(1.0f), angle * object.orbit, glm::vec3(0, 1, 0));
        object.position = glm::vec3(orbit * glm::vec4(object.position, 1.0f));
        object.rotation = orbit * object.rotation;
    }
}

glm::mat4 GetSceneTransform()
{
    // 좌표축과 두 객체가 똑같은 x, y축 30도 기울기를 사용한다.
    glm::mat4 scene(1.0f);
    scene = glm::rotate(scene, glm::radians(30.0f), glm::vec3(1, 0, 0));
    scene = glm::rotate(scene, glm::radians(30.0f), glm::vec3(0, 1, 0));
    return scene;
}

glm::mat4 GetObjectTransform(int index)
{
    const Object& object = objects[index];
    glm::mat4 model = glm::translate(glm::mat4(1.0f), object.position);
    model *= object.rotation;
    model = glm::scale(model, glm::vec3(object.size));
    return model;
}

void DrawObject(int shape, GLint colorLocation)
{
    if (shape == 0)
    {
        for (int face = 0; face < 6; face++)
        {
            glUniform3fv(colorLocation, 1, faceColors[face]);
            glDrawArrays(wireframe ? GL_LINE_LOOP : GL_TRIANGLE_FAN, shapeFirst[0] + face * 4, 4);
        }
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
        for (int first = 0; first < shapeCount[shape]; first += 3)
        {
            int color = (first / 6) % 6;
            glUniform3fv(colorLocation, 1, faceColors[color]);
            glDrawArrays(GL_TRIANGLES, shapeFirst[shape] + first, 3);
        }
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

void DrawScene(GLint transformLocation, GLint colorLocation)
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glm::mat4 projection = glm::ortho(-2.2f, 2.2f, -2.2f, 2.2f, -6.0f, 6.0f);
    glm::mat4 scene = projection * GetSceneTransform();

    glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(scene));
    glUniform3f(colorLocation, 1.0f, 0.0f, 0.0f);
    glDrawArrays(GL_LINES, axesFirst, 2);
    glUniform3f(colorLocation, 0.0f, 0.7f, 0.0f);
    glDrawArrays(GL_LINES, axesFirst + 2, 2);
    glUniform3f(colorLocation, 0.0f, 0.0f, 1.0f);
    glDrawArrays(GL_LINES, axesFirst + 4, 2);

    for (int i = 0; i < 2; i++)
    {
        glm::mat4 transform = scene * GetObjectTransform(i);
        glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(transform));
        DrawObject(objects[i].shape, colorLocation);
    }
}

void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    // 정사각형 뷰포트로 창 크기를 바꿔도 도형 비율을 유지한다.
    int size = width;
    if (height < width)
        size = height;
    glViewport((width - size) / 2, (height - size) / 2, size, size);
}

int main()
{
    //--- GLFW 초기화
    if (!glfwInit())
    {
        std::cerr << "GLFW 초기화 실패!" << std::endl;
        return -1;
    }
    //--- OpenGL 버전 설정
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);

    //--- 윈도우 생성
    GLFWwindow* window = glfwCreateWindow(800, 600, "1-17 Modeling Transformations", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "윈도우 생성 실패!" << std::endl;
        glfwTerminate();
        return -1;
    }
    //--- 컨텍스트 설정
    glfwMakeContextCurrent(window);

    //--- GLEW 초기화
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "GLEW 초기화 실패!" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    glfwSwapInterval(1);

    //--- 셰이더 생성
    std::string vertexSource = ReadShaderFile("1-17-vertex.glsl");
    std::string fragmentSource = ReadShaderFile("1-17-fragment.glsl");
    if (vertexSource.empty() || fragmentSource.empty())
    {
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexSource.c_str());
    GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentSource.c_str());
    if (vertexShader == 0 || fragmentShader == 0)
    {
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (success == GL_FALSE)
    {
        char errorLog[1024];
        glGetProgramInfoLog(shaderProgram, 1024, nullptr, errorLog);
        std::cerr << "셰이더 연결 실패: " << errorLog << std::endl;
        glDeleteProgram(shaderProgram);
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    MakeVertices();
    ResetObjects();

    //--- VAO, VBO 생성: 위치 좌표만 저장하고 색은 면을 그릴 때 보낸다.
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);
    glEnableVertexAttribArray(0);

    //--- 뷰포트와 키보드 설정
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    FramebufferSizeCallback(window, width, height);
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    glfwSetKeyCallback(window, KeyCallback);
    glUseProgram(shaderProgram);
    GLint transformLocation = glGetUniformLocation(shaderProgram, "transform");
    GLint colorLocation = glGetUniformLocation(shaderProgram, "faceColor");

    std::cout << "1: 왼쪽 객체 / 2: 오른쪽 객체 / 3: 두 객체 선택\n";
    std::cout << "x/X, y/Y: 자전 / r/R: 공전 (같은 키: 정지, Shift: 반대 방향)\n";
    std::cout << "a/A: 제자리 확대/축소 / b/B: 원점 기준 확대/축소\n";
    std::cout << "d/D: 왼쪽/오른쪽 이동 / e/E: 위/아래 이동\n";
    std::cout << "t: 원점 통과 자리 교체 / u: 위아래로 자리 교체 / v: 크기 변경과 자전·공전\n";
    std::cout << "c: 두 도형 변경 / w: 선·면 전환 / s: 초기화 / q, Esc: 종료\n";
    std::cout << "t, u, v를 다시 누르면 현재 상태에서 정지합니다.\n" << std::flush;

    //--- 메인 루프
    double previousTime = glfwGetTime();
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        double currentTime = glfwGetTime();
        float deltaTime = (float)(currentTime - previousTime);
        previousTime = currentTime;
        if (deltaTime > 0.1f)
            deltaTime = 0.1f;
        UpdateAnimation(deltaTime);
        DrawScene(transformLocation, colorLocation);
        glfwSwapBuffers(window);
    }

    //--- 종료 처리
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
