// 실습 1-23: Walking Robot
// 이 cpp 하나를 빌드한다. GLFW/GLEW/GLM과 OpenGL 라이브러리는 기존 프로젝트 설정을 사용한다.
// 읽는 순서: main -> ResetObjects -> KeyCallback -> UpdateAnimation -> DrawScene.
// 모델 좌표에 model, 카메라에 view, 화면 투영에 projection 행렬을 적용한다.
#include <GL/glew.h>
#include <GL/glfw3.h>
#include <GL/glm/glm.hpp>
#include <GL/glm/gtc/matrix_transform.hpp>
#include <GL/glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>
#include <random>

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

std::vector<glm::vec3> vertices;
// 원점 중심, 한 변 길이 1인 육면체. 면 순서는 앞/오른쪽/뒤/왼쪽/위/아래이다.
// 각 면의 정점을 바깥에서 볼 때 반시계 방향으로 저장한다.
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

glm::mat4 MakeBox(const glm::mat4 &parent, glm::vec3 position, glm::vec3 size)
{

    // M = 부모 행렬 * 이동 * 크기 조절. 정점에는 오른쪽의 크기 조절부터 적용된다.
    glm::mat4 translation = glm::translate(glm::mat4(1), position);
    glm::mat4 scaling = glm::scale(glm::mat4(1), size);
    glm::mat4 model = parent * translation * scaling;
    return model;
}

// 한 면은 4개 정점의 TRIANGLE_FAN이다. 총 6개 면을 색 농도를 달리하여 그린다.
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

// 화면 크기와 셰이더의 uniform 위치. viewProjection에는 투영 * 뷰 행렬을 저장한다.
int screenWidth = 1000;
int screenHeight = 750;
GLint transformLocation; // 정점 셰이더의 transform 위치
GLint colorLocation;     // 프래그먼트 셰이더의 faceColor 위치
glm::mat4 viewProjection(1);
const glm::vec3 palette[] = {{0.9f, 0.3f, 0.25f}, {0.25f, 0.65f, 0.95f}, {0.95f, 0.7f, 0.15f},
                             {0.35f, 0.8f, 0.4f}, {0.7f, 0.35f, 0.85f},  {0.4f, 0.75f, 0.75f}};
glm::mat4 RotationMatrix(float degrees, glm::vec3 axis)
{
    return glm::rotate(glm::mat4(1), glm::radians(degrees), axis);
}

void DrawBoxAt(glm::mat4 parent, glm::vec3 position, glm::vec3 size, glm::vec3 color)
{

    glm::mat4 model = MakeBox(parent, position, size);
    glm::mat4 transform = viewProjection * model;
    DrawBox(transform, color, transformLocation, colorLocation);
}

// 육면체의 한 면만 그린다. 크기 중 하나를 0으로 두면 두께 없는 평면이 된다.
void DrawFace(glm::mat4 model, int face, glm::vec3 color)
{
    glm::mat4 transform = viewProjection * model;
    glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(transform));
    glUniform3fv(colorLocation, 1, glm::value_ptr(color));
    glDrawArrays(GL_TRIANGLE_FAN, face * 4, 4);
}

void BeginFrame()
{
    glViewport(0, 0, screenWidth, screenHeight);
    glClearColor(0.12f, 0.15f, 0.2f, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
}

// ---------------- 실습 상태와 동작 ----------------

struct Obstacle
{
    glm::vec3 position; // 육면체의 중심. 바닥 위에 놓으므로 y는 높이의 절반이다.
    glm::vec3 size;     // x: 폭, y: 높이, z: 깊이
};
Obstacle obstacles[3];
glm::vec3 robot(0);            // 로봇 발밑의 월드 좌표
glm::vec3 direction(0, 0, -1); // 걷는 방향(단위 벡터)
glm::vec3 camera(0, 2, 20);    // 초기 카메라는 무대 정면에 둔다.
glm::vec3 cameraDirection(0, 0, -1); // 시선 방향. X/Z로 이동할 때는 이 방향을 유지한다.
float walkSpeed = 1.4f;        // 초당 이동 거리
float gait = 0;                // 팔/다리 왕복 운동의 진행 각도(라디안)
float verticalSpeed = 0;       // 점프 속도. 위쪽이 양수이다.
float door = 1;                // 앞문 열림 진행률: 0은 닫힘, 1은 열림. 시작부터 내부를 보여 준다.
bool walking = true;
bool jumping = false; // 점프 중에는 제자리에 머물고, 착지하면 걷기를 다시 시작한다.
bool doorOpen = true;
constexpr float robotRadius = 0.65f;

// XZ 평면에서 로봇의 충돌 영역과 장애물 영역이 겹치는지 검사한다.
// 로봇은 반너비 robotRadius인 사각 영역으로 근사한다. 실제 팔/다리 메시 충돌은 아니다.
bool Overlaps(glm::vec3 position, const Obstacle &obstacle)
{
    return std::abs(position.x - obstacle.position.x) < obstacle.size.x / 2 + robotRadius &&
           std::abs(position.z - obstacle.position.z) < obstacle.size.z / 2 + robotRadius;
}

// 초기값을 한곳에서 복원한다. 시작할 때와 리셋 키를 눌렀을 때 호출한다.
void ResetObjects()
{
    robot = glm::vec3(0);
    direction = {0, 0, -1};
    camera = {0, 2, 20};
    cameraDirection = {0, 0, -1};
    walkSpeed = 1.4f;
    gait = 0;
    verticalSpeed = 0;
    door = 1;
    walking = true;
    jumping = false;
    doorOpen = true;
    // 서로 분리된 세 구역에서 크기와 위치를 무작위로 선택한다.
    static std::mt19937 random(std::random_device{}());
    std::uniform_real_distribution<float> offset(-0.4f, 0.4f), size(0.8f, 1.5f);
    const glm::vec3 centers[] = {{-3, 0, -2.5f}, {3, 0, -2.5f}, {-2, 0, 3}};
    for (int i = 0; i < 3; i++)
    {
        obstacles[i].size = {size(random), 0.7f + 0.2f * i, size(random)};
        obstacles[i].position =
            centers[i] + glm::vec3(offset(random), obstacles[i].size.y / 2, offset(random));
    }
}

void UpdateAnimation(float deltaTime)
{
    int stepCount = std::max(1, int(std::ceil(deltaTime / 0.005f)));
    float stepTime = deltaTime / stepCount;
    for (int step = 0; step < stepCount; step++)
    {
        // door는 닫힘 0부터 열림 1까지의 진행률이다.
        float targetDoor = 0.0f;
        if (doorOpen)
        {
            targetDoor = 1.0f;
        }
        door += std::clamp(targetDoor - door, -stepTime, stepTime);

        // robot.y는 몸통 중심이 아니라 발밑 기준 높이이다.
        // 이전 높이를 보관해야 장애물 윗면을 위에서 내려오며 통과했는지 판단할 수 있다.
        float previousHeight = robot.y;
        verticalSpeed -= 9.8f * stepTime;
        robot.y += verticalSpeed * stepTime;

        // 우선 다음 위치를 계산하고, 충돌하지 않을 때만 XZ 위치를 실제로 바꾼다.
        glm::vec3 nextPosition = robot;
        if (walking)
        {
            glm::vec3 movement = direction * walkSpeed * stepTime;
            nextPosition += movement;
        }
        float movementLimit = 6.0f - robotRadius;
        bool blocked = std::abs(nextPosition.x) > movementLimit || std::abs(nextPosition.z) > movementLimit;
        for (const Obstacle &obstacle : obstacles)
        {
            float obstacleTop = obstacle.size.y;
            bool overlapsXZ = Overlaps(nextPosition, obstacle);
            bool belowTop = robot.y < obstacleTop - 0.01f;
            bool wasBelowTop = previousHeight < obstacleTop - 0.01f;
            // 장애물보다 낮은 상태에서 옆으로 진입하면 막는다.
            // 위쪽에서 내려오는 경우는 아래의 착지 계산으로 처리한다.
            if (overlapsXZ && belowTop && wasBelowTop)
            {
                blocked = true;
            }
        }
        if (blocked)
        {
            direction = -direction; // 벽이나 장애물에 닿으면 뒤로 돌아 걷는다.
        }
        else
        {
            robot.x = nextPosition.x;
            robot.z = nextPosition.z;
        }

        // 발 아래의 지지면 높이를 찾는다. 기본 바닥은 0, 장애물 위는 장애물 높이이다.
        float supportHeight = 0.0f;
        for (const Obstacle &obstacle : obstacles)
        {
            bool overlapsXZ = Overlaps(robot, obstacle);
            bool wasAboveTop = previousHeight >= obstacle.size.y - 0.01f;
            if (overlapsXZ && wasAboveTop)
            {
                supportHeight = std::max(supportHeight, obstacle.size.y);
            }
        }
        if (robot.y <= supportHeight && verticalSpeed <= 0)
        {
            robot.y = supportHeight;
            verticalSpeed = 0; // 착지하면 아래쪽 속도를 없앤다.
            if (jumping)
            {
                jumping = false;
                walking = true; // 바닥이나 장애물 위에 착지하면 현재 방향으로 다시 걷는다.
            }
        }
        // sin(gait)를 팔/다리 각도로 사용한다. 이동 속도가 빠르면 보행 주기도 빨라진다.
        if (walking)
        {
            gait += walkSpeed * stepTime * 4;
        }
    }
}

void DrawRobot()
{
    // 모델의 앞쪽은 +Z이다. 이동 방향의 X/Z 성분으로 몸의 Y축 회전각을 구한다.
    float yaw = glm::degrees(std::atan2(direction.x, direction.z));
    glm::mat4 translation = glm::translate(glm::mat4(1), robot);
    glm::mat4 rotation = RotationMatrix(yaw, glm::vec3(0, 1, 0));
    glm::mat4 root = translation * rotation;

    float swing = 0.0f;
    if (walking)
    {
        float amplitude = std::min(50.0f, walkSpeed * 18);
        swing = std::sin(gait) * amplitude;
    }
    // 몸통, 머리, 코는 로봇 전체 행렬 root를 공유한다.
    DrawBoxAt(root, {0, 1.55f, 0}, {0.8f, 1, 0.5f}, {0.25f, 0.6f, 0.8f});
    DrawBoxAt(root, {0, 2.35f, 0}, {0.65f, 0.6f, 0.6f}, {0.95f, 0.75f, 0.45f});
    DrawBoxAt(root, {0, 2.3f, 0.39f}, {0.17f, 0.17f, 0.22f}, {1, 0.35f, 0.2f});
    for (int i = 0; i < 2; i++)
    {
        float side = 1.0f;
        if (i == 0)
        {
            side = -1.0f;
        }
        // 어깨로 이동한 뒤 회전하고, 팔 중심을 어깨 아래로 내려 그린다.
        // 따라서 팔은 자기 중심이 아니라 어깨 관절을 중심으로 흔들린다.
        glm::mat4 arm =
            glm::translate(root, {side * 0.58f, 1.95f, 0}) * RotationMatrix(side * swing, {1, 0, 0});
        DrawBoxAt(arm, {0, -0.4f, 0}, {0.25f, 0.85f, 0.28f}, palette[i]);
        // 다리는 골반 관절을 기준으로 회전한다. 같은 쪽 팔과 반대 각도를 적용한다.
        glm::mat4 leg =
            glm::translate(root, {side * 0.23f, 1.05f, 0}) * RotationMatrix(-side * swing, {1, 0, 0});
        DrawBoxAt(leg, {0, -0.45f, 0}, {0.3f, 0.9f, 0.35f}, palette[i + 3]);
        DrawBoxAt(leg, {0, -0.92f, 0.08f}, {0.32f, 0.16f, 0.5f}, palette[i + 3] * 0.65f);
    }
}

// 앞문의 윗모서리(y=8, z=6)를 경첩 축으로 삼는다.
glm::mat4 GetDoorTransform()
{
    // 열림 진행률이 0에서 1로 바뀔 때 높이 스케일은 1에서 0으로 줄어든다.
    float heightScale = 1.0f - door;
    float doorHeight = 8.0f * heightScale;
    glm::mat4 size = glm::scale(glm::mat4(1), glm::vec3(12, doorHeight, 0));
    // 높이가 변해도 윗모서리는 경첩에 붙어 있도록 중심 위치도 높이의 절반만큼 보정한다.
    glm::mat4 centerOffset = glm::translate(glm::mat4(1), glm::vec3(0, -doorHeight / 2.0f, 0));
    // X축의 음의 방향으로 회전하면 아래쪽 끝이 무대 바깥(+Z)으로 들린다.
    // 닫힘은 0도, 열림은 90도이다. 회전과 높이 축소를 동시에 적용한다.
    float angle = -90.0f * door;
    glm::mat4 rotation = RotationMatrix(angle, glm::vec3(1, 0, 0));
    glm::mat4 hingePosition = glm::translate(glm::mat4(1), glm::vec3(0, 8, 6));
    // 정점에는 오른쪽부터 크기 설정, 중심 보정, 경첩 회전, 경첩의 월드 위치 이동을 적용한다.
    return hingePosition * rotation * centerOffset * size;
}

void DrawScene()
{
    BeginFrame();
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glm::mat4 projection =
        glm::perspective(glm::radians(48.0f), float(screenWidth) / screenHeight, 0.1f, 100.0f);
    // 목표점을 원점에 고정하면 옆으로 이동할 때 시선도 회전한다.
    // 카메라 위치에 시선 방향을 더해 목표점을 정하면 이동 전후 같은 방향을 바라본다.
    glm::vec3 cameraTarget = camera + cameraDirection;
    glm::mat4 view = glm::lookAt(camera, cameraTarget, glm::vec3(0, 1, 0));
    viewProjection = projection * view;
    // 무대 벽은 안쪽을 앞면으로 지정한다. 가까운 벽의 바깥면은 컬링되어 내부가 보인다.
    glFrontFace(GL_CW);
    glm::mat4 stage = MakeBox(glm::mat4(1), {0, 4, 0}, {12, 8, 12});
    for (int face = 1; face < 6; face++)
    {
        DrawFace(stage, face, palette[face] * 0.65f);
    }
    // 윗모서리는 고정되고 아래쪽 끝이 원호를 그리며 위로 젖혀진다.
    glm::mat4 doorModel = GetDoorTransform();
    // 앞문만 양면으로 그려 열리는 동안 문의 안쪽과 바깥쪽이 모두 보이게 한다.
    glDisable(GL_CULL_FACE);
    DrawFace(doorModel, 0, palette[0] * 0.65f);
    glEnable(GL_CULL_FACE);
    // 장애물과 로봇은 일반적인 바깥쪽 면을 그려야 하므로 CCW로 복구한다.
    glFrontFace(GL_CCW);
    for (int i = 0; i < 3; i++)
    {
        DrawBoxAt(glm::mat4(1), obstacles[i].position, obstacles[i].size, palette[i + 2]);
    }
    DrawRobot();
}

// GLFW는 키를 누를 때, 놓을 때, 길게 누를 때 이 함수를 호출한다.
// 대문자는 Shift로 구분한다. 토글은 PRESS에서만 처리하여 길게 눌러도 한 번만 바뀐다.
void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS && action != GLFW_REPEAT)
    {
        return;
    }
    if (key == GLFW_KEY_W)
    {
        direction = {0, 0, -1};
        walking = true;
    }
    if (key == GLFW_KEY_S)
    {
        direction = {0, 0, 1};
        walking = true;
    }
    if (key == GLFW_KEY_A)
    {
        direction = {-1, 0, 0};
        walking = true;
    }
    if (key == GLFW_KEY_D)
    {
        direction = {1, 0, 0};
        walking = true;
    }
    if (key == GLFW_KEY_EQUAL || key == GLFW_KEY_KP_ADD)
    {
        walkSpeed = std::min(2.7f, walkSpeed + 0.2f);
    }
    if (key == GLFW_KEY_MINUS || key == GLFW_KEY_KP_SUBTRACT)
    {
        walkSpeed = std::max(0.2f, walkSpeed - 0.2f);
    }
    // J만 누르면 수직 점프한 뒤 착지 시 다시 걷는다. 공중 WASD로 장애물 위로 이동할 수도 있다.
    if (key == GLFW_KEY_J && action == GLFW_PRESS && !jumping && verticalSpeed == 0)
    {
        verticalSpeed = 6.5f;
        walking = false;
        jumping = true;
    }
    if (key == GLFW_KEY_O && action == GLFW_PRESS)
    {
        doorOpen = !doorOpen;
    }
    if (key == GLFW_KEY_I)
    {
        ResetObjects();
    }
    float sign = 1.0f;
    if (mods & GLFW_MOD_SHIFT)
    {
        sign = -1.0f;
    }
    if (key == GLFW_KEY_Z)
    {
        // z: 바라보는 방향으로 전진. Shift+Z: 같은 방향을 바라보며 후진.
        camera += cameraDirection * sign * 0.4f;
    }
    if (key == GLFW_KEY_X)
    {
        // 시선과 위쪽 방향의 외적으로 카메라 기준 오른쪽 방향을 구한다.
        // x: 왼쪽으로 이동. Shift+X: 오른쪽으로 이동. 시선은 바꾸지 않는다.
        glm::vec3 cameraRight = glm::normalize(glm::cross(cameraDirection, glm::vec3(0, 1, 0)));
        camera -= cameraRight * sign * 0.4f;
    }
    if (key == GLFW_KEY_Y)
    {
        // 공전할 때만 카메라 위치와 시선을 함께 회전한다.
        glm::mat4 orbit = RotationMatrix(-sign * 5, glm::vec3(0, 1, 0));
        camera = glm::vec3(orbit * glm::vec4(camera, 1));
        cameraDirection = glm::normalize(glm::vec3(orbit * glm::vec4(cameraDirection, 0)));
    }
    if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q)
    {
        glfwSetWindowShouldClose(window, true);
    }
}

int main()
{
    // 1. GLFW로 창과 OpenGL 3.3 Core 컨텍스트를 만든다.
    if (!glfwInit())
    {
        std::cerr << "GLFW 초기화 실패\n";
        return 1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    GLFWwindow *window = glfwCreateWindow(screenWidth, screenHeight, "1-23 Walking Robot", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);

    // 2. GLEW를 초기화한 뒤에 glCreateShader 같은 OpenGL 함수를 사용할 수 있다.
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "GLEW 초기화 실패\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }
    glGetError(); // 일부 환경에서 GLEW 초기화 중 남긴 오류를 소비한다.

    // 3. 정점 셰이더와 프래그먼트 셰이더를 컴파일하여 하나의 프로그램으로 연결한다.
    GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    if (vertexShader == 0 || fragmentShader == 0)
    {
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    GLint linked;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linked);
    if (!linked)
    {
        char errorLog[1024];
        glGetProgramInfoLog(shaderProgram, 1024, nullptr, errorLog);
        std::cerr << "셰이더 연결 실패: " << errorLog << '\n';
        glDeleteProgram(shaderProgram);
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    // 4. 정점 데이터를 CPU에서 만든 뒤 VBO에 복사한다.
    // VAO는 정점 속성의 해석 방법을 저장한다. location 0에는 vec3 위치가 들어간다.
    MakeVertices();

    GLuint vao;
    GLuint vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);
    glEnableVertexAttribArray(0);

    // 5. uniform 위치는 한 번 찾아 보관하고, 행렬과 색 값은 그릴 때마다 전달한다.
    glUseProgram(shaderProgram);
    transformLocation = glGetUniformLocation(shaderProgram, "transform");
    colorLocation = glGetUniformLocation(shaderProgram, "faceColor");
    glfwSetKeyCallback(window, KeyCallback);

    glfwSwapInterval(1);
    ResetObjects();
    std::cout
        << "WASD: walk | +/-: speed | J: jump, resume walking on landing (WASD in air: climb)\nO: open/close front door | I: reset | x/X: camera left/right | z/Z: forward/back | y/Y: orbit | Q: quit\n";

    // 6. 입력 처리, 시간 계산, 상태 갱신, 화면 그리기 순서로 반복한다.
    double previousTime = glfwGetTime();
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
        double currentTime = glfwGetTime();
        float deltaTime = float(currentTime - previousTime);
        previousTime = currentTime;
        // 창을 끌거나 중단점에서 돌아왔을 때 한 번에 너무 멀리 움직이지 않도록 제한한다.
        deltaTime = std::min(deltaTime, 0.05f);
        UpdateAnimation(deltaTime);
        // 최소화된 창은 높이가 0일 수 있으므로 투영 행렬 계산과 그리기를 생략한다.
        if (screenWidth > 0 && screenHeight > 0)
        {
            DrawScene();
            glfwSwapBuffers(window);
        }
    }

    // 7. OpenGL 컨텍스트를 닫기 전에 GPU 자원을 반환한다.
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
