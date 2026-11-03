// [복붙 위치] base.cpp의 FEATURE_BLOCK_END 바로 앞에 namespace 전체를 붙인다.
// [연결] 이 파일만 붙이면 실행되지 않는다. 참고서의 호출 등록부를 기존 On 함수 안에 합친다.
namespace PulseColor
{
// ===== 1. 시험 조건에 맞게 바꿀 설정값 =====
constexpr float AngularSpeed = 2.0f; // [수정] 위상 속도(라디안/초). 한 왕복 주기는 2*pi/이 값이다.
constexpr float MinimumRadius = .5f, MaximumRadius = 1.5f; // [수정] 구의 최소·최대 반지름.
const glm::vec3 StartColor = Red,
                EndColor = Yellow; // [수정] 두 색 사이를 왕복한다. RGB 각 성분은 0~1.

// ===== 2. 실행 중 바뀌는 상태와 자료 =====
// time은 누적 시간(초), Blend()는 0~1 보간 비율이다. 다른 물체의 색·크기에 적용할 때 Update와
// Blend만 쓰고 Draw는 제외한다.

float time = 0;
bool running = true;
// [초기화] 시작/F5에서 호출한다. 설정값을 현재 상태로 복사한다. 조합할 때 use2D는 한 좌표계로
// 통일한다.
void Reset()
{
    use2D = false;
    time = 0;
    running = true;
}
// [키 입력] 실행 플래그나 상태를 바꾼다. 두 기능이 같은 키를 쓰면 등록부에서 키를 분배한다.
void Key(int key, int action, int)
{
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        running = !running;
}
// [시간 갱신] dt는 지난 프레임부터 흐른 초다. 초당 속도에는 dt를 곱한다. 그리기 전에 한 번
// 호출한다.
void Update(float dt)
{
    if (running)
        time += dt;
}
float Blend()
{
    return .5f + .5f * std::sin(time * AngularSpeed);
}
// [그리기] 상태를 화면에 표현한다. 같은 물체에 다른 기능을 합치면 이 함수에서 그 기능의 계산 결과를
// 사용한다.
void Draw()
{
    float u = Blend();
    float size = glm::mix(MinimumRadius, MaximumRadius, u);
    glm::vec3 color = glm::mix(StartColor, EndColor, u);
    Sphere(S(glm::vec3(size)), color);
}
// [자동 검사] 기본 설정값 기준의 검사다. 시험 조건으로 설정을 바꾸면 기대값도 바꿔야 한다. 일반
// 실행에서는 호출되지 않는다.
void Check()
{
    Reset();
    for (int i = 0; i < 100; ++i)
    {
        Update(.1f);
        Require(Blend() >= 0 && Blend() <= 1, "pulse range");
    }
}
} // namespace PulseColor
