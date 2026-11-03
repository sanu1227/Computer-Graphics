// [복붙 위치] base.cpp의 FEATURE_BLOCK_END 바로 앞에 namespace 전체를 붙인다.
// [연결] 이 파일만 붙이면 실행되지 않는다. 참고서의 호출 등록부를 기존 On 함수 안에 합친다.
namespace Quadrants
{
// ===== 1. 시험 조건에 맞게 바꿀 설정값 =====
constexpr int MaxPerQuadrant = 5; // [수정] 각 사분면의 최대 생성 개수. 5→10이면 10개까지 생성한다.
constexpr float ItemSize = 60.0f; // [수정] 사각형 한 변의 전체 길이. 0 초과, 300 이하로 둔다.

// ===== 2. 실행 중 바뀌는 상태와 자료 =====
// items는 도형 목록, counts[q]는 사분면별 개수이다. size는 전체 한 변 길이이며 center는 중심이다.

struct Item
{
    glm::vec2 center;
    float size = ItemSize;
    int quadrant;
};
std::vector<Item> items;
int counts[4] = {};
int Quadrant(float x, float y)
{
    return (y >= 300 ? 2 : 0) + (x >= 400 ? 1 : 0);
}
// [초기화] 시작/F5에서 호출한다. 설정값을 현재 상태로 복사한다. 조합할 때 use2D는 한 좌표계로
// 통일한다.
void Reset()
{
    use2D = true;
    items.clear();
    for (int &c : counts)
        c = 0;
}
// [마우스 버튼] 눌림과 놓임을 구분한다. mouseX/Y는 800×600 논리 좌표다.
void Mouse(int button, int action)
{
    if (action != GLFW_PRESS)
        return;
    int q = Quadrant(mouseX, mouseY);
    if (button == GLFW_MOUSE_BUTTON_LEFT && counts[q] < MaxPerQuadrant)
    {
        // 사분면 경계를 넘지 않게 중심을 제한한다. 반크기는 30.
        float left = (q % 2) * 400.0f, top = (q / 2) * 300.0f;
        items.push_back({{std::clamp(mouseX, left + ItemSize / 2, left + 400 - ItemSize / 2),
                          std::clamp(mouseY, top + ItemSize / 2, top + 300 - ItemSize / 2)},
                         ItemSize,
                         q});
        ++counts[q];
    }
}
// [그리기] 상태를 화면에 표현한다. 같은 물체에 다른 기능을 합치면 이 함수에서 그 기능의 계산 결과를
// 사용한다.
void Draw()
{
    glm::vec3 colors[] = {Red, Green, Blue, Yellow};
    for (int q = 0; q < 4; ++q)
        Rect({200.0f + (q % 2) * 400, 150.0f + (q / 2) * 300}, {398, 298}, colors[q] * .2f);
    for (auto &a : items)
        Rect(a.center, {a.size, a.size}, colors[a.quadrant]);
}
// [자동 검사] 기본 설정값 기준의 검사다. 시험 조건으로 설정을 바꾸면 기대값도 바꿔야 한다. 일반
// 실행에서는 호출되지 않는다.
void Check()
{
    Reset();
    mouseX = 100;
    mouseY = 100;
    for (int i = 0; i < 7; ++i)
        Mouse(GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS);
    Require(items.size() == 5 && Quadrant(500, 400) == 3, "quadrant limit");
}
} // namespace Quadrants
