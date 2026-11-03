// [복붙 위치] base.cpp의 FEATURE_BLOCK_END 바로 앞에 namespace 전체를 붙인다.
// [연결] 이 파일만 붙이면 실행되지 않는다. 참고서의 호출 등록부를 기존 On 함수 안에 합친다.
namespace PyramidSequence
{
// ===== 1. 시험 조건에 맞게 바꿀 설정값 =====
constexpr float OpenAngle =
    120.0f; // [수정] 면별 목표 펼침각(도). 바닥과 맞닿는 각도는 정점 구조에 따라 다르다.
constexpr float OpenSpeed =
    60.0f; // [수정] 면이 열리는 속도(도/초). 순차 대기 시간도 함께 달라진다.

// ===== 2. 실행 중 바뀌는 상태와 자료 =====
// angles[i]는 각 면의 현재 각도다. sequential=true일 때 앞 면의 각도를 확인하고 다음 면을 시작한다.
// 닫기는 동시에 수행한다.

float angles[4] = {};
bool open = false, sequential = false;
// [초기화] 시작/F5에서 호출한다. 설정값을 현재 상태로 복사한다. 조합할 때 use2D는 한 좌표계로
// 통일한다.
void Reset()
{
    use2D = false;
    for (float &a : angles)
        a = 0;
    open = sequential = false;
}
// [키 입력] 실행 플래그나 상태를 바꾼다. 두 기능이 같은 키를 쓰면 등록부에서 키를 분배한다.
void Key(int key, int action, int)
{
    if (action != GLFW_PRESS)
        return;
    if (key == GLFW_KEY_O)
        open = !open;
    if (key == GLFW_KEY_S)
        sequential = !sequential;
}
// [시간 갱신] dt는 지난 프레임부터 흐른 초다. 초당 속도에는 dt를 곱한다. 그리기 전에 한 번
// 호출한다.
void Update(float dt)
{
    for (int i = 0; i < 4; ++i)
    {
        // 순차 펼치기는 앞 면이 목표각에 도달한 뒤 다음 면을 움직인다.
        if (open && sequential && i > 0 && angles[i - 1] < OpenAngle - .1f)
            break;
        angles[i] +=
            std::clamp((open ? OpenAngle : 0.0f) - angles[i], -OpenSpeed * dt, OpenSpeed * dt);
    }
}
// [그리기] 상태를 화면에 표현한다. 같은 물체에 다른 기능을 합치면 이 함수에서 그 기능의 계산 결과를
// 사용한다.
void Draw()
{
    Primitive(GL_TRIANGLE_FAN, {{-1, 0, -1}, {1, 0, -1}, {1, 0, 1}, {-1, 0, 1}}, glm::mat4(1),
              Green);
    for (int i = 0; i < 4; ++i)
    {
        glm::vec3 pivot(0, 0, 1);
        glm::mat4 model = R(i * 90.0f, {0, 1, 0}) * T(pivot) * R(angles[i], {1, 0, 0}) * T(-pivot);
        Primitive(GL_TRIANGLES, {{-1, 0, 1}, {1, 0, 1}, {0, 2, 0}}, model, i % 2 ? Yellow : Blue);
    }
}
// [자동 검사] 기본 설정값 기준의 검사다. 시험 조건으로 설정을 바꾸면 기대값도 바꿔야 한다. 일반
// 실행에서는 호출되지 않는다.
void Check()
{
    Reset();
    open = sequential = true;
    Update(.5f);
    Require(Near(angles[0], 30) && Near(angles[1], 0), "sequential opening");
}
} // namespace PyramidSequence
