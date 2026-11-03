// [복붙 위치] base.cpp의 FEATURE_BLOCK_END 바로 앞에 namespace 전체를 붙인다.
// [연결] 이 파일만 붙이면 실행되지 않는다. 참고서의 호출 등록부를 기존 On 함수 안에 합친다.
namespace EraseSplit
{
// ===== 1. 시험 조건에 맞게 바꿀 설정값 =====
constexpr float InitialSize =
    160.0f; // [수정] 최초 사각형 한 변. 분할한 조각의 크기는 부모의 절반이다.
constexpr float MinimumSplitSize = 20.0f; // [수정] 이 크기보다 큰 사각형만 분할한다.
constexpr float SplitSpeed =
    40.0f; // [수정] 각 축의 초당 이동량. 대각선 실제 속력은 이 값의 sqrt(2)배이다.
const glm::vec2 EraserSize(50, 50); // [수정] 지우개 너비·높이. 충돌 범위와 테두리가 함께 바뀐다.

// ===== 2. 실행 중 바뀌는 상태와 자료 =====
// visible=false인 도형은 그리기와 이동에서 제외된다. center는 중심, velocity는 초당 이동량이다.
// 배열에 추가할 때 기존 참조를 계속 쓰지 않는다.

struct Item
{
    glm::vec2 center, velocity;
    float size;
    bool visible = true;
};
std::vector<Item> items;
bool erasing = false;
// [초기화] 시작/F5에서 호출한다. 설정값을 현재 상태로 복사한다. 조합할 때 use2D는 한 좌표계로
// 통일한다.
void Reset()
{
    use2D = true;
    items = {{{400, 300}, {0, 0}, InitialSize, true}};
    erasing = false;
}
// [마우스 버튼] 눌림과 놓임을 구분한다. mouseX/Y는 800×600 논리 좌표다.
void Mouse(int button, int action)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        erasing = action == GLFW_PRESS;
        return;
    }
    if (button != GLFW_MOUSE_BUTTON_LEFT || action != GLFW_PRESS)
        return;
    for (int i = int(items.size()) - 1; i >= 0; --i)
        if (items[i].visible && items[i].size > MinimumSplitSize &&
            HitRect(mouseX, mouseY, items[i].center, glm::vec2(items[i].size)))
        {
            Item old = items[i];
            items[i].visible = false; // push_back 이전에 복사한다. 참조 무효화 방지.
            if (items.size() + 4 > 500)
            {
                items[i].visible = true;
                return;
            }
            for (int y = -1; y <= 1; y += 2)
                for (int x = -1; x <= 1; x += 2)
                    items.push_back({old.center + glm::vec2(x, y) * (old.size / 4),
                                     glm::vec2(x, y) * SplitSpeed, old.size / 2, true});
            break;
        }
}
// [시간 갱신] dt는 지난 프레임부터 흐른 초다. 초당 속도에는 dt를 곱한다. 그리기 전에 한 번
// 호출한다.
void Update(float dt)
{
    for (auto &a : items)
        if (a.visible)
        {
            a.center += a.velocity * dt;
            if (erasing && OverlapRect(a.center, glm::vec2(a.size), {mouseX, mouseY}, EraserSize))
                a.visible = false;
        }
}
// [그리기] 상태를 화면에 표현한다. 같은 물체에 다른 기능을 합치면 이 함수에서 그 기능의 계산 결과를
// 사용한다.
void Draw()
{
    for (auto &a : items)
        if (a.visible)
            Rect(a.center, glm::vec2(a.size), Green);
    Rect({mouseX, mouseY}, EraserSize, Yellow, false);
}
// [자동 검사] 기본 설정값 기준의 검사다. 시험 조건으로 설정을 바꾸면 기대값도 바꿔야 한다. 일반
// 실행에서는 호출되지 않는다.
void Check()
{
    Reset();
    mouseX = 400;
    mouseY = 300;
    Mouse(GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS);
    Require(items.size() == 5 && !items[0].visible && Near(items[1].size, 80), "split four");
}
} // namespace EraseSplit
