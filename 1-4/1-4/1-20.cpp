// 실습 1-20: Conveyor
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

// ---------------- 실습 상태와 동작 ----------------

// 실습 20: ㄱ자 벨트. 경로 길이를 공유하므로 상자와 줄무늬가 함께 이동한다.
float beltDistance = 0;    // 벨트가 경로를 따라 이동한 누적 거리
float cameraYaw = 38;      // Y축 주위 카메라 공전 각도(도)
float cameraDistance = 17; // 카메라 거리. 커지면 멀어져서 작게 보인다.
bool beltMoving = false;   // B로 이동/정지 전환
bool perspective = true;   // true: 원근 투영, false: 직각 투영
constexpr float pathLength = 14.0f;

// 상자와 줄무늬는 같은 경로 함수를 사용한다. distance는 벨트를 따라 이동한 거리이다.
glm::mat4 GetBeltTransform(float distance)
{
    // 전체 길이 14를 넘으면 다시 시작점으로 돌아온다.
    distance = std::fmod(distance + pathLength, pathLength);
    glm::vec3 position;
    float angle;
    if (distance < 7.0f)
    {
        // 첫 구간: x=-4를 유지하고 z=4에서 z=-3까지 이동한다.
        position = glm::vec3(-4.0f, 1.5f, 4.0f - distance);
        angle = 180.0f;
    }
    else
    {
        // 두 번째 구간: z=-3을 유지하고 x=-4에서 x=3까지 이동한다.
        float secondDistance = distance - 7.0f;
        position = glm::vec3(-4.0f + secondDistance, 1.5f, -3.0f);
        angle = 90.0f;
    }
    // T * R: 상자를 자기 중심에서 회전시킨 뒤 벨트 위의 위치로 옮긴다.
    glm::mat4 translation = glm::translate(glm::mat4(1), position);
    glm::mat4 rotation = RotationMatrix(angle, glm::vec3(0, 1, 0));
    return translation * rotation;
}

// 초기값을 한곳에서 복원한다. 시작할 때와 리셋 키를 눌렀을 때 호출한다.
void ResetObjects()
{
    beltDistance = 0;
    beltMoving = false;
    perspective = true;
    cameraYaw = 38;
    cameraDistance = 17;
}

void UpdateAnimation(float deltaTime)
{
    if (beltMoving)
    {
        // 이동 거리 = 속도(초당 1.4) * 지난 시간. FPS가 달라도 이동 속도는 같다.
        float movement = 1.4f * deltaTime;
        beltDistance = std::fmod(beltDistance + movement, pathLength);
    }
}

void DrawNumber(glm::mat4 parent, int number)
{
    // 3x5 픽셀 숫자를 상자 윗면과 앞면에 그린다.
    const char *digits[] = {"010110010010111", "111001111100111", "111001111001111", "101101111001001",
                            "111100111001111"};
    // 문자열의 '1'인 칸에만 작은 육면체를 놓아 숫자를 만든다.
    // col은 가로 위치, row는 세로 위치이다. 위/앞면에 같은 숫자를 표시한다.
    for (int row = 0; row < 5; row++)
    {
        for (int col = 0; col < 3; col++)
        {
            if (digits[number - 1][row * 3 + col] == '1')
            {
                float x = (col - 1) * 0.14f, y = (2 - row) * 0.14f;
                DrawBoxAt(parent, {x, 0.815f, -y}, {0.115f, 0.015f, 0.115f}, {0.08f, 0.08f, 0.08f});
                DrawBoxAt(parent, {x, 0.4f + y, 0.412f}, {0.115f, 0.115f, 0.015f}, {0.08f, 0.08f, 0.08f});
            }
        }
    }
}

void DrawScene()
{
    BeginFrame();
    glDisable(GL_CULL_FACE);
    // 카메라의 높이/거리 비율은 고정하고 Y축으로 공전시켜 쿼터뷰를 유지한다.
    glm::vec4 initialEye(0, cameraDistance * 0.65f, cameraDistance, 1);
    glm::mat4 cameraRotation = RotationMatrix(cameraYaw, glm::vec3(0, 1, 0));
    glm::vec3 eye = glm::vec3(cameraRotation * initialEye);
    glm::mat4 view = glm::lookAt(eye, glm::vec3(0, 1, 0), glm::vec3(0, 1, 0));
    float aspect = float(screenWidth) / screenHeight;
    glm::mat4 projection;
    if (perspective)
    {
        projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
    }
    else
    {
        float halfHeight = cameraDistance * 0.5f;
        float halfWidth = halfHeight * aspect;
        projection = glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, 0.1f, 100.0f);
    }
    viewProjection = projection * view;
    glm::mat4 root(1);
    DrawBoxAt(root, {0, -0.1f, 0}, {15, 0.2f, 13}, {0.35f, 0.4f, 0.42f});
    // 바닥은 평면, 줄무늬와 받침은 육면체이다.
    DrawFace(MakeBox(root, {-4, 1.5f, 0.5f}, {1.6f, 0, 8.6f}), 4, {0.22f, 0.25f, 0.3f});
    DrawFace(MakeBox(root, {-0.5f, 1.5f, -3}, {8.6f, 0, 1.6f}), 4, {0.22f, 0.25f, 0.3f});
    // 28개 줄무늬를 0.5 간격으로 놓고, 상자와 같은 이동 거리를 더한다.
    for (int i = 0; i < 28; i++)
    {
        DrawBoxAt(GetBeltTransform(beltDistance + i * 0.5f), {0, 0.025f, 0}, {1.5f, 0.05f, 0.10f},
                  {0.8f, 0.75f, 0.4f});
    }
    // 상자 5개는 전체 경로 길이를 5등분한 간격으로 배치한다.
    for (int i = 0; i < 5; i++)
    {
        glm::mat4 pose = GetBeltTransform(beltDistance + i * pathLength / 5);
        DrawBoxAt(pose, {0, 0.45f, 0}, {0.8f, 0.7f, 0.8f}, palette[i]);
        DrawNumber(pose, i + 1);
    }
    // 벨트를 바닥 위에 지탱하는 고정 다리. 이 부분에는 이동 거리를 적용하지 않는다.
    for (float z : {-2.5f, 0.5f, 3.5f})
    {
        DrawBoxAt(root, {-4, 0.75f, z}, {0.3f, 1.5f, 0.3f}, {0.2f, 0.2f, 0.24f});
    }
    for (float x : {-1.0f, 2.5f})
    {
        DrawBoxAt(root, {x, 0.75f, -3}, {0.3f, 1.5f, 0.3f}, {0.2f, 0.2f, 0.24f});
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
    if (key == GLFW_KEY_B && action == GLFW_PRESS)
    {
        beltMoving = !beltMoving;
    }
    if (key == GLFW_KEY_P)
    {
        perspective = !(mods & GLFW_MOD_SHIFT);
    }
    if (key == GLFW_KEY_EQUAL || key == GLFW_KEY_KP_ADD)
    {
        cameraDistance = std::max(7.0f, cameraDistance - 0.5f);
    }
    if (key == GLFW_KEY_MINUS || key == GLFW_KEY_KP_SUBTRACT)
    {
        cameraDistance = std::min(40.0f, cameraDistance + 0.5f);
    }
    if (key == GLFW_KEY_LEFT)
    {
        cameraYaw -= 5;
    }
    if (key == GLFW_KEY_RIGHT)
    {
        cameraYaw += 5;
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
    GLFWwindow *window = glfwCreateWindow(screenWidth, screenHeight, "1-20 Conveyor", nullptr, nullptr);
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
        << "B: belt start/stop | +/-: zoom | p/P: perspective/orthographic\nArrows: camera orbit | R: reset | Q: quit\n";

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
