// 실습 1-21: Bouncing Balls
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

// 위도 0..20, 경도 0..32를 반지름 1인 구의 표면 좌표로 바꾼다.
glm::vec3 GetSpherePoint(int latitude, int longitude)
{
    const float pi = 3.14159265f;
    float latitudeAngle = pi * latitude / 20.0f;
    float longitudeAngle = 2.0f * pi * longitude / 32.0f;
    float x = std::sin(latitudeAngle) * std::cos(longitudeAngle);
    float y = std::cos(latitudeAngle);
    float z = std::sin(latitudeAngle) * std::sin(longitudeAngle);
    return glm::vec3(x, y, z);
}

void MakeSphereVertices()
{
    // 육면체의 정점 24개 뒤에 구의 삼각형을 이어 붙인다.
    // 위도/경도로 만든 작은 사각형 하나를 삼각형 두 개로 나눈다.
    for (int latitude = 0; latitude < 20; latitude++)
    {
        for (int longitude = 0; longitude < 32; longitude++)
        {
            glm::vec3 topLeft = GetSpherePoint(latitude, longitude);
            glm::vec3 bottomLeft = GetSpherePoint(latitude + 1, longitude);
            glm::vec3 bottomRight = GetSpherePoint(latitude + 1, longitude + 1);
            glm::vec3 topRight = GetSpherePoint(latitude, longitude + 1);
            // 바깥쪽이 앞면(CCW)이 되도록 정점 순서를 정한다.
            vertices.push_back(topLeft);
            vertices.push_back(bottomRight);
            vertices.push_back(bottomLeft);
            vertices.push_back(topLeft);
            vertices.push_back(topRight);
            vertices.push_back(bottomRight);
        }
    }
}

// ---------------- 실습 상태와 동작 ----------------

// 위치/속도는 회전 전 공간의 로컬 좌표이다. 공간 전체 회전은 그릴 때 적용한다.
struct Ball
{
    glm::vec3 position; // 공의 중심 위치
    glm::vec3 velocity; // 초당 이동량. 벽에 닿은 축의 성분만 반전한다.
    glm::vec3 color;
};
std::vector<Ball> balls;
float roomRoll = 0;         // 마우스로 조절하는 Z축 기울기(도)
float roomYaw = 0;          // 키보드로 조절하는 Y축 회전(도)
float roomZ = 0;            // 공간 전체의 Z축 이동량
float floorAngle = 0;       // 바닥이 열린 각도(0: 닫힘, 100: 열림)
float cubeX[3] = {};        // 각 육면체의 바닥 위 가로 위치
float cubeVelocity[3] = {}; // 각 육면체의 가로 이동 속도
float cubeY[3] = {};        // 각 육면체의 중심 높이
const float cubeSize[3] = {0.65f, 0.9f, 1.15f};
bool floorOpen = false;
bool dragging = false; // 직전 마우스 이동에서도 왼쪽 버튼을 누르고 있었는지 저장
double lastMouseX = 0;
constexpr float radius = 0.23f;
void AddBall()
{
    if (balls.size() >= 5)
    {
        return;
    }
    int index = int(balls.size());
    Ball ball;
    ball.position = glm::vec3(-1.5f + index * 0.65f, 0.4f, 0);
    ball.velocity = glm::vec3(1.2f + index * 0.23f, 1.8f + index * 0.1f, -1.3f + index * 0.14f);
    ball.color = palette[index];
    balls.push_back(ball);
}

// 초기값을 한곳에서 복원한다. 시작할 때와 리셋 키를 눌렀을 때 호출한다.
void ResetObjects()
{
    balls.clear();
    AddBall();
    roomRoll = 0;
    roomYaw = 0;
    roomZ = 0;
    floorAngle = 0;
    floorOpen = false;
    dragging = false;
    for (int i = 0; i < 3; i++)
    {
        cubeX[i] = 0;
        cubeVelocity[i] = 0;
        cubeY[i] = -3 + cubeSize[i] / 2;
    }
}

// 공 중심이 이동할 수 있는 구간 [low, high] 밖으로 나갔는지 검사한다.
// 벽을 넘어간 거리만큼 안쪽으로 되돌리므로, 반사 직후 벽에 붙어 멈추지 않는다.
void Reflect(float &position, float &velocity, float low, float high)
{
    if (position < low)
    {
        float overshoot = low - position;
        position = low + overshoot;
        velocity = std::abs(velocity); // 낮은 쪽 벽에서는 양의 방향으로 반사
    }
    if (position > high)
    {
        float overshoot = position - high;
        position = high - overshoot;
        velocity = -std::abs(velocity); // 높은 쪽 벽에서는 음의 방향으로 반사
    }
}

void UpdateAnimation(float deltaTime)
{
    // 한 프레임을 최대 0.005초 단위로 나눈다. 빠른 공의 충돌 검사 간격을 줄인다.
    int stepCount = std::max(1, int(std::ceil(deltaTime / 0.005f)));
    float stepTime = deltaTime / stepCount;
    for (int step = 0; step < stepCount; step++)
    {
        float targetAngle = 0.0f;
        if (floorOpen)
        {
            targetAngle = 100.0f;
        }
        float angleDifference = targetAngle - floorAngle;
        float maxAngleChange = 100.0f * stepTime;
        floorAngle += std::clamp(angleDifference, -maxAngleChange, maxAngleChange);
        // 간단한 낙하 모형: 바닥이 5도 넘게 열리면 바닥 반사를 끄고 중력을 적용한다.
        bool falling = floorAngle > 5.0f;
        for (Ball &ball : balls)
        {
            if (falling)
            {
                ball.velocity.y -= 6.0f * stepTime;
            }
            ball.position += ball.velocity * stepTime;
            // 바닥 아래로 완전히 빠져나간 공에는 더 이상 방의 벽을 적용하지 않는다.
            if (ball.position.y > -3.0f - radius)
            {
                // 중심 좌표가 벽에서 반지름만큼 떨어져 있어야 공 표면이 벽에 닿는다.
                Reflect(ball.position.x, ball.velocity.x, -3 + radius, 3 - radius);
                Reflect(ball.position.z, ball.velocity.z, -4 + radius, 4 - radius);
                // 앞면은 그리지 않지만, 공이 방 안에 머물도록 앞쪽 Z 경계에서도 반사한다.
                if (!falling)
                {
                    Reflect(ball.position.y, ball.velocity.y, -3 + radius, 3 - radius);
                }
                else if (ball.position.y > 3 - radius)
                {
                    ball.position.y = 3 - radius;
                    ball.velocity.y = -std::abs(ball.velocity.y);
                }
            }
        }
        for (int i = 0; i < 3; i++)
        {
            float restingHeight = -3 + cubeSize[i] / 2;
            bool alreadyFalling = cubeY[i] < restingHeight - 0.01f;
            if (falling || alreadyFalling)
            {
                // 육면체는 학습용으로 일정한 속도로 낙하하도록 단순화한다.
                cubeY[i] -= 3.0f * stepTime;
            }
            else
            {
                // 월드 중력 (0, -6, 0)을 회전된 공간의 좌표로 바꾸면 X 성분은 -6*sin(angle)이다.
                // 양의 각도에서는 오른쪽이 높으므로 왼쪽(-X)으로, 음의 각도에서는 반대로 미끄러진다.
                float slopeAcceleration = -6.0f * std::sin(glm::radians(roomRoll));
                cubeVelocity[i] += slopeAcceleration * stepTime;
                // 감쇠를 적용하여 기울기를 되돌리면 서서히 멈추도록 한다.
                float damping = std::exp(-1.5f * stepTime);
                cubeVelocity[i] *= damping;
                cubeX[i] += cubeVelocity[i] * stepTime;
                float halfSize = cubeSize[i] / 2;
                Reflect(cubeX[i], cubeVelocity[i], -3 + halfSize, 3 - halfSize);
            }
        }
    }
    // 화면 아래로 멀리 떨어진 공을 삭제한다. 삭제된 자리에 B로 새 공을 추가할 수 있다.
    // 뒤에서 앞으로 지우면 삭제 후 인덱스가 밀려도 아직 검사하지 않은 공을 건너뛰지 않는다.
    for (int i = int(balls.size()) - 1; i >= 0; i--)
    {
        if (balls[i].position.y < -20.0f)
        {
            balls.erase(balls.begin() + i);
        }
    }
}

void DrawScene()
{
    BeginFrame();
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glm::mat4 projection =
        glm::perspective(glm::radians(48.0f), float(screenWidth) / screenHeight, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(0, 1, 16), glm::vec3(0), glm::vec3(0, 1, 0));
    viewProjection = projection * view;
    // 공간, 공, 바닥의 육면체 모두 같은 부모 행렬 room을 적용한다.
    // 오른쪽부터 Z축 기울이기, Y축 회전, Z축 이동 순서이다.
    glm::mat4 translation = glm::translate(glm::mat4(1), glm::vec3(0, 0, roomZ));
    glm::mat4 yawRotation = RotationMatrix(roomYaw, glm::vec3(0, 1, 0));
    glm::mat4 rollRotation = RotationMatrix(roomRoll, glm::vec3(0, 0, 1));
    glm::mat4 room = translation * yawRotation * rollRotation;
    // 기존 상자 정점은 바깥쪽 CCW. CW로 전환하면 안쪽 면만 보인다.
    glFrontFace(GL_CW);
    glm::mat4 walls = glm::scale(room, glm::vec3(6, 6, 8));
    for (int face : {1, 2, 3, 4})
    {
        DrawFace(walls, face, palette[face] * 0.7f);
    }
    // 바닥 왼쪽 모서리를 회전 중심으로 삼아 문처럼 아래로 연다.
    glm::mat4 floor = glm::translate(room, glm::vec3(-3, -3, 0)) * RotationMatrix(-floorAngle, {0, 0, 1});
    DrawFace(MakeBox(floor, {3, 0, 0}, {6, 0, 8}), 5, palette[5]);
    glFrontFace(GL_CCW);
    for (int i = 0; i < 3; i++)
    {
        DrawBoxAt(room, {cubeX[i], cubeY[i], -2.5f + i * 2.5f}, glm::vec3(cubeSize[i]), palette[i]);
    }
    // 구 정점은 육면체 정점 24개 다음부터 저장되어 있다.
    for (const Ball &b : balls)
    {
        glm::mat4 m = viewProjection * MakeBox(room, b.position, glm::vec3(radius));
        glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(m));
        glUniform3fv(colorLocation, 1, glm::value_ptr(b.color));
        glDrawArrays(GL_TRIANGLES, 24, GLsizei(vertices.size() - 24));
    }
}

// GLFW는 키를 누를 때, 놓을 때, 길게 누를 때 이 함수를 호출한다.
// 대문자는 Shift로 구분한다. 토글은 PRESS에서만 처리하여 길게 눌러도 한 번만 바뀐다.
void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS && action != GLFW_REPEAT)
    {
        return;
    }
    float direction = 1.0f;
    if (mods & GLFW_MOD_SHIFT)
    {
        direction = -1.0f;
    }
    if (key == GLFW_KEY_Z)
    {
        roomZ = std::clamp(roomZ + direction * 0.3f, -20.0f, 5.0f);
    }
    if (key == GLFW_KEY_Y)
    {
        roomYaw -= direction * 5;
    }
    if (key == GLFW_KEY_B && action == GLFW_PRESS)
    {
        AddBall();
    }
    if (key == GLFW_KEY_O && action == GLFW_PRESS)
    {
        floorOpen = !floorOpen;
    }
    if (key == GLFW_KEY_R)
    {
        ResetObjects();
    }
    if (key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE)
    {
        glfwSetWindowShouldClose(window, true);
    }
}

void MouseCallback(GLFWwindow *w, double x, double)
{
    // 마우스의 이전 X 위치와 현재 X 위치 차이를 회전량으로 바꾼다.
    bool pressed = glfwGetMouseButton(w, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    if (pressed && dragging)
    {
        roomRoll -= float(x - lastMouseX) * 0.35f;
    }
    dragging = pressed;
    lastMouseX = x;
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
    GLFWwindow *window = glfwCreateWindow(screenWidth, screenHeight, "1-21 Bouncing Balls", nullptr, nullptr);
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
    MakeSphereVertices();
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
    glfwSetCursorPosCallback(window, MouseCallback);
    glfwSwapInterval(1);
    ResetObjects();
    std::cout
        << "z/Z: move | y/Y: rotate | B: ball (max 5) | drag: tilt\nO: open/close floor | R: reset | Q: quit\n";

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
