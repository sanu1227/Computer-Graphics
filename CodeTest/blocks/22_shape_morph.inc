// [복붙 위치] base.cpp의 FEATURE_BLOCK_END 바로 앞에 namespace 전체를 붙인다.
// [연결] 이 파일만 붙이면 실행되지 않는다. 참고서의 호출 등록부를 기존 On 함수 안에 합친다.
namespace ShapeMorph
{
// ===== 1. 시험 조건에 맞게 바꿀 설정값 =====
constexpr float ShapeHeight =
    2.0f; // [수정] 꼭대기 높이. 위쪽 네 정점과 사각뿔 꼭짓점에 같이 적용한다.
constexpr float Duration = 1.0f; // [수정] 모양 변환 시간(초). 2로 바꾸면 두 배 느리게 변한다.

// ===== 2. 실행 중 바뀌는 상태와 자료 =====
// progress=0이면 위쪽 네 정점이 한 꼭짓점에 모여 사각뿔, 1이면 분리되어 육면체다. Top(i)는 진행률에
// 따른 위쪽 정점을 돌려준다.

float progress = 0;
bool cube = false;
// [초기화] 시작/F5에서 호출한다. 설정값을 현재 상태로 복사한다. 조합할 때 use2D는 한 좌표계로
// 통일한다.
void Reset()
{
    use2D = false;
    progress = 0;
    cube = false;
}
// [키 입력] 실행 플래그나 상태를 바꾼다. 두 기능이 같은 키를 쓰면 등록부에서 키를 분배한다.
void Key(int key, int action, int)
{
    if (key == GLFW_KEY_M && action == GLFW_PRESS)
        cube = !cube;
}
// [시간 갱신] dt는 지난 프레임부터 흐른 초다. 초당 속도에는 dt를 곱한다. 그리기 전에 한 번
// 호출한다.
void Update(float dt)
{
    progress += std::clamp((cube ? 1.0f : 0.0f) - progress, -dt / Duration, dt / Duration);
}
glm::vec3 Top(int i)
{
    const glm::vec3 corners[] = {
        {-1, ShapeHeight, 1}, {1, ShapeHeight, 1}, {1, ShapeHeight, -1}, {-1, ShapeHeight, -1}};
    return glm::mix(glm::vec3(0, ShapeHeight, 0), corners[i], progress);
}
// [그리기] 상태를 화면에 표현한다. 같은 물체에 다른 기능을 합치면 이 함수에서 그 기능의 계산 결과를
// 사용한다.
void Draw()
{
    const glm::vec3 bottom[] = {{-1, 0, 1}, {1, 0, 1}, {1, 0, -1}, {-1, 0, -1}};
    for (int i = 0; i < 4; ++i)
    {
        int j = (i + 1) % 4;
        Primitive(GL_TRIANGLE_FAN, {bottom[i], bottom[j], Top(j), Top(i)}, glm::mat4(1),
                  i % 2 ? Green : Blue);
    }
    Primitive(GL_TRIANGLE_FAN, {Top(0), Top(1), Top(2), Top(3)}, glm::mat4(1), Yellow);
    Primitive(GL_TRIANGLE_FAN, {bottom[3], bottom[2], bottom[1], bottom[0]}, glm::mat4(1), Red);
}
// [자동 검사] 기본 설정값 기준의 검사다. 시험 조건으로 설정을 바꾸면 기대값도 바꿔야 한다. 일반
// 실행에서는 호출되지 않는다.
void Check()
{
    Reset();
    Require(glm::length(Top(0) - Top(1)) < .001f, "pyramid apex");
    cube = true;
    Update(1);
    Require(Near(Top(0).x, -1) && Near(Top(1).x, 1), "cube corners");
}
} // namespace ShapeMorph
