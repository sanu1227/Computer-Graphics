// [복붙 위치] base.cpp의 FEATURE_BLOCK_END 바로 앞에 namespace 전체를 붙인다.
// [연결] 이 파일만 붙이면 실행되지 않는다. 참고서의 호출 등록부를 기존 On 함수 안에 합친다.
namespace ShrinkingDoor
{
// ===== 1. 시험 조건에 맞게 바꿀 설정값 =====
constexpr float DoorHeight = 4.0f;   // [수정] 닫힌 문 높이. 경첩과 문기둥 높이도 함께 바뀐다.
constexpr float DoorWidth = 3.0f;    // [수정] 문 너비. 양쪽 기둥 위치도 자동으로 이동한다.
constexpr float OpenDegrees = 90.0f; // [수정] 완전히 열었을 때 회전각(도).
constexpr float Duration = 1.0f;     // [수정] 개폐 완료 시간(초). 0보다 커야 한다.

// ===== 2. 실행 중 바뀌는 상태와 자료 =====
// progress=0은 닫힘, 1은 완전 열림이다. open은 목표 상태다. Model()만 가져와 다른 장면에서 문을
// 그릴 수 있다.

float progress = 0;
bool open = false;
// [초기화] 시작/F5에서 호출한다. 설정값을 현재 상태로 복사한다. 조합할 때 use2D는 한 좌표계로
// 통일한다.
void Reset()
{
    use2D = false;
    progress = 0;
    open = false;
}
// [키 입력] 실행 플래그나 상태를 바꾼다. 두 기능이 같은 키를 쓰면 등록부에서 키를 분배한다.
void Key(int key, int action, int)
{
    if (key == GLFW_KEY_O && action == GLFW_PRESS)
        open = !open;
}
// [시간 갱신] dt는 지난 프레임부터 흐른 초다. 초당 속도에는 dt를 곱한다. 그리기 전에 한 번
// 호출한다.
void Update(float dt)
{
    progress += std::clamp((open ? 1.0f : 0.0f) - progress, -dt / Duration, dt / Duration);
}
glm::mat4 Model()
{
    // 진행률이 커질수록 높이가 줄어든다. 중심을 -height/2로 옮겨 윗모서리를 원점에 맞춘다.
    // 그 뒤 경첩 회전과 경첩의 월드 위치를 적용하므로 문이 줄어도 윗부분은 고정된다.
    float height = DoorHeight * (1 - progress);
    return T({0, DoorHeight, 0}) * R(-OpenDegrees * progress, {1, 0, 0}) * T({0, -height / 2, 0}) *
           S({DoorWidth, height, .1f});
}
// [그리기] 상태를 화면에 표현한다. 같은 물체에 다른 기능을 합치면 이 함수에서 그 기능의 계산 결과를
// 사용한다.
void Draw()
{
    Ground();
    Box(T({-DoorWidth / 2 - .2f, DoorHeight / 2, 0}) * S({.2f, DoorHeight, .3f}), Blue);
    Box(T({DoorWidth / 2 + .2f, DoorHeight / 2, 0}) * S({.2f, DoorHeight, .3f}), Blue);
    Box(Model(), Yellow);
}
// [자동 검사] 기본 설정값 기준의 검사다. 시험 조건으로 설정을 바꾸면 기대값도 바꿔야 한다. 일반
// 실행에서는 호출되지 않는다.
void Check()
{
    Reset();
    open = true;
    Update(.5f);
    glm::vec3 top = glm::vec3(Model() * glm::vec4(0, .5f, 0, 1));
    Require(glm::length(top - glm::vec3(0, 4, 0)) < .001f, "shrinking door hinge");
}
} // namespace ShrinkingDoor
