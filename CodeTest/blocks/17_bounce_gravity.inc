// [복붙 위치] base.cpp의 FEATURE_BLOCK_END 바로 앞에 namespace 전체를 붙인다.
// [연결] 이 파일만 붙이면 실행되지 않는다. 참고서의 호출 등록부를 기존 On 함수 안에 합친다.
namespace BounceGravity
{
// ===== 1. 시험 조건에 맞게 바꿀 설정값 =====
constexpr float BallRadius =
    .3f; // [수정] 공 반지름. 그림·벽 경계·바닥 착지 높이에 함께 적용된다. 0~3 사이.
constexpr float Gravity = 9.8f;    // [수정] 중력 크기. 커지면 더 빠르게 낙하한다.
constexpr float Restitution = .8f; // [수정] 반발 계수(0~1). 1이면 수직 충돌 직전 속력을 유지한다.
constexpr int MaxBalls = 5;        // [수정] B로 추가할 수 있는 최대 공 수.

// ===== 2. 실행 중 바뀌는 상태와 자료 =====
// Ball.p는 공간 회전 전 로컬 중심, Ball.v는 로컬 속도다. tilt는 공간 기울기(도)다. 반지름만
// 그림에서 바꾸면 벽 충돌이 어긋나므로 BallRadius를 바꾼다.

struct Ball
{
    glm::vec3 p, v;
};
std::vector<Ball> balls;
float tilt = 0;
bool floorOpen = false, dragging = false;
float lastX = 0;
// [초기화] 시작/F5에서 호출한다. 설정값을 현재 상태로 복사한다. 조합할 때 use2D는 한 좌표계로
// 통일한다.
void Reset()
{
    use2D = false;
    balls = {{{0, 2, 0}, {2, 0, 1}}};
    tilt = 0;
    floorOpen = dragging = false;
}
// [키 입력] 실행 플래그나 상태를 바꾼다. 두 기능이 같은 키를 쓰면 등록부에서 키를 분배한다.
void Key(int key, int action, int)
{
    if (action != GLFW_PRESS)
        return;
    if (key == GLFW_KEY_B && balls.size() < MaxBalls)
        balls.push_back({{0, 3, 0}, {2, 0, -1}});
    if (key == GLFW_KEY_O)
        floorOpen = !floorOpen;
}
// [마우스 버튼] 눌림과 놓임을 구분한다. mouseX/Y는 800×600 논리 좌표다.
void Mouse(int button, int action)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        dragging = action == GLFW_PRESS;
        lastX = mouseX;
    }
}
// [마우스 이동] 클릭 이후의 드래그 상태를 보고 현재 위치를 갱신한다.
void Cursor()
{
    if (dragging)
    {
        tilt = std::clamp(tilt + (mouseX - lastX) * .2f, -45.0f, 45.0f);
        lastX = mouseX;
    }
}
// [시간 갱신] dt는 지난 프레임부터 흐른 초다. 초당 속도에는 dt를 곱한다. 그리기 전에 한 번
// 호출한다.
void Update(float dt)
{
    int steps = std::max(1, int(std::ceil(dt / .005f)));
    float h = dt / steps;
    for (int s = 0; s < steps; ++s)
        for (auto &b : balls)
        {
            // 월드 중력 (0,-9.8,0)을 회전된 공간의 로컬 좌표로 변환한다.
            glm::vec3 gravity = glm::vec3(R(-tilt, {0, 0, 1}) * glm::vec4(0, -Gravity, 0, 0));
            b.v += gravity * h;
            b.p += b.v * h;
            for (int axis : {0, 2})
            {
                if (b.p[axis] > (3.0f - BallRadius))
                {
                    b.p[axis] = (3.0f - BallRadius);
                    b.v[axis] = -std::abs(b.v[axis]);
                }
                if (b.p[axis] < -(3.0f - BallRadius))
                {
                    b.p[axis] = -(3.0f - BallRadius);
                    b.v[axis] = std::abs(b.v[axis]);
                }
            }
            if (!floorOpen && b.p.y < BallRadius)
            {
                b.p.y = BallRadius;
                b.v.y = std::abs(b.v.y) * Restitution;
            }
            if (b.p.y < -8)
            {
                b.p = {0, 3, 0};
                b.v = {2, 0, 1};
            }
        }
}
// [그리기] 상태를 화면에 표현한다. 같은 물체에 다른 기능을 합치면 이 함수에서 그 기능의 계산 결과를
// 사용한다.
void Draw()
{
    glm::mat4 space = R(tilt, {0, 0, 1});
    if (!floorOpen)
        Box(space * T({0, -.1f, 0}) * S({6, .2f, 6}), {.3f, .35f, .4f});
    for (auto &b : balls)
        Sphere(space * T(b.p) * S(glm::vec3(BallRadius)), Yellow);
}
// [자동 검사] 기본 설정값 기준의 검사다. 시험 조건으로 설정을 바꾸면 기대값도 바꿔야 한다. 일반
// 실행에서는 호출되지 않는다.
void Check()
{
    Reset();
    balls[0] = {{0, .31f, 0}, {0, -2, 0}};
    Update(.05f);
    Require(balls[0].p.y >= .3f && balls[0].v.y > 0, "gravity floor bounce");
}
} // namespace BounceGravity
