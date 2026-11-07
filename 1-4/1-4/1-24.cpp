// 실습 24: 곡선 컨베이어, 트럭 적재, 상자 기준 1인칭/3인칭 시점.
// 이 파일 하나만 빌드한다. C++17 이상, x64.
// 링크: opengl32.lib glew32.lib glfw3.lib
// 참고 기능: D06 육면체/바닥, M06 경로, A03 보간, I01 토글, P01/P04 카메라.
// 읽는 순서: OnReset -> OnKey -> OnUpdate -> GetBoxTransform -> UpdateCamera -> OnDraw.
#include <GL/glew.h>
#include <GL/glfw3.h>
#include <GL/glm/glm.hpp>
#include <GL/glm/gtc/matrix_transform.hpp>
#include <GL/glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

// ===== 공통 바탕: 여러 기능을 붙여도 아래 OpenGL 자원은 하나만 사용한다 =====
// 먼저 기능 블록의 [수정] 값을 바꾼다. 창·투영·그리기 방식을 바꿀 때만 이 영역을 고친다.
GLFWwindow* window = nullptr;
GLuint program = 0, vao = 0, vbo = 0;
GLint transformLocation = -1, colorLocation = -1;
int framebufferWidth = 1200, framebufferHeight = 675;
// 2D 논리 해상도다. 화면 크기와 다르다. 바꾸면 블록의 경계·배치 좌표도 맞춰야 한다.
constexpr float CanvasWidth = 800, CanvasHeight = 600;
float mouseX = 0, mouseY = 0; // 2D 논리 좌표. 왼쪽 위 (0,0), 오른쪽 아래 (800,600).
bool use2D = false, perspective = true, wireframe = false;
glm::vec3 background(0.55f, 0.74f, 0.9f); // PDF 예시의 하늘색 배경.
// eye=카메라 위치, target=바라보는 점, cameraUp=화면 위 방향. 15번 Camera가 이 값을 바꾼다.
glm::vec3 eye(6, 5, 9), target(0), cameraUp(0, 1, 0);
glm::mat4 viewProjection(1);
const glm::vec3 Red(0.95f, 0.25f, 0.25f), Green(0.25f, 0.85f, 0.45f);
const glm::vec3 Blue(0.25f, 0.55f, 1), Yellow(1, 0.8f, 0.2f);

void Require(bool condition, const char* message)
{
    if (!condition)
        throw std::runtime_error(message);
}
bool Near(float a, float b)
{
    return std::abs(a - b) < 0.001f;
}
// 이동: p의 x/y/z가 각 축 이동량이다. 3D에서는 +Y가 위쪽이다.
glm::mat4 T(glm::vec3 p)
{
    return glm::translate(glm::mat4(1), p);
}
// 회전: degrees는 도 단위. {0,1,0}은 Y축. sin/cos에 넣는 라디안과 구분한다.
glm::mat4 R(float degrees, glm::vec3 axis)
{
    return glm::rotate(glm::mat4(1), glm::radians(degrees), axis);
}
// 크기 배율: {2,1,1}이면 X방향만 두 배. Box의 기본 한 변은 1, Sphere의 기본 반지름은 1.
glm::mat4 S(glm::vec3 size)
{
    return glm::scale(glm::mat4(1), size);
}

// position과 size는 중심과 전체 크기이다. halfWidth와 혼동하지 않는다.
bool HitRect(float x, float y, glm::vec2 center, glm::vec2 size)
{
    return std::abs(x - center.x) <= size.x / 2 && std::abs(y - center.y) <= size.y / 2;
}
bool OverlapRect(glm::vec2 a, glm::vec2 sizeA, glm::vec2 b, glm::vec2 sizeB)
{
    return std::abs(a.x - b.x) <= (sizeA.x + sizeB.x) / 2 &&
        std::abs(a.y - b.y) <= (sizeA.y + sizeB.y) / 2;
}
void Primitive(GLenum mode, const std::vector<glm::vec3>& vertices, glm::mat4 model,
    glm::vec3 color)
{
    if (vertices.empty())
        return;
    glUseProgram(program);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(),
        GL_STREAM_DRAW);
    glm::mat4 transform = viewProjection * model;
    glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(transform));
    glUniform3fv(colorLocation, 1, glm::value_ptr(color));
    glDrawArrays(mode, 0, static_cast<GLsizei>(vertices.size()));
}
void Box(glm::mat4 model, glm::vec3 color)
{
    // model은 위치·회전·크기, color는 RGB(0~1)다. 같은 정점을 model만 바꿔 재사용한다.
    static const glm::vec3 p[] = { { -.5f, -.5f, .5f },{ .5f, -.5f, .5f },{ .5f, .5f, .5f },
                                  { -.5f, .5f, .5f },{ -.5f, -.5f, -.5f },{ .5f, -.5f, -.5f },
                                  { .5f, .5f, -.5f },{ -.5f, .5f, -.5f } };
    static const int faces[6][4] = { { 0, 1, 2, 3 },{ 1, 5, 6, 2 },{ 5, 4, 7, 6 },
                                    { 4, 0, 3, 7 },{ 3, 2, 6, 7 },{ 4, 5, 1, 0 } };
    for (int f = 0; f < 6; ++f)
        Primitive(GL_TRIANGLE_FAN, { p[faces[f][0]], p[faces[f][1]], p[faces[f][2]], p[faces[f][3]] },
            model, color * (1.0f - 0.08f * f));
}
void Rect(glm::vec2 center, glm::vec2 size, glm::vec3 color, bool filled = true)
{
    Primitive(filled ? GL_TRIANGLE_FAN : GL_LINE_LOOP,
        { { -.5f, -.5f, 0 },{ .5f, -.5f, 0 },{ .5f, .5f, 0 },{ -.5f, .5f, 0 } },
        T({ center.x, center.y, 0 }) * S({ size.x, size.y, 1 }), color);
}
void Circle(glm::vec2 center, float radius, glm::vec3 color, bool filled = true)
{
    std::vector<glm::vec3> points;
    if (filled)
        points.push_back({ center.x, center.y, 0 });
    for (int i = 0; i <= (filled ? 48 : 47); ++i)
    {
        float a = glm::radians(360.0f * i / 48);
        points.push_back({ center.x + radius * std::cos(a), center.y + radius * std::sin(a), 0 });
    }
    Primitive(filled ? GL_TRIANGLE_FAN : GL_LINE_LOOP, points, glm::mat4(1), color);
}
void Sphere(glm::mat4 model, glm::vec3 color)
{
    static const std::vector<glm::vec3> points = [] {
        std::vector<glm::vec3> result;
        auto point = [](int row, int col) {
            float a = glm::radians(-90.0f + 180.0f * row / 12), b = glm::radians(360.0f * col / 24);
            return glm::vec3(std::cos(a) * std::cos(b), std::sin(a), std::cos(a) * std::sin(b));
            };
        for (int row = 0; row < 12; ++row)
            for (int col = 0; col < 24; ++col)
            {
                auto a = point(row, col), b = point(row, col + 1), c = point(row + 1, col + 1),
                    d = point(row + 1, col);
                if (row != 0)
                    result.insert(result.end(), { a, b, c });
                if (row != 11)
                    result.insert(result.end(), { a, c, d });
            }
        return result;
        }();
    Primitive(GL_TRIANGLES, points, model, color); // 원점 중심, 반지름 1.
}
void Ground()
{
    Box(T({ 0, -.15f, 0 }) * S({ 10, .2f, 10 }), { .3f, .35f, .4f });
}
void SetView(int x, int y, int width, int height, glm::vec3 from, glm::vec3 at, glm::vec3 up,
    bool usePerspective, float halfHeight = 6.0f)
{
    if (width <= 0 || height <= 0)
        return;
    glViewport(x, y, width, height);
    float aspect = float(width) / height;
    glm::mat4 projection =
        usePerspective ? glm::perspective(glm::radians(45.0f), aspect, .1f, 100.0f)
                       : glm::ortho(-halfHeight * aspect, halfHeight * aspect,
                                    -halfHeight, halfHeight, .1f, 100.0f);
    viewProjection = projection * glm::lookAt(from, at, up);
}
void BeginFrame()
{
    glDisable(GL_SCISSOR_TEST);
    glViewport(0, 0, framebufferWidth, framebufferHeight);
    glClearColor(background.r, background.g, background.b, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
    if (use2D)
    {
        glDisable(GL_DEPTH_TEST);
        viewProjection = glm::ortho(0.0f, CanvasWidth, CanvasHeight, 0.0f, -10.0f, 10.0f);
    }
    else
    {
        glEnable(GL_DEPTH_TEST);
        SetView(0, 0, framebufferWidth, framebufferHeight, eye, target, cameraUp, perspective);
    }
}

// FEATURE_BLOCK_BEGIN
// ===== 1. 설정값: D06의 Box와 M06의 경로를 물류 센터에 맞춘다 =====
constexpr int BoxCount = 5;
constexpr float FirstLength = 7.0f;
constexpr float SecondLength = 7.0f;
constexpr float CornerRadius = 1.4f; // 클수록 모서리를 넓게 돌아간다. 벨트 반폭보다 크게 둔다.
constexpr float HalfPi = 1.57079632679f;
constexpr float FirstStraightLength = FirstLength - CornerRadius;
constexpr float SecondStraightLength = SecondLength - CornerRadius;
constexpr float CornerLength = CornerRadius * HalfPi; // 원호 길이 = 반지름 * 각도(라디안).
constexpr float SecondStart = FirstStraightLength + CornerLength;
constexpr float PathLength = SecondStart + SecondStraightLength;
constexpr float BeltHeight = 1.5f;
constexpr float BeltWidth = 1.6f;
constexpr float BeltSpeed = 1.4f; // 초당 이동 거리. 프레임마다 dt를 곱한다.
constexpr float StripeHeight = 0.05f;
constexpr float BoxSize = 0.8f;
constexpr float BeltCornerX = -1.6f;
constexpr float BeltCornerZ = 2.6f;
constexpr float BoxStartOffset = 0.8f;
constexpr float GroundWidth = 46.0f;
constexpr float GroundFarZ = -20.0f;
constexpr float GroundNearZ = 8.0f;
constexpr float GroundDepth = GroundNearZ - GroundFarZ;
constexpr float GroundCenterZ = (GroundFarZ + GroundNearZ) / 2;
constexpr float TruckX = -4.4f; // PDF 예시처럼 세로 벨트 왼쪽에 주차한다.
constexpr float TruckStartZ = -1.5f;
constexpr float TruckSpeed = 1.8f;
constexpr float TruckRearExtent = 2.2f;
constexpr float TruckFrontExtent = 2.5f;
constexpr float TruckMinZ = GroundFarZ + TruckRearExtent;
constexpr float TruckMaxZ = GroundNearZ - TruckFrontExtent;
constexpr float TruckBedTop = 1.1f;
constexpr float LoadDuration = 1.25f;
constexpr float LoadLift = 2.0f;
constexpr float DefaultCameraYaw = 0.0f;
constexpr float DefaultCameraDistance = 15.0f;
constexpr float CameraHeightRatio = 0.45f;
constexpr float CameraTargetY = 2.7f;
const glm::vec3 GroundColor(0.28f, 0.31f, 0.27f);
const glm::vec3 BeltColor(0.37f, 0.37f, 0.43f);
const glm::vec3 RailColor(0.65f, 0.56f, 0.1f);
const glm::vec3 BoxColors[BoxCount] = {
    {0.85f, 0.15f, 0.15f}, {0.9f, 0.52f, 0.08f}, {0.82f, 0.76f, 0.15f},
    {0.18f, 0.65f, 0.24f}, {0.18f, 0.32f, 0.73f}
};

// ===== 2. 상태: 입력은 이 값을 바꾸고, 그리기는 현재 값을 사용한다 =====
float beltDistance = 0;
bool beltMoving = false;
float cameraYaw = DefaultCameraYaw;
float cameraDistance = DefaultCameraDistance;
float truckZ = TruckStartZ;
int truckDirection = 1;
bool truckMoving = false;
bool firstPerson = false;
int cameraBox = BoxCount - 1; // 처음 1인칭으로 전환하면 5번 상자를 따라간다.

enum class ParcelState { OnBelt, Loading, Loaded };
struct Parcel
{
    ParcelState state = ParcelState::OnBelt;
    float loadProgress = 0;
    glm::vec3 loadStartPosition{0};
    float loadStartYaw = 0;
};
Parcel parcels[BoxCount];

// ===== 3. M06: 거리 하나로 위치와 방향을 함께 계산한다 =====
// 가로 직선에서 세로 직선으로 1/4원을 따라 연결한다.
// angle=0이면 진입점, angle=pi/2이면 이탈점이다. 벨트 표면도 이 함수를 공유한다.
glm::vec3 GetCornerPosition(float angle, float radius)
{
    return {BeltCornerX + CornerRadius - radius * std::sin(angle), BeltHeight,
        BeltCornerZ - CornerRadius + radius * std::cos(angle)};
}

// distance는 X좌표가 아니라 경로를 따라 이동한 거리다.
// PDF 예시처럼 오른쪽에서 왼쪽(-X), 모서리, 뒤쪽(-Z) 순서로 이동한다.
glm::mat4 GetBeltTransform(float distance)
{
    float s = std::fmod(distance, PathLength);
    if (s < 0)
        s += PathLength;

    if (s < FirstStraightLength)
        return T({BeltCornerX + FirstLength - s, BeltHeight, BeltCornerZ})
            * R(-90, {0, 1, 0});

    if (s < SecondStart)
    {
        // 이동 거리에서 각도를 구하므로 직선과 곡선의 이동 속도가 같다.
        float angle = (s - FirstStraightLength) / CornerRadius;
        // 원의 접선 방향을 바라본다. -90도에서 -180도까지 연속으로 회전한다.
        return T(GetCornerPosition(angle, CornerRadius))
            * R(-90 - glm::degrees(angle), {0, 1, 0});
    }

    return T({BeltCornerX, BeltHeight, BeltCornerZ - CornerRadius - (s - SecondStart)})
        * R(-180, {0, 1, 0});
}

// 상자 중심을 벨트의 줄무늬 위로 올린다.
glm::mat4 GetBeltBoxTransform(int index)
{
    float spacing = PathLength / BoxCount;
    return GetBeltTransform(beltDistance + BoxStartOffset + index * spacing)
        * T({0, StripeHeight + BoxSize / 2, 0});
}

glm::mat4 GetTruckTransform()
{
    return T({TruckX, 0, truckZ});
}

// 트럭의 로컬 좌표로 적재 위치를 정한다. 부모 행렬이 움직이면 화물도 함께 움직인다.
glm::mat4 GetCargoTransform(int index)
{
    float x = (index % 2 == 0) ? -0.46f : 0.46f;
    float z = -1.65f + (index / 2) * 0.95f;
    // 상자의 앞면은 적재함 뒤쪽을 향한다. 상자 시점에서 운전석 벽 대신 적재함이 보인다.
    return GetTruckTransform() * T({x, TruckBedTop + BoxSize / 2, z}) * R(180, {0, 1, 0});
}

glm::mat4 GetBoxTransform(int index)
{
    const Parcel& parcel = parcels[index];
    if (parcel.state == ParcelState::OnBelt)
        return GetBeltBoxTransform(index);
    if (parcel.state == ParcelState::Loaded)
        return GetCargoTransform(index);

    // A03: 시작 위치는 키를 누른 순간 저장하고, 도착 위치는 움직이는 트럭에서 구한다.
    float u = parcel.loadProgress;
    float smooth = u * u * (3 - 2 * u); // 시작과 끝에서 천천히 움직이는 보간.
    glm::vec3 destination(GetCargoTransform(index)[3]);
    glm::vec3 position = glm::mix(parcel.loadStartPosition, destination, smooth);
    position.y += LoadLift * std::sin(2 * HalfPi * u);
    float yaw = parcel.loadStartYaw
        + std::remainder(180.0f - parcel.loadStartYaw, 360.0f) * smooth;
    return T(position) * R(yaw, {0, 1, 0});
}

void LoadBox(int index)
{
    Parcel& parcel = parcels[index];
    if (parcel.state != ParcelState::OnBelt)
        return; // 적재 중이거나 이미 실린 상자는 다시 시작하지 않는다.

    glm::mat4 pose = GetBeltBoxTransform(index);
    parcel.loadStartPosition = glm::vec3(pose[3]);
    glm::vec3 forward = glm::vec3(pose * glm::vec4(0, 0, 1, 0));
    parcel.loadStartYaw = glm::degrees(std::atan2(forward.x, forward.z));
    parcel.loadProgress = 0;
    parcel.state = ParcelState::Loading;
}

int LoadedCount()
{
    int count = 0;
    for (const Parcel& parcel : parcels)
        if (parcel.state == ParcelState::Loaded)
            ++count;
    return count;
}

// 컨베이어 표면은 두께 없는 XZ 평면이다. 줄무늬/상자/다리는 Box로 그린다.
void DrawPlane(glm::vec3 center, glm::vec2 size, glm::vec3 color = BeltColor)
{
    Primitive(GL_TRIANGLE_FAN,
        {{-0.5f, 0, -0.5f}, {-0.5f, 0, 0.5f}, {0.5f, 0, 0.5f}, {0.5f, 0, -0.5f}},
        T(center) * S({size.x, 1, size.y}), color);
}

void DrawConveyor()
{
    float halfWidth = BeltWidth / 2;
    float turnX = BeltCornerX + CornerRadius;
    float turnZ = BeltCornerZ - CornerRadius;
    float endX = BeltCornerX + FirstLength;
    float endZ = BeltCornerZ - SecondLength;
    // 직선 표면은 곡선 진입/이탈 지점에서 끝난다. 시작/끝에는 반폭만큼 여유를 둔다.
    float horizontalCenter = (turnX + endX + halfWidth) / 2;
    float verticalCenter = (endZ - halfWidth + turnZ) / 2;
    float horizontalLength = FirstStraightLength + halfWidth;
    float verticalLength = SecondStraightLength + halfWidth;
    DrawPlane({horizontalCenter, BeltHeight, BeltCornerZ}, {horizontalLength, BeltWidth});
    DrawPlane({BeltCornerX, BeltHeight, verticalCenter}, {BeltWidth, verticalLength});

    // PDF 예시의 노란 양쪽 테두리. 끝은 열어 두어 상자 진행 방향이 보이게 한다.
    constexpr float RailWidth = 0.1f, RailHeight = 0.16f;
    float railOffset = (BeltWidth - RailWidth) / 2;
    for (float side : {-1.0f, 1.0f})
    {
        Box(T({horizontalCenter, BeltHeight + RailHeight / 2, BeltCornerZ + side * railOffset})
            * S({horizontalLength, RailHeight, RailWidth}), RailColor);
        Box(T({BeltCornerX + side * railOffset, BeltHeight + RailHeight / 2, verticalCenter})
            * S({RailWidth, RailHeight, verticalLength}), RailColor);
    }

    // 안쪽 원과 바깥쪽 원 사이를 삼각형으로 채워 둥근 벨트 표면을 만든다.
    constexpr int CurveSegments = 48;
    std::vector<glm::vec3> curve;
    curve.reserve(CurveSegments * 6);
    for (int i = 0; i < CurveSegments; ++i)
    {
        float a = HalfPi * i / CurveSegments;
        float b = HalfPi * (i + 1) / CurveSegments;
        glm::vec3 outerA = GetCornerPosition(a, CornerRadius + halfWidth);
        glm::vec3 innerA = GetCornerPosition(a, CornerRadius - halfWidth);
        glm::vec3 outerB = GetCornerPosition(b, CornerRadius + halfWidth);
        glm::vec3 innerB = GetCornerPosition(b, CornerRadius - halfWidth);
        curve.insert(curve.end(), {outerA, innerB, outerB, outerA, innerA, innerB});
        // 테두리도 같은 원호를 따라 배치해 부드러운 회전 구간을 유지한다.
        float middleAngle = (a + b) / 2;
        for (float radius : {CornerRadius - railOffset, CornerRadius + railOffset})
            Box(T(GetCornerPosition(middleAngle, radius)) * T({0, RailHeight / 2, 0})
                * R(-90 - glm::degrees(middleAngle), {0, 1, 0})
                * S({RailWidth, RailHeight, radius * (b - a) + 0.005f}), RailColor);
    }
    Primitive(GL_TRIANGLES, curve, glm::mat4(1), BeltColor);

    // 고정 다리도 같은 경로를 기준으로 배치한다. 이동 거리만 더하지 않는다.
    for (float s : {0.5f, FirstStraightLength / 2, FirstStraightLength + CornerLength / 2,
        SecondStart + SecondStraightLength / 2, PathLength - 0.5f})
        Box(GetBeltTransform(s) * T({0, -BeltHeight / 2, 0}) * S({0.3f, BeltHeight, 0.3f}),
            {0.08f, 0.09f, 0.08f});

    // 상자와 줄무늬가 같은 함수를 사용하므로 같은 속도와 방향으로 움직인다.
    constexpr int StripeCount = 28;
    for (int i = 0; i < StripeCount; ++i)
    {
        glm::mat4 pose = GetBeltTransform(beltDistance + i * PathLength / StripeCount);
        Box(pose * T({0, StripeHeight / 2, 0}) * S({1.5f, StripeHeight, 0.1f}),
            {0.07f, 0.07f, 0.09f});
    }
}

void DrawNumber(glm::mat4 parent, int number)
{
    // PDF 예시처럼 윗면의 검은 표식 1~5개로 해당 상자 번호를 나타낸다.
    for (int i = 0; i < number; ++i)
    {
        float x = (i - (number - 1) * 0.5f) * 0.13f;
        Box(parent * T({x, BoxSize / 2 + 0.008f, -0.13f}) * S({0.095f, 0.012f, 0.14f}),
            {0.04f, 0.04f, 0.04f});
    }
}

void DrawTruck()
{
    glm::mat4 parent = GetTruckTransform();
    auto part = [&](glm::vec3 position, glm::vec3 size, glm::vec3 color) {
        Box(parent * T(position) * S(size), color);
    };
    // 모든 부품은 트럭 원점을 기준으로 배치한다. +Z가 운전석 앞쪽이다.
    part({0, 0.65f, 0.15f}, {2.15f, 0.35f, 4.5f}, {0.17f, 0.2f, 0.24f});
    // 지면에 닿는 네 개의 바퀴도 과제 조건대로 육면체로 만든다.
    for (float x : {-1.1f, 1.1f})
        for (float z : {-1.4f, 1.35f})
        {
            part({x, 0.38f, z}, {0.35f, 0.76f, 0.76f}, {0.08f, 0.09f, 0.11f});
        }
    // 열린 적재함: 바닥 윗면은 y=1.1, 측면 벽 사이에 상자 두 열이 들어간다.
    part({0, 1, -0.6f}, {2.2f, 0.2f, 3.2f}, {0.48f, 0.43f, 0.33f});
    for (float x : {-1.05f, 1.05f})
        part({x, 1.45f, -0.6f}, {0.1f, 0.7f, 3.2f}, {0.56f, 0.44f, 0.27f});
    part({0, 1.45f, -2.15f}, {2.2f, 0.7f, 0.1f}, {0.56f, 0.44f, 0.27f});
    part({0, 1.35f, 0.95f}, {2.2f, 0.5f, 0.1f}, {0.56f, 0.44f, 0.27f});
    // 운전석, 유리, 앞 범퍼, 전조등.
    part({0, 1.4f, 1.65f}, {2.15f, 1.6f, 1.3f}, {0.5f, 0.51f, 0.55f});
    part({0, 2.23f, 1.65f}, {2.2f, 0.06f, 1.4f}, {0.76f, 0.76f, 0.8f});
    part({0, 1.75f, 2.31f}, {1.7f, 0.45f, 0.025f}, {0.16f, 0.24f, 0.32f});
    for (float x : {-1.08f, 1.08f})
        part({x, 1.75f, 1.65f}, {0.025f, 0.45f, 0.9f}, {0.16f, 0.24f, 0.32f});
    part({0, 0.8f, 2.4f}, {2.2f, 0.25f, 0.2f}, {0.16f, 0.17f, 0.18f});
}

void UpdateCamera()
{
    if (firstPerson)
    {
        // 상자의 현재 모델 행렬을 그대로 사용하므로 곡선 회전, 적재, 트럭 이동을 모두 따라간다.
        glm::mat4 pose = GetBoxTransform(cameraBox);
        eye = glm::vec3(pose * glm::vec4(0, BoxSize / 2 + 0.35f, BoxSize / 2 + 0.08f, 1));
        glm::vec3 forward = glm::vec3(pose * glm::vec4(0, 0, 1, 0));
        // 조금 아래를 바라보아 벨트와 주변 상자가 화면 안에 들어오게 한다.
        target = eye + forward * 5.0f + glm::vec3(0, -1.7f, 0);
        SetView(0, 0, framebufferWidth, framebufferHeight, eye, target, cameraUp, true);
        return;
    }
    // 높이와 거리의 비율을 유지하므로 줌/공전 중에도 3인칭 쿼터뷰가 유지된다.
    target = {0, CameraTargetY, 0};
    eye = glm::vec3(R(cameraYaw, {0, 1, 0})
        * glm::vec4(0, cameraDistance * CameraHeightRatio, cameraDistance, 1));
    SetView(0, 0, framebufferWidth, framebufferHeight, eye, target, cameraUp,
        perspective, cameraDistance * 0.5f);
}

void UpdateWindowTitle()
{
    std::string view = firstPerson ? "Box " + std::to_string(cameraBox + 1) + " (1st person)"
                                   : "3rd person";
    std::string title = "1-24 Logistics | " + view + " | Cargo " + std::to_string(LoadedCount())
        + "/5 | Truck " + (truckMoving ? "moving" : "stopped");
    static std::string previous;
    if (title != previous)
    {
        glfwSetWindowTitle(window, title.c_str());
        previous = title;
    }
}
// FEATURE_BLOCK_END

// HOOK_BLOCK_BEGIN
void OnReset()
{
    use2D = false;
    wireframe = false;
    perspective = true;
    beltDistance = 0;
    beltMoving = false;
    cameraYaw = DefaultCameraYaw;
    cameraDistance = DefaultCameraDistance;
    eye = {0, cameraDistance * CameraHeightRatio, cameraDistance};
    target = {0, CameraTargetY, 0};
    cameraUp = {0, 1, 0};
    truckZ = TruckStartZ;
    truckDirection = 1;
    truckMoving = false;
    firstPerson = false;
    cameraBox = BoxCount - 1;
    for (Parcel& parcel : parcels)
        parcel = Parcel{};
}
void OnKey(int key, int action, int mods)
{
    if (action != GLFW_PRESS && action != GLFW_REPEAT)
        return;

    // I01: 토글은 PRESS에서만 처리한다. 길게 눌러도 반복해서 뒤집히지 않는다.
    if (key == GLFW_KEY_B && action == GLFW_PRESS)
        beltMoving = !beltMoving;
    if (action == GLFW_PRESS)
    {
        if (key >= GLFW_KEY_1 && key <= GLFW_KEY_5)
            LoadBox(key - GLFW_KEY_1);
        if (key >= GLFW_KEY_KP_1 && key <= GLFW_KEY_KP_5)
            LoadBox(key - GLFW_KEY_KP_1);
        if (key == GLFW_KEY_V)
            firstPerson = !firstPerson;
        if (key == GLFW_KEY_N && firstPerson)
        {
            int direction = (mods & GLFW_MOD_SHIFT) ? -1 : 1;
            cameraBox = (cameraBox + direction + BoxCount) % BoxCount;
        }
        if (key == GLFW_KEY_G)
            truckMoving = !truckMoving;
    }
    if (key == GLFW_KEY_P)
        perspective = (mods & GLFW_MOD_SHIFT) == 0; // p=원근, P=직각.
    if (key == GLFW_KEY_EQUAL || key == GLFW_KEY_KP_ADD)
        cameraDistance = std::max(7.0f, cameraDistance - 0.5f);
    if (key == GLFW_KEY_MINUS || key == GLFW_KEY_KP_SUBTRACT)
        cameraDistance = std::min(40.0f, cameraDistance + 0.5f);
    if (key == GLFW_KEY_LEFT)
        cameraYaw -= 5.0f;
    if (key == GLFW_KEY_RIGHT)
        cameraYaw += 5.0f;
    if (key == GLFW_KEY_R)
        OnReset();
    if (key == GLFW_KEY_Q)
        glfwSetWindowShouldClose(window, true);
}
void OnMouse(int button, int action)
{
}
void OnCursor()
{
}
void OnUpdate(float dt)
{
    if (beltMoving)
        beltDistance = std::fmod(beltDistance + BeltSpeed * dt, PathLength);
    if (truckMoving)
    {
        truckZ += truckDirection * TruckSpeed * dt;
        if (truckDirection > 0 && truckZ >= TruckMaxZ)
        {
            truckZ = TruckMaxZ;
            truckMoving = false;
            truckDirection = -1; // 자동으로 되돌아가지 않고, 다음 G 입력에서 후진한다.
        }
        else if (truckDirection < 0 && truckZ <= TruckMinZ)
        {
            truckZ = TruckMinZ;
            truckMoving = false;
            truckDirection = 1;
        }
    }
    for (Parcel& parcel : parcels)
        if (parcel.state == ParcelState::Loading)
        {
            parcel.loadProgress = std::min(1.0f, parcel.loadProgress + dt / LoadDuration);
            if (parcel.loadProgress >= 1)
                parcel.state = ParcelState::Loaded;
        }
    UpdateWindowTitle();
}
void OnDraw()
{
    UpdateCamera();
    // PDF 예시처럼 넓은 녹색 XZ 평면을 바닥으로 사용한다.
    DrawPlane({0, 0, GroundCenterZ}, {GroundWidth, GroundDepth}, GroundColor);
    DrawConveyor();
    DrawTruck();
    for (int i = 0; i < BoxCount; ++i)
    {
        if (firstPerson && i == cameraBox)
            continue; // 현재 시점의 상자는 생략해 화면을 가리지 않게 한다.
        glm::mat4 pose = GetBoxTransform(i);
        Box(pose * S(glm::vec3(BoxSize)), BoxColors[i]);
        DrawNumber(pose, i + 1);
    }
}
void OnCheck()
{
    OnReset();
    OnUpdate(1);
    Require(Near(beltDistance, 0), "belt starts paused");
    OnKey(GLFW_KEY_B, GLFW_PRESS, 0);
    OnKey(GLFW_KEY_B, GLFW_REPEAT, 0);
    OnUpdate(1);
    Require(beltMoving && Near(beltDistance, BeltSpeed), "belt start and key repeat");
    OnKey(GLFW_KEY_B, GLFW_PRESS, 0);
    OnUpdate(1);
    Require(Near(beltDistance, BeltSpeed), "belt pause");

    auto position = [](float distance) { return glm::vec3(GetBeltTransform(distance)[3]); };
    Require(glm::length(position(0) - position(PathLength)) < 0.001f, "path wraps");
    Require(glm::length(position(-0.5f) - position(PathLength - 0.5f)) < 0.001f,
        "negative path distance wraps");
    auto forward = [](float s) {
        return glm::vec3(GetBeltTransform(s) * glm::vec4(0, 0, 1, 0));
    };
    for (float boundary : {FirstStraightLength, SecondStart})
    {
        Require(glm::length(position(boundary - 0.001f) - position(boundary + 0.001f))
            < 0.003f, "curve entry/exit position is continuous");
        Require(glm::length(forward(boundary - 0.001f) - forward(boundary + 0.001f))
            < 0.003f, "curve entry/exit rotation is continuous");
    }
    glm::vec3 firstForward = glm::vec3(GetBeltTransform(0) * glm::vec4(0, 0, 1, 0));
    glm::vec3 secondForward = glm::vec3(GetBeltTransform(SecondStart) * glm::vec4(0, 0, 1, 0));
    Require(glm::length(firstForward - glm::vec3(-1, 0, 0)) < 0.001f
        && glm::length(secondForward - glm::vec3(0, 0, -1)) < 0.001f, "box turns at corner");
    glm::vec3 middleForward = forward(FirstStraightLength + CornerLength / 2);
    Require(glm::dot(middleForward, glm::normalize(glm::vec3(-1, 0, -1))) > 0.9999f,
        "box turns halfway along curve");
    for (float s = FirstStraightLength - 0.5f; s < SecondStart + 0.5f; s += 0.02f)
    {
        glm::vec3 tangent = glm::normalize(position(s + 0.001f) - position(s - 0.001f));
        Require(glm::dot(forward(s), tangent) > 0.9999f, "box faces direction of travel");
        Require(std::abs(glm::length(position(s + 0.02f) - position(s)) - 0.02f) < 0.0001f,
            "constant speed through curve");
    }

    OnReset();
    for (int i = 0; i < BoxCount; ++i)
    {
        glm::vec3 center(GetBoxTransform(i)[3]);
        Require(Near(center.y - BoxSize / 2, BeltHeight + StripeHeight), "box rests on stripes");
        for (int j = i + 1; j < BoxCount; ++j)
            Require(glm::length(center - glm::vec3(GetBoxTransform(j)[3])) > BoxSize,
                "boxes are separated");
    }
    beltMoving = true;
    beltDistance = PathLength - BeltSpeed / 2;
    OnUpdate(1);
    Require(Near(beltDistance, BeltSpeed / 2), "moving belt wraps past endpoint");
    OnKey(GLFW_KEY_P, GLFW_PRESS, GLFW_MOD_SHIFT);
    Require(!perspective, "uppercase P selects orthographic");
    OnKey(GLFW_KEY_P, GLFW_PRESS, 0);
    Require(perspective, "lowercase p selects perspective");
    for (int i = 0; i < 100; ++i)
        OnKey(GLFW_KEY_EQUAL, GLFW_REPEAT, 0);
    Require(Near(cameraDistance, 7), "minimum camera distance");
    for (int i = 0; i < 100; ++i)
        OnKey(GLFW_KEY_MINUS, GLFW_REPEAT, 0);
    Require(Near(cameraDistance, 40), "maximum camera distance");
    OnKey(GLFW_KEY_LEFT, GLFW_PRESS, 0);
    Require(Near(cameraYaw, DefaultCameraYaw - 5), "clockwise camera orbit");
    OnKey(GLFW_KEY_RIGHT, GLFW_PRESS, 0);
    Require(Near(cameraYaw, DefaultCameraYaw), "counterclockwise camera orbit");
    OnKey(GLFW_KEY_R, GLFW_PRESS, 0);
    Require(!beltMoving && Near(beltDistance, 0) && perspective
        && Near(cameraDistance, DefaultCameraDistance) && Near(cameraYaw, DefaultCameraYaw),
        "reset restores scene");
    Require(TruckX + 1.3f < BeltCornerX - BeltWidth / 2,
        "truck lane stays left of conveyor");

    // 실습 24: 시점 전환은 한 번씩만 처리하고, 상자 번호는 양방향으로 순환한다.
    OnKey(GLFW_KEY_V, GLFW_PRESS, 0);
    OnKey(GLFW_KEY_V, GLFW_REPEAT, 0);
    Require(firstPerson && cameraBox == 4, "first-person starts at box 5 without key repeat");
    OnKey(GLFW_KEY_N, GLFW_PRESS, 0);
    Require(cameraBox == 0, "next box wraps from 5 to 1");
    OnKey(GLFW_KEY_N, GLFW_PRESS, GLFW_MOD_SHIFT);
    Require(cameraBox == 4, "previous box wraps from 1 to 5");
    OnKey(GLFW_KEY_V, GLFW_PRESS, 0);
    OnKey(GLFW_KEY_N, GLFW_PRESS, 0);
    Require(!firstPerson && cameraBox == 4, "box selection only changes in first-person");

    // 트럭은 양쪽 끝에서 정지하고, 다시 G를 누르기 전까지 움직이지 않는다.
    OnKey(GLFW_KEY_G, GLFW_PRESS, 0);
    OnKey(GLFW_KEY_G, GLFW_REPEAT, 0);
    Require(truckMoving, "truck start ignores key repeat");
    OnUpdate(100);
    Require(Near(truckZ, TruckMaxZ) && !truckMoving && truckDirection == -1,
        "truck stops at forward floor boundary");
    OnUpdate(10);
    Require(Near(truckZ, TruckMaxZ), "truck waits for restart at boundary");
    OnKey(GLFW_KEY_G, GLFW_PRESS, 0);
    OnUpdate(0.5f);
    Require(truckMoving && truckZ < TruckMaxZ, "truck restarts in reverse direction");
    OnUpdate(100);
    Require(Near(truckZ, TruckMinZ) && !truckMoving && truckDirection == 1,
        "truck stops at backward floor boundary");
    OnKey(GLFW_KEY_G, GLFW_PRESS, 0);
    OnUpdate(0.1f);
    OnKey(GLFW_KEY_G, GLFW_PRESS, 0);
    float stoppedZ = truckZ;
    OnUpdate(1);
    Require(Near(truckZ, stoppedZ), "truck can pause between boundaries");

    // 이동 중인 트럭에도 적재할 수 있고, 적재된 상자는 벨트 대신 트럭을 따라간다.
    OnReset();
    glm::vec3 start(GetBoxTransform(0)[3]);
    OnKey(GLFW_KEY_1, GLFW_PRESS, 0);
    Require(parcels[0].state == ParcelState::Loading
        && glm::length(glm::vec3(GetBoxTransform(0)[3]) - start) < 0.001f,
        "loading begins at current box position");
    beltMoving = true;
    OnKey(GLFW_KEY_G, GLFW_PRESS, 0);
    OnUpdate(LoadDuration / 2);
    Require(Near(parcels[0].loadProgress, 0.5f)
        && GetBoxTransform(0)[3].y > start.y + 0.5f, "box lifts during loading");
    OnKey(GLFW_KEY_1, GLFW_PRESS, 0);
    OnKey(GLFW_KEY_1, GLFW_REPEAT, 0);
    Require(Near(parcels[0].loadProgress, 0.5f), "duplicate load does not restart animation");
    OnUpdate(LoadDuration / 2);
    Require(parcels[0].state == ParcelState::Loaded
        && glm::length(glm::vec3(GetBoxTransform(0)[3]) - glm::vec3(GetCargoTransform(0)[3])) < 0.001f,
        "box lands in moving truck cargo slot");
    glm::vec3 cargoBefore(GetBoxTransform(0)[3]);
    float truckBefore = truckZ;
    OnUpdate(0.25f);
    glm::vec3 cargoDelta = glm::vec3(GetBoxTransform(0)[3]) - cargoBefore;
    Require(glm::length(cargoDelta - glm::vec3(0, 0, truckZ - truckBefore)) < 0.001f,
        "loaded box follows truck independently of belt");
    for (int i = 0; i < BoxCount; ++i)
        OnKey(GLFW_KEY_1 + i, GLFW_PRESS, 0);
    OnUpdate(LoadDuration);
    Require(LoadedCount() == BoxCount, "all five numbered boxes load");
    for (int i = 0; i < BoxCount; ++i)
    {
        glm::vec3 center(GetBoxTransform(i)[3]);
        Require(Near(center.y - BoxSize / 2, TruckBedTop), "cargo rests on truck bed");
        for (int j = i + 1; j < BoxCount; ++j)
            Require(glm::length(center - glm::vec3(GetBoxTransform(j)[3])) > BoxSize,
                "loaded boxes have separate slots");
    }

    OnKey(GLFW_KEY_V, GLFW_PRESS, 0);
    UpdateCamera();
    glm::vec3 cameraBefore = eye;
    truckBefore = truckZ;
    if (!truckMoving)
        OnKey(GLFW_KEY_G, GLFW_PRESS, 0);
    OnUpdate(0.25f);
    UpdateCamera();
    Require(std::abs(truckZ - truckBefore) > 0.001f
        && glm::length(eye - cameraBefore - glm::vec3(0, 0, truckZ - truckBefore)) < 0.001f,
        "first-person camera follows loaded box on truck");

    OnReset();
    OnKey(GLFW_KEY_V, GLFW_PRESS, 0);
    beltDistance = FirstStraightLength + CornerLength / 2 - BoxStartOffset
        - cameraBox * PathLength / BoxCount + PathLength;
    UpdateCamera();
    glm::vec3 viewForward = target - eye;
    viewForward.y = 0;
    glm::vec3 boxForward = glm::vec3(GetBoxTransform(cameraBox) * glm::vec4(0, 0, 1, 0));
    Require(glm::dot(glm::normalize(viewForward), boxForward) > 0.9999f,
        "first-person camera turns with box on curved belt");
    OnKey(GLFW_KEY_KP_5, GLFW_PRESS, 0);
    Require(parcels[4].state == ParcelState::Loading, "numpad also loads numbered box");
    OnKey(GLFW_KEY_R, GLFW_PRESS, 0);
    Require(!firstPerson && cameraBox == 4 && LoadedCount() == 0 && !truckMoving
        && Near(truckZ, TruckStartZ) && truckDirection == 1, "reset restores exercise 24 state");
    for (const Parcel& parcel : parcels)
        Require(parcel.state == ParcelState::OnBelt && Near(parcel.loadProgress, 0),
            "reset puts every box back on belt");
}
// HOOK_BLOCK_END

void UpdateMouse()
{
    int width, height;
    double x, y;
    glfwGetWindowSize(window, &width, &height);
    glfwGetCursorPos(window, &x, &y);
    if (width > 0 && height > 0)
    {
        mouseX = float(x) * CanvasWidth / width;
        mouseY = float(y) * CanvasHeight / height;
    }
}
GLuint Compile(GLenum type, const char* source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        char log[2048];
        glGetShaderInfoLog(shader, 2048, nullptr, log);
        throw std::runtime_error(log);
    }
    return shader;
}
int main(int argc, char** argv)
{
    bool check = argc > 1 && std::string(argv[1]) == "--check";
    try
    {
        Require(glfwInit() != 0, "GLFW init failed");
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        if (check)
            glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        window = glfwCreateWindow(framebufferWidth, framebufferHeight, "1-24 Logistics", nullptr, nullptr);
        Require(window != nullptr, "Window creation failed");
        glfwMakeContextCurrent(window);
        glewExperimental = GL_TRUE;
        Require(glewInit() == GLEW_OK, "GLEW init failed");
        while (glGetError() != GL_NO_ERROR)
        {
        }
        const char* vs = R"GLSL(#version 330 core
layout(location=0) in vec3 position;
uniform mat4 transform;
void main(){gl_Position=transform*vec4(position,1.0);}
)GLSL";
        const char* fs = R"GLSL(#version 330 core
uniform vec3 color;
out vec4 outColor;
void main(){outColor=vec4(color,1.0);}
)GLSL";
        GLuint vertex = Compile(GL_VERTEX_SHADER, vs), fragment = Compile(GL_FRAGMENT_SHADER, fs);
        program = glCreateProgram();
        glAttachShader(program, vertex);
        glAttachShader(program, fragment);
        glLinkProgram(program);
        GLint linked = 0;
        glGetProgramiv(program, GL_LINK_STATUS, &linked);
        if (!linked)
        {
            char log[2048];
            glGetProgramInfoLog(program, 2048, nullptr, log);
            throw std::runtime_error(log);
        }
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        transformLocation = glGetUniformLocation(program, "transform");
        colorLocation = glGetUniformLocation(program, "color");
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);
        glEnableVertexAttribArray(0);
        glfwSetKeyCallback(window, [](GLFWwindow*, int key, int, int action, int mods) {
            if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
                glfwSetWindowShouldClose(window, true);
            if (key == GLFW_KEY_F5 && action == GLFW_PRESS)
                OnReset();
            OnKey(key, action, mods);
            });
        glfwSetMouseButtonCallback(window, [](GLFWwindow*, int button, int action, int) {
            UpdateMouse();
            OnMouse(button, action);
            });
        glfwSetCursorPosCallback(window, [](GLFWwindow*, double, double) {
            UpdateMouse();
            OnCursor();
            });
        glfwSwapInterval(check ? 0 : 1);
        OnReset();
        if (!check)
            std::cout << "1-5: load box | V: 1st/3rd person | n/N: next/previous box\n"
                         "G: truck start/stop (restart reverses at boundary) | B: belt start/stop\n"
                         "+/-: zoom | p/P: perspective/orthographic | Left/Right: camera orbit\n"
                         "R/F5: reset | Q/Esc: quit\n";
        if (check)
        {
            OnCheck();
            OnReset();
        }
        double previous = glfwGetTime();
        int frames = 0;
        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();
            double now = glfwGetTime();
            float dt = check ? 1.0f / 60 : std::min(float(now - previous), 0.05f);
            previous = now;
            OnUpdate(dt);
            glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
            if (framebufferWidth > 0 && framebufferHeight > 0)
            {
                BeginFrame();
                OnDraw();
                if (check)
                    Require(glGetError() == GL_NO_ERROR, "OpenGL error");
                if (check && frames == 7)
                {
                    std::vector<unsigned char> pixels(framebufferWidth * framebufferHeight * 3);
                    glPixelStorei(GL_PACK_ALIGNMENT, 1);
                    glReadPixels(0, 0, framebufferWidth, framebufferHeight, GL_RGB,
                        GL_UNSIGNED_BYTE, pixels.data());
                    Require(glGetError() == GL_NO_ERROR, "Pixel read error");
                    int changed = 0;
                    for (size_t i = 0; i < pixels.size(); i += 3)
                    {
                        bool different = false;
                        for (int c = 0; c < 3; ++c)
                            different = different ||
                            std::abs(int(pixels[i + c]) - int(background[c] * 255)) > 5;
                        if (different)
                            ++changed;
                    }
                    Require(changed > 50, "No visible geometry in framebuffer");
                }
                glfwSwapBuffers(window);
            }
            if (check && ++frames >= 8)
                break;
        }
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
        glDeleteProgram(program);
        glfwDestroyWindow(window);
        glfwTerminate();
        if (check)
            std::cout << "PASS: logic checks, 8 hidden OpenGL frames, non-background pixels\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        if (window)
            glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }
}
