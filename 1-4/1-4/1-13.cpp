#include <GL/glew.h>
#include <GL/glfw3.h>
#include <GL/glm/glm.hpp>
#include <GL/glm/gtc/matrix_transform.hpp>
#include <GL/glm/gtc/type_ptr.hpp>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

// 프로젝트 속성 > 링커 > 명령줄: opengl32.lib glew32.lib glfw3.lib
// PDF 19쪽: GLM 회전, 22쪽: 셰이더의 행렬 적용, 29쪽: 깊이 검사.

// 이 두 값을 바꾸면 도형의 기울기를 바꿀 수 있다.
float angleX = 30.0f;
float angleY = 30.0f;

// 0: 육면체, 1: 사각뿔. 처음에는 육면체의 모든 면을 보여 준다.
int objectType = 0;
bool cubeVisible[6] = { true, true, true, true, true, true };
bool pyramidVisible[5] = { false, false, false, false, false };

// 좌표 3개와 색상 3개를 묶어서 저장한다.
struct Vertex
{
    float x, y, z;
    float red, green, blue;
};

// 육면체의 꼭짓점 8개. 한 변의 길이는 0.8이다.
float cubePoints[8][3] = {
    { -0.4f, -0.4f,  0.4f },  // 0: 앞쪽 왼쪽 아래
    {  0.4f, -0.4f,  0.4f },  // 1: 앞쪽 오른쪽 아래
    {  0.4f,  0.4f,  0.4f },  // 2: 앞쪽 오른쪽 위
    { -0.4f,  0.4f,  0.4f },  // 3: 앞쪽 왼쪽 위
    { -0.4f, -0.4f, -0.4f },  // 4: 뒤쪽 왼쪽 아래
    {  0.4f, -0.4f, -0.4f },  // 5: 뒤쪽 오른쪽 아래
    {  0.4f,  0.4f, -0.4f },  // 6: 뒤쪽 오른쪽 위
    { -0.4f,  0.4f, -0.4f }   // 7: 뒤쪽 왼쪽 위
};

// 사각형 한 면을 삼각형 2개로 나눈다. 숫자는 꼭짓점 번호이다.
int cubeFaces[6][6] = {
    { 0, 1, 2, 0, 2, 3 },  // 1: 앞면
    { 5, 4, 7, 5, 7, 6 },  // 2: 뒷면
    { 4, 0, 3, 4, 3, 7 },  // 3: 왼쪽 면
    { 1, 5, 6, 1, 6, 2 },  // 4: 오른쪽 면
    { 3, 2, 6, 3, 6, 7 },  // 5: 윗면
    { 4, 5, 1, 4, 1, 0 }   // 6: 아랫면
};

float cubeColors[6][3] = {
    { 1.0f, 0.2f, 0.2f },  // 빨강
    { 0.2f, 0.8f, 0.2f },  // 초록
    { 0.2f, 0.4f, 1.0f },  // 파랑
    { 1.0f, 0.8f, 0.1f },  // 노랑
    { 1.0f, 0.3f, 0.8f },  // 분홍
    { 0.1f, 0.8f, 0.9f }   // 청록
};

// 사각뿔: 바닥 꼭짓점 4개와 위쪽 꼭짓점 1개.
float pyramidPoints[5][3] = {
    { -0.4f, -0.4f,  0.4f },
    {  0.4f, -0.4f,  0.4f },
    {  0.4f, -0.4f, -0.4f },
    { -0.4f, -0.4f, -0.4f },
    {  0.0f,  0.4f,  0.0f }
};

int pyramidFaces[5][6] = {
    { 0, 1, 4, 0, 0, 0 },  // 7: 앞쪽 삼각형
    { 1, 2, 4, 0, 0, 0 },  // 8: 오른쪽 삼각형
    { 2, 3, 4, 0, 0, 0 },  // 9: 뒤쪽 삼각형
    { 3, 0, 4, 0, 0, 0 },  // 0: 왼쪽 삼각형
    { 3, 2, 1, 3, 1, 0 }   // 바닥: 삼각형 2개
};

float pyramidColors[5][3] = {
    { 1.0f, 0.2f, 0.2f },
    { 0.2f, 0.8f, 0.2f },
    { 0.2f, 0.4f, 1.0f },
    { 1.0f, 0.8f, 0.1f },
    { 1.0f, 0.3f, 0.8f }
};

// PDF의 화면 좌표계: x축은 오른쪽 위, y축은 수직, z축은 오른쪽 아래.
// 화면 안내용 선이므로 z좌표는 0이고, 도형의 회전은 적용하지 않는다.
Vertex axisVertices[6] = {
    { -1.0f, -0.3f, 0.0f, 1.0f, 0.0f, 0.0f },  // x축: 빨강
    {  1.0f,  0.3f, 0.0f, 1.0f, 0.0f, 0.0f },
    {  0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f },  // y축: 초록
    {  0.0f,  1.0f, 0.0f, 0.0f, 1.0f, 0.0f },
    { -0.55f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f },  // z축 표시: 파랑
    {  0.55f,-0.5f, 0.0f, 0.0f, 0.0f, 1.0f }
};

// 실행할 때의 작업 폴더에서 먼저 찾고, 없으면 소스 파일 옆에서 찾는다.
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

    // 한 줄씩 읽고 줄바꿈을 붙여서 셰이더 코드 전체를 만든다.
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

// 새 명령을 입력하면 이전에 선택한 면을 모두 지운다.
void ClearFaces()
{
    for (int i = 0; i < 6; i++)
        cubeVisible[i] = false;
    for (int i = 0; i < 5; i++)
        pyramidVisible[i] = false;
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // 키를 누른 순간에만 처리한다. 누르고 있어도 랜덤 선택을 반복하지 않는다.
    if (action != GLFW_PRESS)
        return;
    if (key == GLFW_KEY_ESCAPE)
    {
        glfwSetWindowShouldClose(window, true);
    }
    else if (key >= GLFW_KEY_1 && key <= GLFW_KEY_6)
    {
        ClearFaces();
        objectType = 0;
        cubeVisible[key - GLFW_KEY_1] = true;
    }
    else if (key >= GLFW_KEY_7 && key <= GLFW_KEY_9)
    {
        ClearFaces();
        objectType = 1;
        pyramidVisible[key - GLFW_KEY_7] = true;
    }
    else if (key == GLFW_KEY_0)
    {
        ClearFaces();
        objectType = 1;
        pyramidVisible[3] = true;
    }
    else if (key == GLFW_KEY_C)
    {
        ClearFaces();
        objectType = 0;
        int first = std::rand() % 6;
        int second = std::rand() % 6;
        while (first == second)
            second = std::rand() % 6;
        cubeVisible[first] = true;
        cubeVisible[second] = true;
    }
    else if (key == GLFW_KEY_T)
    {
        ClearFaces();
        objectType = 1;
        int side = std::rand() % 4;
        pyramidVisible[side] = true;
        pyramidVisible[4] = true;
    }
}

// 정사각형 뷰포트로 비율을 유지한다. 창 크기가 바뀌어도 도형은 중앙에 있다.
void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
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
    GLFWwindow* window = glfwCreateWindow(800, 600, "1-13 Cube / Pyramid", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "윈도우 생성 실패!" << std::endl;
        glfwTerminate();
        return -1;
    }
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

    //--- 셰이더 생성
    std::string vertexSource = ReadShaderFile("1-13-vertex.glsl");
    std::string fragmentSource = ReadShaderFile("1-13-fragment.glsl");
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

    //--- GPU에 보낼 정점 배열 만들기
    // 육면체 36개 + 사각뿔 18개 + 좌표축 6개 = 60개.
    Vertex vertices[60];
    int vertexCount = 0;
    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < 6; i++)
        {
            int point = cubeFaces[face][i];
            vertices[vertexCount] = {
                cubePoints[point][0], cubePoints[point][1], cubePoints[point][2],
                cubeColors[face][0], cubeColors[face][1], cubeColors[face][2]
            };
            vertexCount++;
        }
    }
    for (int face = 0; face < 5; face++)
    {
        int count = 3;
        if (face == 4)
            count = 6;  // 바닥만 삼각형 2개이다.
        for (int i = 0; i < count; i++)
        {
            int point = pyramidFaces[face][i];
            vertices[vertexCount] = {
                pyramidPoints[point][0], pyramidPoints[point][1], pyramidPoints[point][2],
                pyramidColors[face][0], pyramidColors[face][1], pyramidColors[face][2]
            };
            vertexCount++;
        }
    }
    for (int i = 0; i < 6; i++)
    {
        vertices[vertexCount] = axisVertices[i];
        vertexCount++;
    }

    //--- VAO: 정점 읽는 방법, VBO: 실제 정점 데이터
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
    std::srand((unsigned int)std::time(nullptr));

    //--- 은면 제거: 겹친 면 중 카메라에 가까운 면이 보인다.
    // 일부 면만 그리므로 뒷면 컬링은 사용하지 않는다.
    glEnable(GL_DEPTH_TEST);
    glUseProgram(shaderProgram);
    GLint transformLocation = glGetUniformLocation(shaderProgram, "transform");

    std::cout << "1~6: 육면체 앞/뒤/왼쪽/오른쪽/위/아래 면\n";
    std::cout << "7/8/9/0: 사각뿔 앞/오른쪽/뒤/왼쪽 삼각형 면\n";
    std::cout << "c: 육면체의 서로 다른 두 면, t: 사각뿔 바닥과 옆면 하나\n";
    std::cout << "ESC: 종료. 각 명령은 이전 면 선택을 교체합니다.\n";

    //--- 메인 루프
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 단위 행렬에서 시작한다. 정점에는 x축 회전, y축 회전 순서로 적용된다.
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(angleX), glm::vec3(1.0f, 0.0f, 0.0f));
        // 투영 행렬 없이 회전 행렬만 전달한다.
        glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(model));
        glBindVertexArray(vao);

        if (objectType == 0)
        {
            for (int face = 0; face < 6; face++)
            {
                if (cubeVisible[face])
                    glDrawArrays(GL_TRIANGLES, face * 6, 6);
            }
        }
        else
        {
            for (int face = 0; face < 4; face++)
            {
                if (pyramidVisible[face])
                    glDrawArrays(GL_TRIANGLES, 36 + face * 3, 3);
            }
            if (pyramidVisible[4])
                glDrawArrays(GL_TRIANGLES, 48, 6);
        }

        // 좌표축은 화면에 고정하고, 도형과 같은 깊이 검사를 적용한다.
        glm::mat4 identity = glm::mat4(1.0f);
        glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(identity));
        glDrawArrays(GL_LINES, 54, 6);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    //--- 종료 처리
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
