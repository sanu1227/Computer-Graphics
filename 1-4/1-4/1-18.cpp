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

// 중심 구, 행성 3개, 달 3개를 하나의 장면으로 이동한다.
glm::vec3 scenePosition(0.0f);
bool perspective = true;
bool wireframe = false;
bool largeOrbits = false;
bool pulse = false;
bool rotateMoonOrbits = false;
float pulseTime = 0.0f;
float moonOrbitAngle = 0.0f;
float planetAngles[3] = {15.0f, 145.0f, 260.0f};
float moonAngles[3] = {0.0f, 120.0f, 240.0f};
const float planetSpeeds[3] = {22.0f, -33.0f, 47.0f};
const float moonSpeeds[3] = {90.0f, -120.0f, 150.0f};
const float orbitTilts[3] = {0.0f, 45.0f, -45.0f};
const float orbitRadii[3] = {1.8f, 2.5f, 3.2f};
const glm::vec3 planetColors[3] = {
    {0.15f, 0.65f, 0.95f}, {0.15f, 0.8f, 0.35f}, {0.85f, 0.3f, 0.85f}
};
const glm::vec3 moonColors[3] = {
    {0.95f, 0.25f, 0.15f}, {0.15f, 0.85f, 0.85f}, {0.65f, 0.65f, 0.85f}
};
float aspectRatio = 800.0f / 600.0f;
std::vector<glm::vec3> vertices;
int sphereCount = 0;
int orbitFirst = 0;
const int orbitSegments = 120;
GLint lightingLocation = -1;

glm::vec3 SpherePoint(float latitude, float longitude)
{
    return glm::vec3(std::cos(latitude) * std::cos(longitude),
                     std::sin(latitude),
                     std::cos(latitude) * std::sin(longitude));
}

void MakeVertices()
{
    vertices.clear();
    const int rings = 18;
    const int segments = 36;
    const float pi = glm::radians(180.0f);
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
                vertices.insert(vertices.end(), {a, b, c});
            if (row != rings - 1)
                vertices.insert(vertices.end(), {a, c, d});
        }
    }
    sphereCount = (int)vertices.size();
    orbitFirst = sphereCount;
    // 기본 원은 xy 평면에 만든다. 행성 궤도만 xz 평면으로 돌린다.
    for (int i = 0; i < orbitSegments; i++)
    {
        float angle = 2.0f * pi * i / orbitSegments;
        vertices.push_back(glm::vec3(std::cos(angle), std::sin(angle), 0.0f));
    }
}

void ResetObjects()
{
    scenePosition = glm::vec3(0);
    perspective = true;
    wireframe = largeOrbits = pulse = rotateMoonOrbits = false;
    pulseTime = moonOrbitAngle = 0.0f;
    const float startAngles[3] = {15.0f, 145.0f, 260.0f};
    for (int i = 0; i < 3; i++)
    {
        planetAngles[i] = startAngles[i];
        moonAngles[i] = 120.0f * i;
    }
}

glm::mat4 GetViewTransform()
{
    return glm::lookAt(glm::vec3(0, 5, 11), glm::vec3(0), glm::vec3(0, 1, 0));
}

glm::mat4 GetProjection()
{
    if (perspective)
        return glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
    return glm::ortho(-4.8f * aspectRatio, 4.8f * aspectRatio, -4.8f, 4.8f, 0.1f, 100.0f);
}

float GetOrbitScale()
{
    return largeOrbits ? 1.25f : 1.0f;
}

glm::mat4 GetPlanetOrbitTransform(int index)
{
    glm::mat4 model = glm::translate(glm::mat4(1), scenePosition);
    model = glm::rotate(model, glm::radians(orbitTilts[index]), glm::vec3(0, 0, 1));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1, 0, 0));
    return glm::scale(model, glm::vec3(orbitRadii[index] * GetOrbitScale()));
}

glm::vec3 GetPlanetPosition(int index)
{
    float angle = glm::radians(planetAngles[index]);
    return glm::vec3(GetPlanetOrbitTransform(index) * glm::vec4(std::cos(angle), std::sin(angle), 0, 1));
}

glm::mat4 GetMoonOrbitTransform(int index)
{
    glm::mat4 model = glm::translate(glm::mat4(1), GetPlanetPosition(index));
    // 행성 궤도의 기울기를 상속하지 않는다. 기본 달 궤도는 화면과 평행하다.
    model = glm::rotate(model, glm::radians(moonOrbitAngle), glm::vec3(0, 0, 1));
    model *= glm::mat4(glm::transpose(glm::mat3(GetViewTransform())));
    return glm::scale(model, glm::vec3(0.48f * GetOrbitScale()));
}

glm::vec3 GetMoonPosition(int index)
{
    float angle = glm::radians(moonAngles[index]);
    return glm::vec3(GetMoonOrbitTransform(index) * glm::vec4(std::cos(angle), std::sin(angle), 0, 1));
}

float GetSunSize()
{
    return 0.55f + 0.15f * std::sin(pulseTime * 2.0f);
}

glm::vec3 GetSunColor()
{
    float blend = 0.5f + 0.5f * std::sin(pulseTime * 2.0f);
    return glm::mix(glm::vec3(1.0f, 0.35f, 0.05f), glm::vec3(1.0f, 0.85f, 0.15f), blend);
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS && action != GLFW_REPEAT)
        return;
    bool moveKey = key == GLFW_KEY_W || key == GLFW_KEY_A || key == GLFW_KEY_S || key == GLFW_KEY_D
        || key == GLFW_KEY_EQUAL || key == GLFW_KEY_MINUS || key == GLFW_KEY_KP_ADD || key == GLFW_KEY_KP_SUBTRACT;
    if (action == GLFW_REPEAT && !moveKey)
        return;
    bool upper = (mods & GLFW_MOD_SHIFT) != 0;
    if (key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE)
        glfwSetWindowShouldClose(window, true);
    else if (key == GLFW_KEY_P)
        perspective = upper;
    else if (key == GLFW_KEY_M)
        wireframe = upper;
    // 일반적인 WASD 조작: W 위, A 왼쪽, S 아래, D 오른쪽.
    else if (key == GLFW_KEY_W)
        scenePosition.y += 0.15f;
    else if (key == GLFW_KEY_A)
        scenePosition.x -= 0.15f;
    else if (key == GLFW_KEY_S)
        scenePosition.y -= 0.15f;
    else if (key == GLFW_KEY_D)
        scenePosition.x += 0.15f;
    else if (key == GLFW_KEY_EQUAL || key == GLFW_KEY_KP_ADD)
        scenePosition.z += 0.15f;
    else if (key == GLFW_KEY_MINUS || key == GLFW_KEY_KP_SUBTRACT)
        scenePosition.z -= 0.15f;
    else if (key == GLFW_KEY_1)
        largeOrbits = !largeOrbits;
    else if (key == GLFW_KEY_2)
        pulse = !pulse;
    else if (key == GLFW_KEY_3)
        rotateMoonOrbits = !rotateMoonOrbits;
}

void UpdateAnimation(float deltaTime)
{
    for (int i = 0; i < 3; i++)
    {
        planetAngles[i] = std::fmod(planetAngles[i] + planetSpeeds[i] * deltaTime, 360.0f);
        moonAngles[i] = std::fmod(moonAngles[i] + moonSpeeds[i] * deltaTime, 360.0f);
    }
    if (pulse)
        pulseTime = std::fmod(pulseTime + deltaTime, glm::radians(180.0f));
    if (rotateMoonOrbits)
        moonOrbitAngle = std::fmod(moonOrbitAngle + 45.0f * deltaTime, 360.0f);
}

void DrawSphere(glm::vec3 position, float size, glm::vec3 color, const glm::mat4& viewProjection,
                GLint transformLocation, GLint colorLocation)
{
    glm::mat4 model = glm::translate(glm::mat4(1), position);
    model = glm::scale(model, glm::vec3(size));
    glm::mat4 transform = viewProjection * model;
    glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(transform));
    glUniform3fv(colorLocation, 1, glm::value_ptr(color));
    glUniform1i(lightingLocation, true);
    glDrawArrays(GL_TRIANGLES, 0, sphereCount);
}

void DrawAllSpheres(const glm::mat4& viewProjection, GLint transformLocation, GLint colorLocation)
{
    DrawSphere(scenePosition, GetSunSize(), GetSunColor(), viewProjection, transformLocation, colorLocation);
    for (int i = 0; i < 3; i++)
    {
        DrawSphere(GetPlanetPosition(i), 0.24f, planetColors[i], viewProjection, transformLocation, colorLocation);
        DrawSphere(GetMoonPosition(i), 0.10f, moonColors[i], viewProjection, transformLocation, colorLocation);
    }
}

void DrawScene(GLint transformLocation, GLint colorLocation)
{
    glClearColor(0.96f, 0.97f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glm::mat4 viewProjection = GetProjection() * GetViewTransform();
    if (wireframe)
    {
        // 색 없이 면의 깊이를 먼저 기록하여 뒤쪽 선과 궤도를 가린다.
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.0f, 1.0f);
        DrawAllSpheres(viewProjection, transformLocation, colorLocation);
        glDisable(GL_POLYGON_OFFSET_FILL);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    DrawAllSpheres(viewProjection, transformLocation, colorLocation);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glUniform1i(lightingLocation, false);
    for (int i = 0; i < 3; i++)
    {
        glm::mat4 transform = viewProjection * GetPlanetOrbitTransform(i);
        glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(transform));
        glUniform3f(colorLocation, 0.4f, 0.45f, 0.5f);
        glDrawArrays(GL_LINE_LOOP, orbitFirst, orbitSegments);
        transform = viewProjection * GetMoonOrbitTransform(i);
        glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(transform));
        glDrawArrays(GL_LINE_LOOP, orbitFirst, orbitSegments);
    }
}

void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    if (width > 0 && height > 0)
        aspectRatio = (float)width / height;
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
    GLFWwindow* window = glfwCreateWindow(800, 600, "1-18 Planet and Moon Orbits", nullptr, nullptr);
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
    std::string vertexSource = ReadShaderFile("1-18-vertex.glsl");
    std::string fragmentSource = ReadShaderFile("1-18-fragment.glsl");
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
    lightingLocation = glGetUniformLocation(shaderProgram, "sphereLighting");

    std::cout << "p/P: 직각/원근 투영, m/M: 솔리드/와이어 (대문자: Shift)\n";
    std::cout << "W: 위, A: 왼쪽, S: 아래, D: 오른쪽, +/-: z축 앞/뒤 이동\n";
    std::cout << "1: 모든 궤도 확대/복원, 2: 중심 구 크기·색 변화 시작/정지\n";
    std::cout << "3: 달 궤도 z축 회전 시작/정지, q/Esc: 종료\n" << std::flush;

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
