// [복붙 위치] base.cpp의 FEATURE_BLOCK_END 바로 앞에 namespace 전체를 붙인다.
// [연결] 이 파일만 붙이면 실행되지 않는다. 참고서의 호출 등록부를 기존 On 함수 안에 합친다.
namespace Conveyor
{
// ===== 1. 시험 조건에 맞게 바꿀 설정값 =====
constexpr int BoxCount = 5; // [수정] 상자 수. 증가하면 같은 경로에 더 촘촘하게 배치된다. 1 이상.
constexpr float BeltSpeed = 1.5f; // [수정] 초당 경로 이동 거리. 양수로 둔다.
constexpr float BoxSize = .8f;    // [수정] 상자 한 변. 바닥 위 높이도 함께 계산한다.

// ===== 2. 실행 중 바뀌는 상태와 자료 =====
// distance는 x좌표가 아니라 경로를 따라 누적한 거리다. First/Second를 바꾸면 경로와 벨트 길이가
// 함께 바뀐다. Path(s)는 다른 물체에도 그대로 사용할 수 있다.

float distance = 0;
bool running = true;
constexpr float First = 6, Second = 4, Length = First + Second;
// [초기화] 시작/F5에서 호출한다. 설정값을 현재 상태로 복사한다. 조합할 때 use2D는 한 좌표계로
// 통일한다.
void Reset()
{
    use2D = false;
    distance = 0;
    running = true;
}
// [키 입력] 실행 플래그나 상태를 바꾼다. 두 기능이 같은 키를 쓰면 등록부에서 키를 분배한다.
void Key(int key, int action, int)
{
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        running = !running;
}
glm::mat4 Path(float s)
{
    s = std::fmod(s, Length);
    if (s < 0)
        s += Length;
    if (s < First)
        return T({-First / 2 + s, BoxSize / 2, -Second / 2});
    return T({First / 2, BoxSize / 2, -Second / 2 + (s - First)}) * R(-90, {0, 1, 0});
}
// [시간 갱신] dt는 지난 프레임부터 흐른 초다. 초당 속도에는 dt를 곱한다. 그리기 전에 한 번
// 호출한다.
void Update(float dt)
{
    if (running)
        distance = std::fmod(distance + BeltSpeed * dt, Length);
}
// [그리기] 상태를 화면에 표현한다. 같은 물체에 다른 기능을 합치면 이 함수에서 그 기능의 계산 결과를
// 사용한다.
void Draw()
{
    Box(T({0, -.1f, -Second / 2}) * S({First, .2f, 1.4f}), {.3f, .3f, .3f});
    Box(T({First / 2, -.1f, 0}) * S({1.4f, .2f, Second}), {.3f, .3f, .3f});
    for (int i = 0; i < BoxCount; ++i)
        Box(Path(distance + i * Length / BoxCount) * S(glm::vec3(BoxSize)), i % 2 ? Blue : Yellow);
}
// [자동 검사] 기본 설정값 기준의 검사다. 시험 조건으로 설정을 바꾸면 기대값도 바꿔야 한다. 일반
// 실행에서는 호출되지 않는다.
void Check()
{
    Reset();
    auto a = glm::vec3(Path(First) * glm::vec4(0, 0, 0, 1));
    auto b = glm::vec3(Path(Length) * glm::vec4(0, 0, 0, 1));
    Require(Near(a.x, 3) && Near(a.z, -2) && Near(b.x, -3), "piecewise path");
}
} // namespace Conveyor
