// 실습 1-22: 하나의 탱크 상태를 세 카메라로 그리기.
// 이 cpp 하나로 실행한다. 19번 소스나 셰이더 파일을 참조하지 않는다.
// 읽는 순서: main -> KeyCallback/UpdateAnimation -> DrawScene -> DrawViewport.
// 모델 행렬은 세 화면에서 같고, view/projection/viewport만 달라진다.
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
#include <iostream>
#include <string>

// 셰이더도 이 소스 안에 둔다. 다른 실습의 파일이나 외부 GLSL 파일이 필요하지 않다.
// transform = projection * view * model 이므로, 정점에는 모델 변환부터 적용된다.
const char *vertexShaderSource = R"GLSL(#version 330 core
layout (location = 0) in vec3 vertexPosition;
uniform mat4 transform;

void main()
{
    gl_Position = transform * vec4(vertexPosition, 1.0);
}
)GLSL";

const char *fragmentShaderSource = R"GLSL(#version 330 core
uniform vec3 faceColor;
out vec4 fragmentColor;

void main()
{
    fragmentColor = vec4(faceColor, 1.0);
}
)GLSL";

GLuint CompileShader(GLenum type, const char *source)
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
// 정면의 +Z축에서 같은 높이의 목표점을 본다. 초기 시점에는 좌우/상하 기울기가 없다.
glm::vec3 cameraPosition(0, 2, 14);
glm::mat4 cameraRotation(1.0f);
int cameraSpin = 0;
int cameraOrbit = 0;
float aspectRatio = 800.0f / 600.0f;
std::vector<glm::vec3> vertices;
const glm::vec3 initialColors[9] = {{0.2f, 0.3f, 0.4f},    {0.35f, 0.3f, 0.45f}, {0.25f, 0.6f, 0.4f},
                                    {0.75f, 0.85f, 0.25f}, {0.25f, 0.75f, 0.8f}, {0.95f, 0.55f, 0.15f},
                                    {0.85f, 0.3f, 0.2f},   {0.85f, 0.3f, 0.7f},  {0.55f, 0.4f, 0.9f}};
glm::vec3 partColors[9];

void MakeVertices()
{
    vertices.clear();
    glm::vec3 points[8] = {{-0.5f, -0.5f, 0.5f}, {0.5f, -0.5f, 0.5f},   {0.5f, 0.5f, 0.5f},
                           {-0.5f, 0.5f, 0.5f},  {-0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, -0.5f},
                           {0.5f, 0.5f, -0.5f},  {-0.5f, 0.5f, -0.5f}};
    int faces[6][4] = {{0, 1, 2, 3}, {1, 5, 6, 2}, {5, 4, 7, 6}, {4, 0, 3, 7}, {3, 2, 6, 7}, {4, 5, 1, 0}};
    for (int face = 0; face < 6; face++)
    {
        for (int corner = 0; corner < 4; corner++)
        {
            vertices.push_back(points[faces[face][corner]]);
        }
    }
}

void ResetObjects()
{
    tankPosition = glm::vec3(0);
    animateBase = animateCenter = animateGuns = animatePoles = swapping = false;
    baseTime = centerAngle = gunTime = poleTime = 0.0f;
    swapProgress = swapStart = swapTarget = swapTime = 0.0f;
    cameraPosition = glm::vec3(0, 2, 14);
    glm::mat4 view = glm::lookAt(cameraPosition, glm::vec3(0, 2, 0), glm::vec3(0, 1, 0));
    cameraRotation = glm::mat4(glm::transpose(glm::mat3(view)));
    cameraSpin = cameraOrbit = 0;
    for (int i = 0; i < 9; i++)
    {
        partColors[i] = initialColors[i];
    }
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
    float side = 1.0f;
    if (index == 0)
    {
        side = -1.0f;
    }
    float angle = glm::radians(180.0f) * swapProgress;
    glm::vec3 position(side * 0.85f * std::cos(angle), 0.4f, side * 0.9f * std::sin(angle));
    // 중앙 몸체의 회전은 상부, 포신, 깃대에 함께 적용된다.
    return glm::translate(GetCenterTransform(), position);
}

glm::mat4 MakeBox(const glm::mat4 &parent, glm::vec3 position, glm::vec3 size)
{
    return glm::scale(glm::translate(parent, position), size);
}

// 부품 9개의 모델 행렬: 아래 몸체 2, 중앙 1, 상부 2, 포신 2, 깃대 2.
// 부모 행렬을 곱하므로 중앙 몸체가 회전하면 그 위의 부품들도 함께 움직인다.
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
        float side = -1.0f;
        float upperHeight = 0.8f;
        float upperDepth = 0.9f;
        float upperWidth = 0.9f;
        if (i == 0)
        {
            side = 1.0f;
            upperHeight = 0.65f;
            upperDepth = 1.0f;
            upperWidth = 1.0f;
        }
        glm::mat4 upper = GetUpperTransform(i);
        parts[3 + i] =
            MakeBox(upper, glm::vec3(0, upperHeight / 2, 0), glm::vec3(upperWidth, upperHeight, upperDepth));
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

void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS && action != GLFW_REPEAT)
    {
        return;
    }
    bool stepKey = key == GLFW_KEY_LEFT || key == GLFW_KEY_RIGHT || key == GLFW_KEY_UP ||
                   key == GLFW_KEY_DOWN || key == GLFW_KEY_X || key == GLFW_KEY_Z;
    if (action == GLFW_REPEAT && !stepKey)
    {
        return;
    }
    int direction = 1;
    if (mods & GLFW_MOD_SHIFT)
    {
        direction = -1;
    }
    if (key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE)
    {
        glfwSetWindowShouldClose(window, true);
    }
    else if (key == GLFW_KEY_LEFT)
    {
        tankPosition.x -= 0.2f;
    }
    else if (key == GLFW_KEY_RIGHT)
    {
        tankPosition.x += 0.2f;
    }
    else if (key == GLFW_KEY_UP)
    {
        tankPosition.z -= 0.2f;
    }
    else if (key == GLFW_KEY_DOWN)
    {
        tankPosition.z += 0.2f;
    }
    else if (key == GLFW_KEY_D)
    {
        animateBase = !animateBase;
    }
    else if (key == GLFW_KEY_T)
    {
        animateCenter = !animateCenter;
    }
    else if (key == GLFW_KEY_G)
    {
        animateGuns = !animateGuns;
    }
    else if (key == GLFW_KEY_P)
    {
        animatePoles = !animatePoles;
    }
    else if (key == GLFW_KEY_L && !swapping)
    {
        swapStart = swapProgress;
        swapTarget = 1.0f - swapTarget;
        swapTime = 0.0f;
        swapping = true;
    }
    else if (key == GLFW_KEY_Z)
    {
        // 카메라의 로컬 -Z축이 앞쪽이다. 회전 행렬로 월드 공간의 전진 방향을 구한다.
        // 방향 벡터이므로 w=0이다. z는 전진, Shift+Z는 후진하고 시선은 유지한다.
        glm::vec3 cameraForward = glm::normalize(glm::vec3(cameraRotation * glm::vec4(0, 0, -1, 0)));
        cameraPosition += cameraForward * (0.3f * direction);
    }
    else if (key == GLFW_KEY_X)
    {
        // 카메라의 로컬 +X축이 오른쪽이다. 자전/공전 후에도 화면 기준으로 좌우 이동한다.
        // x는 왼쪽, Shift+X는 오른쪽이다. 이동만 하고 cameraRotation은 바꾸지 않는다.
        glm::vec3 cameraRight = glm::normalize(glm::vec3(cameraRotation * glm::vec4(1, 0, 0, 0)));
        cameraPosition -= cameraRight * (0.3f * direction);
    }
    else if (key == GLFW_KEY_Y)
    {
        // 같은 방향 키를 다시 누르면 정지한다. 반대 방향 키는 방향을 바꾼다.
        if (cameraSpin == direction)
        {
            cameraSpin = 0;
        }
        else
        {
            cameraSpin = direction;
        }
    }
    else if (key == GLFW_KEY_R)
    {
        // 같은 방향 키를 다시 누르면 정지한다. 반대 방향 키는 방향을 바꾼다.
        if (cameraOrbit == direction)
        {
            cameraOrbit = 0;
        }
        else
        {
            cameraOrbit = direction;
        }
    }
    else if (key == GLFW_KEY_O)
    {
        for (glm::vec3 &color : partColors)
        {
            for (int channel = 0; channel < 3; channel++)
            {
                color[channel] = 0.2f + 0.75f * (float)std::rand() / RAND_MAX;
            }
        }
    }
    else if (key == GLFW_KEY_C)
    {
        ResetObjects();
    }
    float limit = floorHalfSize - tankMargin;
    tankPosition.x = std::clamp(tankPosition.x, -limit, limit);
    tankPosition.z = std::clamp(tankPosition.z, -limit, limit);
}

void UpdateAnimation(float deltaTime)
{
    if (animateBase)
    {
        baseTime = std::fmod(baseTime + deltaTime, glm::radians(180.0f));
    }
    if (animateCenter)
    {
        centerAngle = std::fmod(centerAngle + 45.0f * deltaTime, 360.0f);
    }
    if (animateGuns)
    {
        gunTime = std::fmod(gunTime + deltaTime, glm::radians(360.0f) / 1.5f);
    }
    if (animatePoles)
    {
        poleTime = std::fmod(poleTime + deltaTime, glm::radians(360.0f) / 1.5f);
    }
    if (swapping)
    {
        swapTime += deltaTime;
        float progress = std::min(swapTime / 2.0f, 1.0f);
        swapProgress = glm::mix(swapStart, swapTarget, progress);
        if (progress == 1.0f)
        {
            swapping = false;
        }
    }
    // 양의 y축 위에서 내려다볼 때 음의 각도가 시계방향이다.
    if (cameraSpin != 0)
    {
        cameraRotation =
            glm::rotate(cameraRotation, glm::radians(-35.0f * cameraSpin * deltaTime), glm::vec3(0, 1, 0));
    }
    if (cameraOrbit != 0)
    {
        glm::mat4 orbit =
            glm::rotate(glm::mat4(1), glm::radians(-25.0f * cameraOrbit * deltaTime), glm::vec3(0, 1, 0));
        cameraPosition = glm::vec3(orbit * glm::vec4(cameraPosition, 1));
        cameraRotation = orbit * cameraRotation;
    }
}

void DrawBox(const glm::mat4 &transform, glm::vec3 color, GLint transformLocation, GLint colorLocation)
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

// 같은 모델을 지정한 직사각형 안에 그린다. 여기서는 물체 상태를 갱신하지 않는다.
void DrawViewport(int x, int y, int width, int height, glm::mat4 view, bool perspective,
                  GLint transformLocation, GLint colorLocation)
{
    if (width <= 0 || height <= 0)
    {
        return;
    }
    // viewport는 NDC를 배치할 화면 영역이다.
    // scissor는 glClear까지 이 영역으로 제한하여 다른 뷰포트의 그림을 지우지 않게 한다.
    glViewport(x, y, width, height);
    glScissor(x, y, width, height);
    float backgroundRed = 0.22f;
    if (perspective)
    {
        backgroundRed = 0.15f;
    }
    glClearColor(backgroundRed, 0.24f, 0.3f, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    float aspect = float(width) / height;
    glm::mat4 projection;
    if (perspective)
    {
        projection = glm::perspective(glm::radians(50.0f), aspect, 0.1f, 100.0f);
    }
    else
    {
        // 세로로 긴 뷰포트에서도 바닥의 폭이 잘리지 않도록 표시 범위를 늘린다.
        float halfHeight = 8.0f / std::min(aspect, 1.0f);
        float halfWidth = halfHeight * aspect;
        projection = glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, 0.1f, 100.0f);
    }
    glm::mat4 viewProjection = projection * view;
    glm::mat4 floorModel = MakeBox(glm::mat4(1), glm::vec3(0, -0.1f, 0), glm::vec3(14, 0.2f, 14));
    DrawBox(viewProjection * floorModel, glm::vec3(0.65f, 0.72f, 0.78f), transformLocation, colorLocation);

    // 세 화면 모두 현재 tankPosition과 부품 각도에서 같은 모델 행렬을 얻는다.
    glm::mat4 parts[9];
    GetPartTransforms(parts);
    for (int i = 0; i < 9; i++)
    {
        glm::mat4 transform = viewProjection * parts[i];
        DrawBox(transform, partColors[i], transformLocation, colorLocation);
    }
}

void DrawScene(GLint transformLocation, GLint colorLocation)
{
    int width;
    int height;
    glfwGetFramebufferSize(glfwGetCurrentContext(), &width, &height);
    if (width < 4 || height < 4)
    {
        return;
    }
    // 화면 전체를 테두리 색으로 지운 다음 세 영역만 다시 채운다.
    glDisable(GL_SCISSOR_TEST);
    glClearColor(0.85f, 0.85f, 0.85f, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);

    int leftWidth = width * 2 / 3;
    int halfHeight = height / 2;
    int rightX = leftWidth + 1;
    int rightWidth = width - leftWidth - 1;

    // 왼쪽: 기존 탱크의 자유 카메라와 원근 투영.
    // OpenGL 뷰포트의 (0, 0)은 화면의 왼쪽 아래이다.
    glm::mat4 perspectiveView = GetViewTransform();
    DrawViewport(0, 0, leftWidth - 1, height, perspectiveView, true, transformLocation, colorLocation);

    // 오른쪽 위: +Y에서 원점을 내려다보므로 XZ 평면도이다.
    // 시선과 up이 평행하면 lookAt이 성립하지 않으므로 up을 -Z로 정한다.
    glm::mat4 topView = glm::lookAt(glm::vec3(0, 20, 0), glm::vec3(0), glm::vec3(0, 0, -1));
    DrawViewport(rightX, halfHeight + 1, rightWidth, height - halfHeight - 1, topView, false,
                 transformLocation, colorLocation);

    // 오른쪽 아래: +Z에서 -Z를 바라보므로 XY 정면도이다.
    glm::mat4 frontView = glm::lookAt(glm::vec3(0, 3, 20), glm::vec3(0, 3, 0), glm::vec3(0, 1, 0));
    DrawViewport(rightX, 0, rightWidth, halfHeight - 1, frontView, false, transformLocation, colorLocation);
    glDisable(GL_SCISSOR_TEST);
}

void FramebufferSizeCallback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
    if (width > 0 && height > 0)
    {
        aspectRatio = (float)width / height;
    }
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
    GLFWwindow *window = glfwCreateWindow(800, 600, "1-22 Three Viewports", nullptr, nullptr);
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
    GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
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
    std::cout << "x/X: 카메라 기준 왼쪽/오른쪽, z/Z: 전진/후진 (대문자: Shift)\n";
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
        {
            deltaTime = 0.1f;
        }
        // 상태 갱신은 프레임마다 한 번만 한다. 뷰포트마다 갱신하면 애니메이션이 3배 빨라진다.
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
