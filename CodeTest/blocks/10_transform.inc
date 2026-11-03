// [복붙 위치] base.cpp의 FEATURE_BLOCK_END 바로 앞에 namespace 전체를 붙인다.
// [연결] 이 파일만 붙이면 실행되지 않는다. 참고서의 호출 등록부를 기존 On 함수 안에 합친다.
namespace Transform
{
// ===== 1. 시험 조건에 맞게 바꿀 설정값 =====
constexpr float MoveStep = .2f;      // [수정] 키 입력 한 번의 이동 거리. 연속 초당 속도가 아니다.
constexpr float SpinSpeed = 60.0f;   // [수정] 자전 각속도(도/초). 음수면 반대 회전.
constexpr float SwapDuration = 2.0f; // [수정] 두 물체 자리 교환 시간(초). 값이 작으면 빨라진다.

// ===== 2. 실행 중 바뀌는 상태와 자료 =====
// position은 두 물체의 위치, angle은 각자의 자전각, size는 각자의 배율이다. selected는 0 또는
// 1이다. size를 두 배로 하면 모든 축 길이가 두 배다.

glm::vec3 position[2] = {{-2, 0, 0}, {2, 0, 0}};
float angle[2] = {}, size[2] = {1, 1};
int selected = 0;
bool spinning = false, swapping = false;
float progress = 0;
glm::vec3 start[2];
// [초기화] 시작/F5에서 호출한다. 설정값을 현재 상태로 복사한다. 조합할 때 use2D는 한 좌표계로
// 통일한다.
void Reset()
{
    use2D = false;
    position[0] = {-2, 0, 0};
    position[1] = {2, 0, 0};
    angle[0] = angle[1] = 0;
    size[0] = size[1] = 1;
    selected = 0;
    spinning = swapping = false;
    progress = 0;
}
// [키 입력] 실행 플래그나 상태를 바꾼다. 두 기능이 같은 키를 쓰면 등록부에서 키를 분배한다.
void Key(int key, int action, int mods)
{
    if (action != GLFW_PRESS && action != GLFW_REPEAT)
        return;
    float sign = (mods & GLFW_MOD_SHIFT) ? -1.0f : 1.0f;
    if (key == GLFW_KEY_1)
        selected = 0;
    if (key == GLFW_KEY_2)
        selected = 1;
    if (key == GLFW_KEY_X)
        position[selected].x += MoveStep * sign;
    if (key == GLFW_KEY_Y)
        position[selected].y += MoveStep * sign;
    if (key == GLFW_KEY_Z)
        position[selected].z += MoveStep * sign;
    if (key == GLFW_KEY_S)
        size[selected] = std::clamp(size[selected] + .1f * sign, .1f, 3.0f);
    if (key == GLFW_KEY_R && action == GLFW_PRESS)
        spinning = !spinning;
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS && !swapping)
    {
        swapping = true;
        progress = 0;
        start[0] = position[0];
        start[1] = position[1];
    }
}
// [시간 갱신] dt는 지난 프레임부터 흐른 초다. 초당 속도에는 dt를 곱한다. 그리기 전에 한 번
// 호출한다.
void Update(float dt)
{
    if (spinning)
        angle[selected] += SpinSpeed * dt;
    if (swapping)
    {
        progress = std::min(1.0f, progress + dt / SwapDuration);
        position[0] = glm::mix(start[0], start[1], progress);
        position[1] = glm::mix(start[1], start[0], progress);
        if (progress >= 1)
            swapping = false;
    }
}
// [그리기] 상태를 화면에 표현한다. 같은 물체에 다른 기능을 합치면 이 함수에서 그 기능의 계산 결과를
// 사용한다.
void Draw()
{
    for (int i = 0; i < 2; ++i)
        Box(T(position[i]) * R(angle[i], {0, 1, 0}) * S(glm::vec3(size[i])),
            i == selected ? Yellow : Blue);
}
// [자동 검사] 기본 설정값 기준의 검사다. 시험 조건으로 설정을 바꾸면 기대값도 바꿔야 한다. 일반
// 실행에서는 호출되지 않는다.
void Check()
{
    Reset();
    Key(GLFW_KEY_SPACE, GLFW_PRESS, 0);
    Update(2);
    Require(Near(position[0].x, 2) && Near(position[1].x, -2) && !swapping, "swap endpoints");
}
} // namespace Transform
