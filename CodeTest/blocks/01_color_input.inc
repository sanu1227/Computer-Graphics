// [복붙 위치] base.cpp의 FEATURE_BLOCK_END 바로 앞에 namespace 전체를 붙인다.
// [연결] 이 파일만 붙이면 실행되지 않는다. 참고서의 호출 등록부를 기존 On 함수 안에 합친다.
namespace ColorInput
{
// ===== 1. 시험 조건에 맞게 바꿀 설정값 =====
constexpr float ChangeInterval =
    1.0f; // [수정] 초. 1→0.5이면 색이 두 배 자주 바뀐다. 0보다 커야 한다.
const glm::vec2 MarkerSize(160,
                           100); // [수정] 화면에 보이는 사각형의 전체 너비·높이. 단위: 논리 픽셀.

// ===== 2. 실행 중 바뀌는 상태와 자료 =====
// running은 자동 변경 여부, timer는 지난 변경 이후 누적 시간이다. 다른 기능의 배경만 바꾸려면 Draw
// 호출은 제외한다.

bool running = false;
float timer = 0;
// [초기화] 시작/F5에서 호출한다. 설정값을 현재 상태로 복사한다. 조합할 때 use2D는 한 좌표계로
// 통일한다.
void Reset()
{
    use2D = true;
    running = false;
    timer = 0;
    background = {.08f, .1f, .15f};
}
// [키 입력] 실행 플래그나 상태를 바꾼다. 두 기능이 같은 키를 쓰면 등록부에서 키를 분배한다.
void Key(int key, int action, int mods)
{
    if (action != GLFW_PRESS)
        return; // 토글은 반복 입력을 받지 않는다.
    if (key == GLFW_KEY_SPACE)
        running = !running;
    if (key == GLFW_KEY_C)
        background = (mods & GLFW_MOD_SHIFT) ? Red : Blue;
}
// [시간 갱신] dt는 지난 프레임부터 흐른 초다. 초당 속도에는 dt를 곱한다. 그리기 전에 한 번
// 호출한다.
void Update(float dt)
{
    if (!running)
        return;
    timer += dt;
    while (timer >= ChangeInterval)
    {
        timer -= ChangeInterval;
        background = {float(rand() % 256) / 255, float(rand() % 256) / 255,
                      float(rand() % 256) / 255};
    }
}
// [그리기] 상태를 화면에 표현한다. 같은 물체에 다른 기능을 합치면 이 함수에서 그 기능의 계산 결과를
// 사용한다.
void Draw()
{
    Rect({400, 300}, MarkerSize, Yellow);
}
// [자동 검사] 기본 설정값 기준의 검사다. 시험 조건으로 설정을 바꾸면 기대값도 바꿔야 한다. 일반
// 실행에서는 호출되지 않는다.
void Check()
{
    Reset();
    Key(GLFW_KEY_SPACE, GLFW_PRESS, 0);
    Update(1.1f);
    Require(running && Near(timer, .1f), "color timer");
}
} // namespace ColorInput
