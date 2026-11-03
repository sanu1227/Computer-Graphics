// [복붙 위치] base.cpp의 FEATURE_BLOCK_END 바로 앞에 namespace 전체를 붙인다.
// [연결] 이 파일만 붙이면 실행되지 않는다. 참고서의 호출 등록부를 기존 On 함수 안에 합친다.
namespace MotionPath
{
// ===== 1. 시험 조건에 맞게 바꿀 설정값 =====
const glm::vec2 InitialVelocity(150, 100); // [수정] 초당 x·y 이동량. 음수면 왼쪽·위쪽으로 간다.
constexpr float SpiralRadius =
    200.0f; // [수정] 나선의 최대 반지름. 중심 (400,300)에서 이 거리까지 퍼진다.
constexpr float SpiralGrowth = 20.0f; // [수정] 나선 반지름이 초당 커지는 양.
constexpr float SpiralSpeed = 3.0f;   // [수정] 라디안/초. 음수로 바꾸면 회전 방향이 반대가 된다.

// ===== 2. 실행 중 바뀌는 상태와 자료 =====
// position은 현재 중심, velocity는 초당 속도, angle은 도형 방향(도), time은 경로 시간(초)이다. 기본
// 삼각형은 +X를 바라본다.

glm::vec2 position(400, 300), velocity(InitialVelocity);
float time = 0, angle = 0;
int mode = 1;
std::vector<glm::vec3> path;
// [초기화] 시작/F5에서 호출한다. 설정값을 현재 상태로 복사한다. 조합할 때 use2D는 한 좌표계로
// 통일한다.
void Reset()
{
    use2D = true;
    position = {400, 300};
    velocity = InitialVelocity;
    time = angle = 0;
    mode = 1;
    path.clear();
}
// [키 입력] 실행 플래그나 상태를 바꾼다. 두 기능이 같은 키를 쓰면 등록부에서 키를 분배한다.
void Key(int key, int action, int)
{
    if (action == GLFW_PRESS && key >= GLFW_KEY_1 && key <= GLFW_KEY_3)
    {
        mode = key - GLFW_KEY_0;
        time = 0;
        path.clear();
    }
}
void Reflect(float &p, float &v, float low, float high)
{
    // 시험용: 위치를 경계로 보정한다. 한 프레임의 초과 이동 거리는 버린다.
    if (p > high)
    {
        p = high;
        v = -std::abs(v);
    }
    else if (p < low)
    {
        p = low;
        v = std::abs(v);
    }
}
// [시간 갱신] dt는 지난 프레임부터 흐른 초다. 초당 속도에는 dt를 곱한다. 그리기 전에 한 번
// 호출한다.
void Update(float dt)
{
    glm::vec2 old = position;
    time += dt;
    if (mode == 1)
    {
        position += velocity * dt;
        Reflect(position.x, velocity.x, 25, 775);
        Reflect(position.y, velocity.y, 25, 575);
    }
    if (mode == 2)
    {
        float u = std::fmod(time * 120, 1400.0f);
        position.x = u <= 700 ? 50 + u : 1450 - u;
        float v = std::fmod(time, 2.0f);
        position.y = 150 + 300 * (v <= 1 ? v : 2 - v);
    }
    if (mode == 3)
    {
        float radius = std::min(SpiralRadius, time * SpiralGrowth);
        position = {400 + radius * std::cos(time * SpiralSpeed),
                    300 + radius * std::sin(time * SpiralSpeed)};
    }
    glm::vec2 d = position - old;
    if (glm::length(d) > .001f)
        angle = glm::degrees(std::atan2(d.y, d.x));
    path.push_back({position.x, position.y, 0});
    if (path.size() > 500)
        path.erase(path.begin());
}
// [그리기] 상태를 화면에 표현한다. 같은 물체에 다른 기능을 합치면 이 함수에서 그 기능의 계산 결과를
// 사용한다.
void Draw()
{
    Primitive(GL_LINE_STRIP, path, glm::mat4(1), {.5f, .5f, .5f});
    Primitive(GL_TRIANGLES, {{25, 0, 0}, {-20, -15, 0}, {-20, 15, 0}},
              T({position.x, position.y, 0}) * R(angle, {0, 0, 1}), Yellow);
}
// [자동 검사] 기본 설정값 기준의 검사다. 시험 조건으로 설정을 바꾸면 기대값도 바꿔야 한다. 일반
// 실행에서는 호출되지 않는다.
void Check()
{
    Reset();
    position.x = 774;
    velocity.x = 150;
    Update(.1f);
    Require(position.x == 775 && velocity.x < 0, "wall reflection");
}
} // namespace MotionPath
