#include <GL/glew.h>
#include <GL/glfw3.h>
#include <GL/glm/glm.hpp>
#include <GL/glm/gtc/matrix_transform.hpp>
#include <GL/glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

// 프로젝트 속성 > 링커 > 명령줄: opengl32.lib glew32.lib glfw3.lib

const float startAngleX = 25.0f;
const float startAngleY = 30.0f;
const float rotationSpeed = 60.0f;  // 1초에 60도 회전한다.

int objectType = 0;                // 0: 육면체, 1: 사각뿔
bool hiddenSurface = true;
bool rotateObject = false;         // y: 전체 자전
bool rotateTop = false;            // t: 윗면 회전
bool openFront = false;            // f: 앞면을 열지 여부
bool rotateSide = false;           // s: 오른쪽 면 회전
bool hideBack = false;             // b: 뒷면을 숨길지 여부
int twoFaceMode = 0;               // 0: 정지, 1: 모서리 회전, 2: 자리 교체
int fourFaceMode = 0;              // 0: 정지, 4: y축 공전, 5: x축 공전

float objectAngle = 0.0f;
float topAngle = 0.0f;
float frontAngle = 0.0f;
float sideAngle = 0.0f;
float backScale = 1.0f;
float twoFaceAngle = 0.0f;
float fourFaceAngle = 0.0f;
int firstFace = 0;                 // 두 면 애니메이션에서 선택한 면 번호
int secondFace = 1;

// 면 번호: 0=앞, 1=오른쪽, 2=뒤, 3=왼쪽, 4=위, 5=아래
// 사각형 한 면은 삼각형 2개(정점 6개)로 만든다.
// 같은 위치라도 면마다 정점을 따로 두어 각각 다른 행렬을 적용한다.
float vertices[60][3] = {
    // 육면체 앞면: 0~5
    { -0.4f, -0.4f,  0.4f }, {  0.4f, -0.4f,  0.4f }, {  0.4f,  0.4f,  0.4f },
    { -0.4f, -0.4f,  0.4f }, {  0.4f,  0.4f,  0.4f }, { -0.4f,  0.4f,  0.4f },
    // 육면체 오른쪽 면: 6~11
    {  0.4f, -0.4f,  0.4f }, {  0.4f, -0.4f, -0.4f }, {  0.4f,  0.4f, -0.4f },
    {  0.4f, -0.4f,  0.4f }, {  0.4f,  0.4f, -0.4f }, {  0.4f,  0.4f,  0.4f },
    // 육면체 뒷면: 12~17
    {  0.4f, -0.4f, -0.4f }, { -0.4f, -0.4f, -0.4f }, { -0.4f,  0.4f, -0.4f },
    {  0.4f, -0.4f, -0.4f }, { -0.4f,  0.4f, -0.4f }, {  0.4f,  0.4f, -0.4f },
    // 육면체 왼쪽 면: 18~23
    { -0.4f, -0.4f, -0.4f }, { -0.4f, -0.4f,  0.4f }, { -0.4f,  0.4f,  0.4f },
    { -0.4f, -0.4f, -0.4f }, { -0.4f,  0.4f,  0.4f }, { -0.4f,  0.4f, -0.4f },
    // 육면체 윗면: 24~29
    { -0.4f,  0.4f,  0.4f }, {  0.4f,  0.4f,  0.4f }, {  0.4f,  0.4f, -0.4f },
    { -0.4f,  0.4f,  0.4f }, {  0.4f,  0.4f, -0.4f }, { -0.4f,  0.4f, -0.4f },
    // 육면체 아랫면: 30~35
    { -0.4f, -0.4f, -0.4f }, {  0.4f, -0.4f, -0.4f }, {  0.4f, -0.4f,  0.4f },
    { -0.4f, -0.4f, -0.4f }, {  0.4f, -0.4f,  0.4f }, { -0.4f, -0.4f,  0.4f },
    // 사각뿔 앞면: 36~38
    { -0.4f, -0.4f,  0.4f }, {  0.4f, -0.4f,  0.4f }, {  0.0f,  0.4f,  0.0f },
    // 사각뿔 오른쪽 면: 39~41
    {  0.4f, -0.4f,  0.4f }, {  0.4f, -0.4f, -0.4f }, {  0.0f,  0.4f,  0.0f },
    // 사각뿔 뒷면: 42~44
    {  0.4f, -0.4f, -0.4f }, { -0.4f, -0.4f, -0.4f }, {  0.0f,  0.4f,  0.0f },
    // 사각뿔 왼쪽 면: 45~47
    { -0.4f, -0.4f, -0.4f }, { -0.4f, -0.4f,  0.4f }, {  0.0f,  0.4f,  0.0f },
    // 사각뿔 바닥: 48~53
    { -0.4f, -0.4f, -0.4f }, {  0.4f, -0.4f, -0.4f }, {  0.4f, -0.4f,  0.4f },
    { -0.4f, -0.4f, -0.4f }, {  0.4f, -0.4f,  0.4f }, { -0.4f, -0.4f,  0.4f },
    // 화면 좌표축: 54~59. 도형과 별도로 고정해서 그린다.
    { -1.0f, -0.3f, 0.0f }, { 1.0f,  0.3f, 0.0f },  // x축: 오른쪽 위
    {  0.0f, -1.0f, 0.0f }, { 0.0f,  1.0f, 0.0f },  // y축: 수직
    { -0.55f, 0.5f, 0.0f }, { 0.55f,-0.5f, 0.0f }   // z축 표시: 오른쪽 아래
};

float faceColors[6][3] = {
    { 1.0f, 0.0f, 0.0f },  // 빨강
    { 0.0f, 0.7f, 0.0f },  // 초록
    { 0.0f, 0.0f, 1.0f },  // 파랑
    { 1.0f, 1.0f, 0.0f },  // 노랑
    { 1.0f, 0.5f, 0.0f },  // 주황
    { 0.6f, 0.0f, 0.8f }   // 보라
};

// 면의 중심. 면 번호는 앞, 오른쪽, 뒤, 왼쪽, 위, 아래 순서이다.
glm::vec3 faceCenters[6] = {
    { 0.0f, 0.0f, 0.4f }, { 0.4f, 0.0f, 0.0f },
    { 0.0f, 0.0f,-0.4f }, {-0.4f, 0.0f, 0.0f },
    { 0.0f, 0.4f, 0.0f }, { 0.0f,-0.4f, 0.0f }
};

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

// 여러 면 명령을 새로 시작할 때 면들을 원래 위치로 돌린다.
// 전체 도형의 자전과 은면 제거 설정은 여기서 바꾸지 않는다.
void ResetFaces()
{
    rotateTop = false;
    openFront = false;
    rotateSide = false;
    hideBack = false;
    twoFaceMode = 0;
    fourFaceMode = 0;
    topAngle = 0.0f;
    frontAngle = 0.0f;
    sideAngle = 0.0f;
    backScale = 1.0f;
    twoFaceAngle = 0.0f;
    fourFaceAngle = 0.0f;
    firstFace = 0;
    secondFace = 1;
}

void ResetObject()
{
    objectType = 0;
    hiddenSurface = true;
    rotateObject = false;
    objectAngle = 0.0f;
    ResetFaces();
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // 누르고 있을 때 반복해서 켜졌다 꺼지지 않게 처음 누른 순간만 처리한다.
    if (action != GLFW_PRESS)
        return;

    if (key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE)
        glfwSetWindowShouldClose(window, true);
    else if (key == GLFW_KEY_H)
        hiddenSurface = !hiddenSurface;
    else if (key == GLFW_KEY_P)
        objectType = 1 - objectType;
    else if (key == GLFW_KEY_Y)
        rotateObject = !rotateObject;
    else if (key == GLFW_KEY_R)
        ResetObject();
    else if (objectType == 0)
    {
        // 한 면 명령으로 돌아오면 진행 중이던 여러 면 변환을 초기화한다.
        bool singleFaceKey = key == GLFW_KEY_T || key == GLFW_KEY_F ||
                             key == GLFW_KEY_S || key == GLFW_KEY_B;
        if (singleFaceKey && (twoFaceMode != 0 || fourFaceMode != 0))
            ResetFaces();

        if (key == GLFW_KEY_T)
            rotateTop = !rotateTop;
        else if (key == GLFW_KEY_F)
            openFront = !openFront;
        else if (key == GLFW_KEY_S)
            rotateSide = !rotateSide;
        else if (key == GLFW_KEY_B)
            hideBack = !hideBack;
        else if (key == GLFW_KEY_1 || key == GLFW_KEY_2)
        {
            ResetFaces();
            twoFaceMode = key == GLFW_KEY_1 ? 1 : 2;
            firstFace = std::rand() % 4;
            secondFace = (firstFace + 1) % 4;  // 3번과 0번도 이웃한다.
        }
        else if (key == GLFW_KEY_4 || key == GLFW_KEY_5)
        {
            ResetFaces();
            fourFaceMode = key == GLFW_KEY_4 ? 4 : 5;
        }
    }
}

void UpdateAnimation(float deltaTime)
{
    // deltaTime: 직전 프레임부터 지난 시간(초).
    float angleStep = rotationSpeed * deltaTime;
    if (rotateObject)
        objectAngle = std::fmod(objectAngle + angleStep, 360.0f);

    // 사각뿔에서는 전체 자전만 진행하고, 육면체의 면 상태는 잠시 보관한다.
    if (objectType != 0)
        return;

    if (rotateTop)
        topAngle = std::fmod(topAngle + angleStep, 360.0f);
    if (rotateSide)
        sideAngle = std::fmod(sideAngle + angleStep, 360.0f);

    // 앞면은 0~90도에서 열고 닫는다. 도중에 f를 누르면 방향을 바꾼다.
    if (openFront)
        frontAngle = std::min(frontAngle + angleStep, 90.0f);
    else
        frontAngle = std::max(frontAngle - angleStep, 0.0f);

    // 1초에 원래 크기의 1배만큼 줄이거나 늘린다.
    if (hideBack)
        backScale = std::max(backScale - deltaTime, 0.0f);
    else
        backScale = std::min(backScale + deltaTime, 1.0f);

    if (twoFaceMode == 1)
    {
        twoFaceAngle += angleStep;
        if (twoFaceAngle >= 360.0f)
        {
            twoFaceAngle = 0.0f;
            twoFaceMode = 0;  // 한 바퀴를 마치면 원래 위치에서 멈춘다.
        }
    }
    else if (twoFaceMode == 2)
        twoFaceAngle = std::min(twoFaceAngle + angleStep, 90.0f);

    if (fourFaceMode != 0)
        fourFaceAngle = std::fmod(fourFaceAngle + angleStep, 360.0f);
}

glm::mat4 GetFaceTransform(int face)
{
    glm::mat4 faceTransform = glm::mat4(1.0f);
    glm::vec3 center = faceCenters[face];
    glm::vec3 axis(1.0f, 0.0f, 0.0f);
    float angle = 0.0f;

    if (twoFaceMode != 0)
    {
        if (face != firstFace && face != secondFace)
            return faceTransform;
        axis = glm::vec3(0.0f, 1.0f, 0.0f);
        if (twoFaceMode == 1)
        {
            // 이웃한 두 면 중심의 합이 공통 세로 모서리의 중점이다.
            center = faceCenters[firstFace] + faceCenters[secondFace];
            angle = twoFaceAngle;
        }
        else
        {
            center = glm::vec3(0.0f);
            angle = face == firstFace ? twoFaceAngle : -twoFaceAngle;
        }
    }
    else if (fourFaceMode != 0)
    {
        center = glm::vec3(0.0f);
        if (fourFaceMode == 4)
        {
            if (face >= 4)
                return faceTransform;
            axis = glm::vec3(0.0f, 1.0f, 0.0f);
        }
        else if (face == 1 || face == 3)
            return faceTransform;  // x축 공전에서는 오른쪽과 왼쪽 면은 고정한다.
        angle = fourFaceAngle;
    }
    else if (face == 4)
    {
        axis = glm::vec3(0.0f, 0.0f, 1.0f);
        angle = topAngle;
    }
    else if (face == 0)
    {
        center = glm::vec3(0.0f, 0.4f, 0.4f);  // 앞면 위 모서리의 중점
        angle = -frontAngle;  // 앞쪽 바깥으로 열린다.
    }
    else if (face == 1)
        angle = sideAngle;
    else if (face == 2)
    {
        faceTransform = glm::translate(faceTransform, center);
        faceTransform = glm::scale(faceTransform, glm::vec3(backScale));
        faceTransform = glm::translate(faceTransform, -center);
        return faceTransform;
    }

    // 정점에는 오른쪽부터 적용: 중심을 원점으로 이동, 회전, 원래 위치로 이동.
    faceTransform = glm::translate(faceTransform, center);
    faceTransform = glm::rotate(faceTransform, glm::radians(angle), axis);
    faceTransform = glm::translate(faceTransform, -center);
    return faceTransform;
}

void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    // 정사각형 뷰포트로 창 크기를 바꿔도 도형 비율을 유지한다.
    int size = width;
    if (height < width)
        size = height;
    glViewport((width - size) / 2, (height - size) / 2, size, size);
}

void DrawScene(GLint transformLocation, GLint colorLocation)
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (hiddenSurface)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);

    // 처음부터 x, y축으로 조금 기울어진 상태로 출력한다.
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(startAngleY + objectAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(startAngleX), glm::vec3(1.0f, 0.0f, 0.0f));
    // 공통 모서리를 한 바퀴 도는 동안에도 면이 잘리지 않는 표시 범위.
    glm::mat4 projection = glm::ortho(-1.8f, 1.8f, -1.8f, 1.8f, -3.0f, 3.0f);

    if (objectType == 0)
    {
        for (int face = 0; face < 6; face++)
        {
            if (face == 2 && backScale == 0.0f)
                continue;
            glm::mat4 faceTransform = GetFaceTransform(face);
            glm::mat4 transform = projection * model * faceTransform;
            glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(transform));
            glUniform3f(colorLocation, faceColors[face][0], faceColors[face][1], faceColors[face][2]);
            glDrawArrays(GL_TRIANGLES, face * 6, 6);
        }
    }
    else
    {
        glm::mat4 transform = projection * model;
        glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(transform));
        for (int face = 0; face < 5; face++)
        {
            int count = 3;
            if (face == 4)
                count = 6;  // 바닥만 사각형이다.
            glUniform3f(colorLocation, faceColors[face][0], faceColors[face][1], faceColors[face][2]);
            glDrawArrays(GL_TRIANGLES, 36 + face * 3, count);
        }
    }

    // 화면 안내용 좌표축은 단위 행렬로 고정하고 같은 깊이 검사를 적용한다.
    glm::mat4 identity = glm::mat4(1.0f);
    glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(identity));
    glUniform3f(colorLocation, 1.0f, 0.0f, 0.0f);
    glDrawArrays(GL_LINES, 54, 2);
    glUniform3f(colorLocation, 0.0f, 1.0f, 0.0f);
    glDrawArrays(GL_LINES, 56, 2);
    glUniform3f(colorLocation, 0.0f, 0.0f, 1.0f);
    glDrawArrays(GL_LINES, 58, 2);
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
    GLFWwindow* window = glfwCreateWindow(800, 600, "1-15 Cube / Pyramid Animation", nullptr, nullptr);
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
    std::string vertexSource = ReadShaderFile("1-15-vertex.glsl");
    std::string fragmentSource = ReadShaderFile("1-15-fragment.glsl");
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

    //--- VAO, VBO 생성: 위치 좌표만 저장하고 색은 면을 그릴 때 보낸다.
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    //--- 뷰포트와 키보드 설정
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    FramebufferSizeCallback(window, width, height);
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    glfwSetKeyCallback(window, KeyCallback);
    std::srand((unsigned int)std::time(nullptr));
    glUseProgram(shaderProgram);
    GLint transformLocation = glGetUniformLocation(shaderProgram, "transform");
    GLint colorLocation = glGetUniformLocation(shaderProgram, "faceColor");

    std::cout << "h: 은면 제거 / p: 도형 변경 / r: 리셋 / q, ESC: 종료\n";
    std::cout << "y: 전체 자전/정지 / t: 윗면 회전/정지 / f: 앞면 열기/닫기\n";
    std::cout << "s: 오른쪽 면 회전/정지 / b: 뒷면 축소/복원\n";
    std::cout << "1: 이웃한 두 면 모서리 회전 / 2: 두 면 자리 교체\n";
    std::cout << "4: 옆면 네 개 y축 공전 / 5: 위·아래·앞·뒤 x축 공전\n";
    std::cout << std::flush;

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
