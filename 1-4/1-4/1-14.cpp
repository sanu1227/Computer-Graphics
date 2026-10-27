#include <GL/glew.h>
#include <GL/glfw3.h>
#include <GL/glm/glm.hpp>
#include <GL/glm/gtc/matrix_transform.hpp>
#include <GL/glm/gtc/type_ptr.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

// 프로젝트 속성 > 링커 > 명령줄: opengl32.lib glew32.lib glfw3.lib


// 이 값을 변경하면 시작할 때와 s를 누를 때의 회전 각도가 바뀐다.
const float startAngleX = 30.0f;
const float startAngleY = 30.0f;
const float rotationSpeed = 60.0f;  // 1초에 60도 회전한다.
const float moveStep = 0.1f;

int objectType = 0;       // 0: 육면체, 1: 사각뿔
bool hiddenSurface = true;
bool wireframe = false;
float positionX = 0.0f;
float positionY = 0.0f;
float angleX = startAngleX;
float angleY = startAngleY;
int rotationX = 0;        // 0: 정지, 1: 양의 방향, -1: 음의 방향
int rotationY = 0;

// 정점 하나에 위치와 색상을 함께 저장한다.
struct Vertex
{
    float x, y, z;
    float red, green, blue;
};

// 육면체 꼭짓점 8개. 같은 꼭짓점은 어느 면에서도 같은 색이다.
Vertex cubePoints[8] = {
    { -0.4f, -0.4f,  0.4f, 1.0f, 0.0f, 0.0f },  // 0: 앞 왼쪽 아래, 빨강
    {  0.4f, -0.4f,  0.4f, 1.0f, 0.0f, 0.0f },  // 1: 앞 오른쪽 아래, 초록
    {  0.4f,  0.4f,  0.4f, 1.0f, 0.0f, 0.0f },  // 2: 앞 오른쪽 위, 파랑
    { -0.4f,  0.4f,  0.4f, 1.0f, 0.0f, 0.0f },  // 3: 앞 왼쪽 위, 노랑
    { -0.4f, -0.4f, -0.4f, 0.0f, 0.0f, 1.0f },  // 4: 뒤 왼쪽 아래, 자홍
    {  0.4f, -0.4f, -0.4f, 0.0f, 0.0f, 1.0f },  // 5: 뒤 오른쪽 아래, 청록
    {  0.4f,  0.4f, -0.4f, 0.0f, 0.0f, 1.0f },  // 6: 뒤 오른쪽 위, 주황
    { -0.4f,  0.4f, -0.4f, 0.0f, 0.0f, 1.0f }   // 7: 뒤 왼쪽 위, 보라
};

// 사각형 한 면은 삼각형 2개로 나눈다. 숫자는 위의 꼭짓점 번호이다.
int cubeTriangles[36] = {
    0, 1, 2, 0, 2, 3,  // 앞면
    5, 4, 7, 5, 7, 6,  // 뒷면
    4, 0, 3, 4, 3, 7,  // 왼쪽 면
    1, 5, 6, 1, 6, 2,  // 오른쪽 면
    3, 2, 6, 3, 6, 7,  // 윗면
    4, 5, 1, 4, 1, 0   // 아랫면
};

// 모서리 하나는 꼭짓점 2개로 만든다. 면을 나눈 대각선은 그리지 않는다.
int cubeEdges[24] = {
    0, 1, 1, 2, 2, 3, 3, 0,  // 앞쪽 모서리 4개
    4, 5, 5, 6, 6, 7, 7, 4,  // 뒤쪽 모서리 4개
    0, 4, 1, 5, 2, 6, 3, 7   // 앞뒤를 잇는 모서리 4개
};

// 사각뿔은 사각형 바닥의 꼭짓점 4개와 위쪽 꼭짓점 1개를 사용한다.
Vertex pyramidPoints[5] = {
    { -0.4f, -0.4f,  0.4f, 1.0f, 0.0f, 0.0f },
    {  0.4f, -0.4f,  0.4f, 0.0f, 1.0f, 0.0f },
    {  0.4f, -0.4f, -0.4f, 0.0f, 0.0f, 1.0f },
    { -0.4f, -0.4f, -0.4f, 1.0f, 1.0f, 0.0f },
    {  0.0f,  0.4f,  0.0f, 1.0f, 0.0f, 1.0f }
};

int pyramidTriangles[18] = {
    0, 1, 4,           // 앞쪽 삼각형
    1, 2, 4,           // 오른쪽 삼각형
    2, 3, 4,           // 뒤쪽 삼각형
    3, 0, 4,           // 왼쪽 삼각형
    3, 2, 1, 3, 1, 0  // 사각형 바닥: 삼각형 2개
};

int pyramidEdges[16] = {
    0, 1, 1, 2, 2, 3, 3, 0,  // 바닥 모서리 4개
    0, 4, 1, 4, 2, 4, 3, 4   // 꼭대기로 향하는 모서리 4개
};

// PDF의 화면 좌표계: x축은 오른쪽 위, y축은 수직, z축은 오른쪽 아래.
// 화면 안내용 선이므로 z좌표는 0이다.
// 좌표축에는 도형의 이동이나 회전 행렬을 적용하지 않는다.
Vertex axisVertices[6] = {
    { -1.0f, -0.3f, 0.0f, 1.0f, 0.0f, 0.0f },  // x축: 빨강
    {  1.0f,  0.3f, 0.0f, 1.0f, 0.0f, 0.0f },
    {  0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f },  // y축: 초록
    {  0.0f,  1.0f, 0.0f, 0.0f, 1.0f, 0.0f },
    { -0.55f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f },  // z축 표시: 파랑
    {  0.55f,-0.5f, 0.0f, 0.0f, 0.0f, 1.0f }
};

// 배열 안의 시작 위치와 개수. glDrawArrays에서 사용한다.
const int cubeFaceStart = 0;
const int cubeFaceCount = 36;
const int pyramidFaceStart = 36;
const int pyramidFaceCount = 18;
const int cubeEdgeStart = 54;
const int cubeEdgeCount = 24;
const int pyramidEdgeStart = 78;
const int pyramidEdgeCount = 16;
const int axisStart = 94;
Vertex vertices[100];

void CreateVertices()
{
    int count = 0;
    for (int i = 0; i < cubeFaceCount; i++)
        vertices[count++] = cubePoints[cubeTriangles[i]];
    for (int i = 0; i < pyramidFaceCount; i++)
        vertices[count++] = pyramidPoints[pyramidTriangles[i]];
    for (int i = 0; i < cubeEdgeCount; i++)
        vertices[count++] = cubePoints[cubeEdges[i]];
    for (int i = 0; i < pyramidEdgeCount; i++)
        vertices[count++] = pyramidPoints[pyramidEdges[i]];
    for (int i = 0; i < 6; i++)
        vertices[count++] = axisVertices[i];
}

// 작업 폴더에서 먼저 찾고, 없으면 소스 파일 옆에서 찾는다.
// 프로젝트의 /FC 옵션은 __FILE__에 소스 파일의 전체 경로를 저장한다.
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
        source += line + "\n";
    if (file.bad() || source.empty())
    {
        std::cerr << "셰이더 파일이 비어 있거나 읽기에 실패했습니다: " << fileName << std::endl;
        return "";
    }
    return source;
}

GLuint CompileShader(GLenum type, const char* source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    GLint success = GL_FALSE;
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

void ResetObject()
{
    positionX = 0.0f;
    positionY = 0.0f;
    angleX = startAngleX;
    angleY = startAngleY;
    rotationX = 0;
    rotationY = 0;
    // 선택한 도형, 은면 제거, 와이어/솔리드 설정은 유지한다.
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS && action != GLFW_REPEAT)
        return;

    // 방향키는 누르고 있으면 계속 이동한다.
    if (key == GLFW_KEY_LEFT)
        positionX -= moveStep;
    else if (key == GLFW_KEY_RIGHT)
        positionX += moveStep;
    else if (key == GLFW_KEY_UP)
        positionY += moveStep;
    else if (key == GLFW_KEY_DOWN)
        positionY -= moveStep;
    else if (action == GLFW_PRESS)
    {
        // GLFW의 문자 키는 대소문자가 같은 key 값이다. Shift로 구분한다.
        bool shiftPressed = (mods & GLFW_MOD_SHIFT) != 0;
        if (key == GLFW_KEY_ESCAPE)
            glfwSetWindowShouldClose(window, true);
        else if (key == GLFW_KEY_C)
            objectType = 0;
        else if (key == GLFW_KEY_P)
            objectType = 1;
        else if (key == GLFW_KEY_H)
            hiddenSurface = !hiddenSurface;
        else if (key == GLFW_KEY_W)
            wireframe = !shiftPressed;  // w: 와이어, Shift+w: 솔리드
        else if (key == GLFW_KEY_X)
        {
            rotationY = 0;
            rotationX = 1;
            if (shiftPressed)
                rotationX = -1;
        }
        else if (key == GLFW_KEY_Y)
        {
            rotationX = 0;
            rotationY = 1;
            if (shiftPressed)
                rotationY = -1;
        }
        else if (key == GLFW_KEY_S)
            ResetObject();
    }
}

void UpdateAnimation(float deltaTime)
{
    angleX += rotationX * rotationSpeed * deltaTime;
    angleY += rotationY * rotationSpeed * deltaTime;
    // 각도가 계속 커지지 않도록 한 바퀴를 넘으면 360도를 빼거나 더한다.
    if (angleX >= 360.0f)
        angleX -= 360.0f;
    else if (angleX < 0.0f)
        angleX += 360.0f;
    if (angleY >= 360.0f)
        angleY -= 360.0f;
    else if (angleY < 0.0f)
        angleY += 360.0f;
}

void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    // 정사각형 뷰포트로 도형 비율을 유지하고 화면 중앙에 배치한다.
    int size = width;
    if (height < width)
        size = height;
    glViewport((width - size) / 2, (height - size) / 2, size, size);
}

void DrawScene(GLint transformLocation)
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (hiddenSurface)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);

    // 이동 행렬을 먼저 만들면 정점에는 회전 후 이동이 적용된다.
    // 도형을 옮긴 뒤에도 도형 자신의 중심에서 회전한다(자전).
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(positionX, positionY, 0.0f));
    model = glm::rotate(model, glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(angleX), glm::vec3(1.0f, 0.0f, 0.0f));
    // 투영 행렬 없이 이동과 회전 행렬만 전달한다.
    glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(model));

    int faceStart = cubeFaceStart;
    int faceCount = cubeFaceCount;
    int edgeStart = cubeEdgeStart;
    int edgeCount = cubeEdgeCount;
    if (objectType == 1)
    {
        faceStart = pyramidFaceStart;
        faceCount = pyramidFaceCount;
        edgeStart = pyramidEdgeStart;
        edgeCount = pyramidEdgeCount;
    }

    if (!wireframe)
        glDrawArrays(GL_TRIANGLES, faceStart, faceCount);
    else
        glDrawArrays(GL_LINES, edgeStart, edgeCount);

    // 화면 좌표축은 도형의 변환 없이 고정된 위치에 그린다.
    // h로 설정한 깊이 검사를 좌표축에도 그대로 적용한다.
    glm::mat4 identity = glm::mat4(1.0f);
    glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(identity));
    glDrawArrays(GL_LINES, axisStart, 6);
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
    GLFWwindow* window = glfwCreateWindow(800, 600, "1-14 Cube / Pyramid", nullptr, nullptr);
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
    std::string vertexSource = ReadShaderFile("1-14-vertex.glsl");
    std::string fragmentSource = ReadShaderFile("1-14-fragment.glsl");
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
    GLint success = GL_FALSE;
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

    //--- 정점 데이터와 VAO, VBO 생성
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

    //--- 뷰포트와 키보드 설정
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    FramebufferSizeCallback(window, width, height);
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    glfwSetKeyCallback(window, KeyCallback);
    glUseProgram(shaderProgram);
    GLint transformLocation = glGetUniformLocation(shaderProgram, "transform");

    std::cout << "c: 육면체 / p: 사각뿔 / h: 은면 제거 적용 또는 해제\n";
    std::cout << "w: 와이어 / Shift+w(W): 솔리드\n";
    std::cout << "x, y: 양의 방향 자전 / Shift+x(X), Shift+y(Y): 음의 방향 자전\n";
    std::cout << "방향키: 이동 / s: 위치와 각도 초기화, 모든 자전 정지 / ESC: 종료\n";
    std::cout << "고정 좌표축: x=빨강, y=초록, z=파랑(대각선 표시)\n";

    //--- 메인 루프
    double previousTime = glfwGetTime();
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        double currentTime = glfwGetTime();
        float deltaTime = (float)(currentTime - previousTime);
        previousTime = currentTime;
        // 창 이동이나 디버깅으로 오래 멈췄을 때 갑자기 크게 회전하지 않게 한다.
        if (deltaTime > 0.1f)
            deltaTime = 0.1f;
        UpdateAnimation(deltaTime);
        DrawScene(transformLocation);
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
