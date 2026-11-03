// [복붙 위치] base.cpp의 FEATURE_BLOCK_END 바로 앞에 namespace 전체를 붙인다.
// [연결] 이 파일만 붙이면 실행되지 않는다. 참고서의 호출 등록부를 기존 On 함수 안에 합친다.
namespace SnapMatch
{
// ===== 1. 시험 조건에 맞게 바꿀 설정값 =====
const glm::vec2 PieceSize(80, 80);    // [수정] 이동할 도형 크기. 그림과 클릭 판정에 같이 사용한다.
constexpr float SnapDistance = 45.0f; // [수정] 중심 간 허용 거리. 키우면 멀리서 놓아도 붙는다.
const glm::vec2 TargetCenter(600,
                             300); // [수정] 붙일 목표점. 목표 테두리와 최종 위치가 함께 바뀐다.

// ===== 2. 실행 중 바뀌는 상태와 자료 =====
// position은 움직일 도형 중심, slot은 목표 중심이다. locked=true이면 다시 드래그하지 않는다. 다중
// 객체 조합에는 TrySnap 함수를 재사용한다.

glm::vec2 position(180, 300), slot(600, 300), offset(0);
bool dragging = false, locked = false;
float threshold = SnapDistance; // 실행 중 바꾸고 싶으면 이 값을 수정한다.
// [조합용 계산 함수] 전달받은 '실제 물체의 위치'를 수정한다. 별도 물체를 만들지 않는다.
// 예: TrySnap(DragSelect::items[i].center, TargetCenter, SnapDistance);
// true이면 목표점에 붙었고, false이면 위치를 그대로 유지한다.
bool TrySnap(glm::vec2 &objectPosition, glm::vec2 targetPosition, float allowedDistance)
{
    if (glm::length(objectPosition - targetPosition) > allowedDistance)
        return false;
    objectPosition = targetPosition;
    return true;
}
// [초기화] 시작/F5에서 호출한다. 설정값을 현재 상태로 복사한다. 조합할 때 use2D는 한 좌표계로
// 통일한다.
void Reset()
{
    use2D = true;
    position = {180, 300};
    slot = TargetCenter;
    threshold = SnapDistance;
    dragging = locked = false;
}
// [마우스 버튼] 눌림과 놓임을 구분한다. mouseX/Y는 800×600 논리 좌표다.
void Mouse(int button, int action)
{
    if (button != GLFW_MOUSE_BUTTON_LEFT)
        return;
    if (action == GLFW_PRESS && !locked && HitRect(mouseX, mouseY, position, PieceSize))
    {
        dragging = true;
        offset = position - glm::vec2(mouseX, mouseY);
    }
    if (action == GLFW_RELEASE)
    {
        if (dragging && TrySnap(position, slot, threshold))
        {
            locked = true;
        }
        dragging = false;
    }
}
// [마우스 이동] 클릭 이후의 드래그 상태를 보고 현재 위치를 갱신한다.
void Cursor()
{
    if (dragging)
        position = glm::vec2(mouseX, mouseY) + offset;
}
// [그리기] 상태를 화면에 표현한다. 같은 물체에 다른 기능을 합치면 이 함수에서 그 기능의 계산 결과를
// 사용한다.
void Draw()
{
    Rect(slot, PieceSize + glm::vec2(10), Yellow, false);
    Rect(position, PieceSize, locked ? Green : Blue);
}
// [자동 검사] 기본 설정값 기준의 검사다. 시험 조건으로 설정을 바꾸면 기대값도 바꿔야 한다. 일반
// 실행에서는 호출되지 않는다.
void Check()
{
    Reset();
    dragging = true;
    position = slot + glm::vec2(20, 0);
    Mouse(GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE);
    Require(locked && Near(position.x, slot.x), "snap distance");
    glm::vec2 farPoint(0, 0);
    Require(!TrySnap(farPoint, TargetCenter, SnapDistance) && Near(farPoint.x, 0),
            "outside snap range unchanged");
}
} // namespace SnapMatch
