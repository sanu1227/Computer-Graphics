// [복붙 위치] base.cpp의 FEATURE_BLOCK_END 바로 앞에 namespace 전체를 붙인다.
// [연결] 이 파일만 붙이면 실행되지 않는다. 참고서의 호출 등록부를 기존 On 함수 안에 합친다.
namespace TimedMatch
{
// ===== 1. 시험 조건에 맞게 바꿀 설정값 =====
constexpr float MatchTolerance =
    35.0f;                           // [수정] 두 도형 중심 Y 차이의 허용치. 커지면 맞추기 쉬워진다.
constexpr float SlideDuration = .8f; // [수정] 오른쪽으로 이동 완료하는 시간(초). 0보다 커야 한다.
constexpr int MaxStack =
    8; // [수정] 최대 쌓기 개수. 늘리면 쌓는 높이가 화면 안에 드는지도 확인한다.

// ===== 2. 실행 중 바뀌는 상태와 자료 =====
// progress는 0=출발, 1=도착이다. start는 이동 시작 때 한 번 저장한다. 매 프레임 현재 위치를
// 시작점으로 쓰면 일정 시간 보간이 되지 않는다.

float y[2] = {100, 450}, velocity[2] = {130, -100}, progress = 0;
bool sliding = false;
int stacked = 0;
glm::vec2 start(0), moving(0);
// [초기화] 시작/F5에서 호출한다. 설정값을 현재 상태로 복사한다. 조합할 때 use2D는 한 좌표계로
// 통일한다.
void Reset()
{
    use2D = true;
    y[0] = 100;
    y[1] = 450;
    velocity[0] = 130;
    velocity[1] = -100;
    progress = 0;
    sliding = false;
    stacked = 0;
}
bool CanMatch()
{
    return !sliding && std::abs(y[0] - y[1]) < MatchTolerance && y[0] >= 220 && y[0] <= 380;
}
// [키 입력] 실행 플래그나 상태를 바꾼다. 두 기능이 같은 키를 쓰면 등록부에서 키를 분배한다.
void Key(int key, int action, int)
{
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS && CanMatch() && stacked < MaxStack)
    {
        sliding = true;
        progress = 0;
        start = {280, (y[0] + y[1]) / 2};
        moving = start;
    }
}
// [시간 갱신] dt는 지난 프레임부터 흐른 초다. 초당 속도에는 dt를 곱한다. 그리기 전에 한 번
// 호출한다.
void Update(float dt)
{
    if (sliding)
    {
        progress = std::min(1.0f, progress + dt / SlideDuration);
        moving = glm::mix(start, glm::vec2(660, 550 - stacked * 35), progress);
        if (progress >= 1)
        {
            ++stacked;
            sliding = false;
            y[0] = 100;
            y[1] = 450;
        }
        return;
    }
    for (int i = 0; i < 2; ++i)
    {
        y[i] += velocity[i] * dt;
        if (y[i] < 60)
        {
            y[i] = 60;
            velocity[i] = std::abs(velocity[i]);
        }
        if (y[i] > 540)
        {
            y[i] = 540;
            velocity[i] = -std::abs(velocity[i]);
        }
    }
}
// [그리기] 상태를 화면에 표현한다. 같은 물체에 다른 기능을 합치면 이 함수에서 그 기능의 계산 결과를
// 사용한다.
void Draw()
{
    Rect({280, 300}, {280, 160}, {.2f, .3f, .2f}, false);
    if (sliding)
        Rect(moving, {100, 30}, Yellow);
    else
    {
        Rect({200, y[0]}, {60, 30}, Blue);
        Rect({360, y[1]}, {60, 30}, Red);
    }
    for (int i = 0; i < stacked; ++i)
        Rect({660, 550.0f - i * 35}, {100, 30}, Green);
}
// [자동 검사] 기본 설정값 기준의 검사다. 시험 조건으로 설정을 바꾸면 기대값도 바꿔야 한다. 일반
// 실행에서는 호출되지 않는다.
void Check()
{
    Reset();
    y[0] = 300;
    y[1] = 310;
    Key(GLFW_KEY_SPACE, GLFW_PRESS, 0);
    Update(.8f);
    Require(stacked == 1 && !sliding, "match interpolation");
}
} // namespace TimedMatch
