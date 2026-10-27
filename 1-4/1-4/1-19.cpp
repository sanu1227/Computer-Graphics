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

glm::vec3 tankPosition(0.0f);
const float floorHalfSize = 7.0f;
// 포신과 깃대가 회전할 때의 최대 범위까지 바닥 안에 둔다.
const float tankMargin = 3.5f;
bool animateBase = false;
bool animateCenter = false;
bool animateGuns = false;
bool animatePoles = false;
bool swapping = false;
float baseTime = 0.0f;
float centerAngle = 0.0f;
float gunTime = 0.0f;
float poleTime = 0.0f;
float swapProgress = 0.0f;
float swapStart = 0.0f;
float swapTarget = 0.0f;
float swapTime = 0.0f;
glm::vec3 cameraPosition(7, 6, 10);
glm::mat4 cameraRotation(1.0f);
int cameraSpin = 0;
int cameraOrbit = 0;
float aspectRatio = 800.0f / 600.0f;
std::vector<glm::vec3> vertices;
const glm::vec3 initialColors[9] = {
    {0.2f, 0.3f, 0.4f}, {0.35f, 0.3f, 0.45f}, {0.25f, 0.6f, 0.4f},
    {0.75f, 0.85f, 0.25f}, {0.25f, 0.75f, 0.8f}, {0.95f, 0.55f, 0.15f},
    {0.85f, 0.3f, 0.2f}, {0.85f, 0.3f, 0.7f}, {0.55f, 0.4f, 0.9f}
};
glm::vec3 partColors[9];

void MakeVertices()
{
    vertices.clear();
    glm::vec3 points[8] = {
        {-0.5f, -0.5f, 0.5f}, {0.5f, -0.5f, 0.5f},
        {0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f},
        {-0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, -0.5f},
        {0.5f, 0.5f, -0.5f}, {-0.5f, 0.5f, -0.5f}
    };
    int faces[6][4] = {
        {0, 1, 2, 3}, {1, 5, 6, 2}, {5, 4, 7, 6},
        {4, 0, 3, 7}, {3, 2, 6, 7}, {4, 5, 1, 0}
    };
    for (int face = 0; face < 6; face++)
        for (int corner = 0; corner < 4; corner++)
            vertices.push_back(points[faces[face][corner]]);
}

void ResetObjects()
{
    tankPosition = glm::vec3(0);
    animateBase = animateCenter = animateGuns = animatePoles = swapping = false;
    baseTime = centerAngle = gunTime = poleTime = 0.0f;
    swapProgress = swapStart = swapTarget = swapTime = 0.0f;
    cameraPosition = glm::vec3(7, 6, 10);
    glm::mat4 view = glm::lookAt(cameraPosition, glm::vec3(0, 0.8f, 0), glm::vec3(0, 1, 0));
    cameraRotation = glm::mat4(glm::transpose(glm::mat3(view)));
    cameraSpin = cameraOrbit = 0;
    for (int i = 0; i < 9; i++)
        partColors[i] = initialColors[i];
}

float GetBaseHeight()
{
    return 0.6f + 0.2f * std::sin(baseTime * 2.0f);
}

glm::mat4 GetCenterTransform()
{
    glm::mat4 model = glm::translate(glm::mat4(1), tankPosition);
    model = glm::translate(model, glm::vec3(0, GetBaseHeight(), 0));
    return glm::rotate(model, glm::radians(centerAngle), glm::vec3(0, 1, 0));
}

glm::mat4 GetUpperTransform(int index)
{
    float side = index == 0 ? -1.0f : 1.0f;
    float angle = glm::radians(180.0f) * swapProgress;
    glm::vec3 position(side * 0.85f * std::cos(angle), 0.4f, side * 0.9f * std::sin(angle));
    // 중앙 몸체의 회전은 상부, 포신, 깃대에 함께 적용된다.
    return glm::translate(GetCenterTransform(), position);
}

glm::mat4 MakeBox(const glm::mat4& parent, glm::vec3 position, glm::vec3 size)
{
    return glm::scale(glm::translate(parent, position), size);
}

void GetPartTransforms(glm::mat4 parts[9])
{
    glm::mat4 root = glm::translate(glm::mat4(1), tankPosition);
    float height = GetBaseHeight();
    // 아래쪽 면은 항상 y=0이다. 높이가 바뀌면 중심도 높이의 절반만큼 올린다.
    parts[0] = MakeBox(root, glm::vec3(-0.95f, height / 2, 0), glm::vec3(0.75f, height, 2.6f));
    parts[1] = MakeBox(root, glm::vec3(0.95f, height / 2, 0), glm::vec3(0.85f, height, 2.4f));
    parts[2] = MakeBox(GetCenterTransform(), glm::vec3(0, 0.2f, 0), glm::vec3(3.0f, 0.4f, 2.2f));
    for (int i = 0; i < 2; i++)
    {
        float side = i == 0 ? 1.0f : -1.0f;
        float upperHeight = i == 0 ? 0.65f : 0.8f;
        float upperDepth = i == 0 ? 1.0f : 0.9f;
        glm::mat4 upper = GetUpperTransform(i);
        parts[3 + i] = MakeBox(upper, glm::vec3(0, upperHeight / 2, 0),
                              glm::vec3(i == 0 ? 1.0f : 0.9f, upperHeight, upperDepth));
        // 포신의 뿌리에서 먼저 회전하고, 포신 중심을 길이의 절반만큼 이동한다.
        glm::mat4 gun = glm::translate(upper, glm::vec3(0, upperHeight / 2, upperDepth / 2));
        gun = glm::rotate(gun, glm::radians(side * 55.0f * std::sin(gunTime * 1.5f)), glm::vec3(0, 1, 0));
        parts[5 + i] = MakeBox(gun, glm::vec3(0, 0, 0.65f), glm::vec3(0.14f, 0.14f, 1.3f));
        // 깃대도 상부 몸체 위의 뿌리를 중심으로 서로 반대로 기울인다.
        glm::mat4 pole = glm::translate(upper, glm::vec3(0, upperHeight, 0));
        pole = glm::rotate(pole, glm::radians(side * 40.0f * std::sin(poleTime * 1.5f)), glm::vec3(1, 0, 0));
        parts[7 + i] = MakeBox(pole, glm::vec3(0, 0.6f, 0), glm::vec3(0.1f, 1.2f, 0.1f));
    }
}

glm::mat4 GetViewTransform()
{
    glm::mat4 camera = glm::translate(glm::mat4(1), cameraPosition) * cameraRotation;
    return glm::inverse(camera);
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS && action != GLFW_REPEAT)
        return;
    bool stepKey = key == GLFW_KEY_LEFT || key == GLFW_KEY_RIGHT || key == GLFW_KEY_UP
        || key == GLFW_KEY_DOWN || key == GLFW_KEY_X || key == GLFW_KEY_Z;
    if (action == GLFW_REPEAT && !stepKey)
        return;
    int direction = (mods & GLFW_MOD_SHIFT) ? -1 : 1;
    if (key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE)
        glfwSetWindowShouldClose(window, true);
    else if (key == GLFW_KEY_LEFT)
        tankPosition.x -= 0.2f;
    else if (key == GLFW_KEY_RIGHT)
        tankPosition.x += 0.2f;
    else if (key == GLFW_KEY_UP)
        tankPosition.z -= 0.2f;
    else if (key == GLFW_KEY_DOWN)
        tankPosition.z += 0.2f;
    else if (key == GLFW_KEY_D)
        animateBase = !animateBase;
    else if (key == GLFW_KEY_T)
        animateCenter = !animateCenter;
    else if (key == GLFW_KEY_G)
        animateGuns = !animateGuns;
    else if (key == GLFW_KEY_P)
        animatePoles = !animatePoles;
    else if (key == GLFW_KEY_L && !swapping)
    {
        swapStart = swapProgress;
        swapTarget = 1.0f - swapTarget;
        swapTime = 0.0f;
        swapping = true;
    }
    else if (key == GLFW_KEY_Z)
        cameraPosition.z += 0.3f * direction;
    else if (key == GLFW_KEY_X)
        cameraPosition.x += 0.3f * direction;
    else if (key == GLFW_KEY_Y)
        cameraSpin = cameraSpin == direction ? 0 : direction;
    else if (key == GLFW_KEY_R)
        cameraOrbit = cameraOrbit == direction ? 0 : direction;
    else if (key == GLFW_KEY_O)
    {
        for (glm::vec3& color : partColors)
            for (int channel = 0; channel < 3; channel++)
                color[channel] = 0.2f + 0.75f * (float)std::rand() / RAND_MAX;
    }
    else if (key == GLFW_KEY_C)
        ResetObjects();
    float limit = floorHalfSize - tankMargin;
    tankPosition.x = std::clamp(tankPosition.x, -limit, limit);
    tankPosition.z = std::clamp(tankPosition.z, -limit, limit);
}

void UpdateAnimation(float deltaTime)
{
    if (animateBase)
        baseTime = std::fmod(baseTime + deltaTime, glm::radians(180.0f));
    if (animateCenter)
        centerAngle = std::fmod(centerAngle + 45.0f * deltaTime, 360.0f);
    if (animateGuns)
        gunTime = std::fmod(gunTime + deltaTime, glm::radians(360.0f) / 1.5f);
    if (animatePoles)
        poleTime = std::fmod(poleTime + deltaTime, glm::radians(360.0f) / 1.5f);
    if (swapping)
    {
        swapTime += deltaTime;
        float progress = std::min(swapTime / 2.0f, 1.0f);
        swapProgress = glm::mix(swapStart, swapTarget, progress);
        if (progress == 1.0f)
            swapping = false;
    }
    // 양의 y축 위에서 내려다볼 때 음의 각도가 시계방향이다.
    if (cameraSpin != 0)
        cameraRotation = glm::rotate(cameraRotation, glm::radians(-35.0f * cameraSpin * deltaTime), glm::vec3(0, 1, 0));
    if (cameraOrbit != 0)
    {
        glm::mat4 orbit = glm::rotate(glm::mat4(1), glm::radians(-25.0f * cameraOrbit * deltaTime), glm::vec3(0, 1, 0));
        cameraPosition = glm::vec3(orbit * glm::vec4(cameraPosition, 1));
        cameraRotation = orbit * cameraRotation;
    }
}

void DrawBox(const glm::mat4& transform, glm::vec3 color, GLint transformLocation, GLint colorLocation)
{
    glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(transform));
    const float shades[6] = {0.85f, 0.65f, 0.75f, 0.7f, 1.0f, 0.5f};
    for (int face = 0; face < 6; face++)
    {
        glm::vec3 faceColor = color * shades[face];
        glUniform3fv(colorLocation, 1, glm::value_ptr(faceColor));
        glDrawArrays(GL_TRIANGLE_FAN, face * 4, 4);
    }
}

void DrawScene(GLint transformLocation, GLint colorLocation)
{
    glClearColor(0.96f, 0.97f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glm::mat4 projection = glm::perspective(glm::radians(50.0f), aspectRatio, 0.1f, 100.0f);
    glm::mat4 viewProjection = projection * GetViewTransform();
    glm::mat4 floor = MakeBox(glm::mat4(1), glm::vec3(0, -0.1f, 0), glm::vec3(floorHalfSize * 2, 0.2f, floorHalfSize * 2));
    DrawBox(viewProjection * floor, glm::vec3(0.78f, 0.85f, 0.92f), transformLocation, colorLocation);
    glm::mat4 parts[9];
    GetPartTransforms(parts);
    for (int i = 0; i < 9; i++)
        DrawBox(viewProjection * parts[i], partColors[i], transformLocation, colorLocation);
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
    GLFWwindow* window = glfwCreateWindow(800, 600, "1-19 Tank and Camera", nullptr, nullptr);
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
    std::string vertexSource = ReadShaderFile("1-19-vertex.glsl");
    std::string fragmentSource = ReadShaderFile("1-19-fragment.glsl");
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

    std::srand((unsigned int)std::time(nullptr));
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

    std::cout << "방향키: 탱크 xz 이동, d: 아래 몸체 높이 변화, t: 중앙 몸체 회전\n";
    std::cout << "l: 상부 자리 교환, g: 포신 회전, p: 깃대 회전\n";
    std::cout << "x/X, z/Z: 카메라 양/음 방향 이동 (대문자: Shift)\n";
    std::cout << "y/Y: 카메라 자전, r/R: 원점 공전 (시계/반시계, 같은 키로 정지)\n";
    std::cout << "o: 색상 변경, c: 전체 초기화, q/Esc: 종료\n" << std::flush;

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
