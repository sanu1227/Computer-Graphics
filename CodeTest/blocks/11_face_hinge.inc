// [복붙 위치] base.cpp의 FEATURE_BLOCK_END 바로 앞에 namespace 전체를 붙인다.
// [연결] 이 파일만 붙이면 실행되지 않는다. 참고서의 호출 등록부를 기존 On 함수 안에 합친다.
namespace FaceHinge
{
// ===== 1. 시험 조건에 맞게 바꿀 설정값 =====
constexpr float OpenAngle = 100.0f; // [수정] 완전히 열었을 때 각도(도). 90이면 직각으로 열린다.
constexpr float OpenSpeed = 80.0f;  // [수정] 초당 회전 각도. 양수로 둔다.
const glm::vec3 Pivot(0, 1, 1);     // [수정] 원본 정점 기준 경첩 위치. 현재 앞면 윗모서리 중심이다.

// ===== 2. 실행 중 바뀌는 상태와 자료 =====
// visible은 앞/오른쪽/뒤/왼쪽/위/아래 6면의 표시 여부다. angle은 현재 각도이고 open은 목표 상태라서
// 둘을 구분한다.

float angle = 0;
bool open = false;
bool visible[6] = {true, true, true, true, true, true};
const glm::vec3 points[8] = {{-1, -1, 1},  {1, -1, 1},  {1, 1, 1},  {-1, 1, 1},
                             {-1, -1, -1}, {1, -1, -1}, {1, 1, -1}, {-1, 1, -1}};
const int indices[6][4] = {{0, 1, 2, 3}, {1, 5, 6, 2}, {5, 4, 7, 6},
                           {4, 0, 3, 7}, {3, 2, 6, 7}, {4, 5, 1, 0}};
// [초기화] 시작/F5에서 호출한다. 설정값을 현재 상태로 복사한다. 조합할 때 use2D는 한 좌표계로
// 통일한다.
void Reset()
{
    use2D = false;
    open = false;
    angle = 0;
    for (bool &v : visible)
        v = true;
}
glm::mat4 Hinge()
{
    glm::vec3 pivot = Pivot;
    return T(pivot) * R(-angle, {1, 0, 0}) * T(-pivot);
}
// [키 입력] 실행 플래그나 상태를 바꾼다. 두 기능이 같은 키를 쓰면 등록부에서 키를 분배한다.
void Key(int key, int action, int)
{
    if (action != GLFW_PRESS)
        return;
    if (key == GLFW_KEY_O)
        open = !open;
    if (key >= GLFW_KEY_1 && key <= GLFW_KEY_6)
        visible[key - GLFW_KEY_1] = !visible[key - GLFW_KEY_1];
}
// [시간 갱신] dt는 지난 프레임부터 흐른 초다. 초당 속도에는 dt를 곱한다. 그리기 전에 한 번
// 호출한다.
void Update(float dt)
{
    angle += std::clamp((open ? OpenAngle : 0.0f) - angle, -OpenSpeed * dt, OpenSpeed * dt);
}
// [그리기] 상태를 화면에 표현한다. 같은 물체에 다른 기능을 합치면 이 함수에서 그 기능의 계산 결과를
// 사용한다.
void Draw()
{
    for (int f = 0; f < 6; ++f)
        if (visible[f])
            Primitive(GL_TRIANGLE_FAN,
                      {points[indices[f][0]], points[indices[f][1]], points[indices[f][2]],
                       points[indices[f][3]]},
                      f == 0 ? Hinge() : glm::mat4(1), f == 0 ? Yellow : Blue * (1 - .1f * f));
}
// [자동 검사] 기본 설정값 기준의 검사다. 시험 조건으로 설정을 바꾸면 기대값도 바꿔야 한다. 일반
// 실행에서는 호출되지 않는다.
void Check()
{
    Reset();
    open = true;
    Update(2);
    glm::vec3 p = glm::vec3(Hinge() * glm::vec4(0, 1, 1, 1));
    Require(Near(angle, 100) && glm::length(p - glm::vec3(0, 1, 1)) < .001f, "hinge fixed point");
}
} // namespace FaceHinge
