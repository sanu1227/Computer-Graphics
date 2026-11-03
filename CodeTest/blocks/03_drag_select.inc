// [복붙 위치] base.cpp의 FEATURE_BLOCK_END 바로 앞에 namespace 전체를 붙인다.
// [연결] 이 파일만 붙이면 실행되지 않는다. 참고서의 호출 등록부를 기존 On 함수 안에 합친다.
namespace DragSelect
{
// ===== 1. 시험 조건에 맞게 바꿀 설정값 =====
const glm::vec2 InitialSize(
    140, 100); // [수정] 전체 너비·높이. (200,100)이면 가로만 커진다. 클릭 범위도 함께 바뀐다.
const glm::vec2 FirstCenter(300, 250);  // [수정] 첫 도형 중심. x 증가=오른쪽, y 증가=아래쪽.
const glm::vec2 SecondCenter(370, 290); // [수정] 두 번째 도형 중심. 배열 뒤의 도형이 위에 그려진다.

// ===== 2. 실행 중 바뀌는 상태와 자료 =====
// selected=-1은 미선택이다. offset은 클릭점과 도형 중심의 차이여서 잡은 지점이 갑자기 중심으로 튀지
// 않는다. items[selected].center를 다른 기능과 공유할 수 있다.

struct Item
{
    glm::vec2 center; // 물체 중심 좌표. 다른 기능으로 이동시키려면 이 값을 바꾼다.
    glm::vec2 size;   // 전체 너비·높이. 반크기가 아니다. 그리기와 클릭 판정이 같은 값을 읽는다.
    glm::vec3 color;  // RGB. (1,0,0)=빨강, (0,1,0)=초록, (0,0,1)=파랑.
};
std::vector<Item> items;
int selected = -1;
bool dragging = false;
glm::vec2 offset(0);
// [초기화] 시작/F5에서 호출한다. 설정값을 현재 상태로 복사한다. 조합할 때 use2D는 한 좌표계로
// 통일한다.
void Reset()
{
    use2D = true;
    items = {{FirstCenter, InitialSize, Red}, {SecondCenter, InitialSize, Blue}};
    selected = -1;
    dragging = false;
}
// [마우스 버튼] 눌림과 놓임을 구분한다. mouseX/Y는 800×600 논리 좌표다.
void Mouse(int button, int action)
{
    if (button != GLFW_MOUSE_BUTTON_LEFT)
        return;
    if (action == GLFW_RELEASE)
    {
        dragging = false;
        return;
    }
    if (action != GLFW_PRESS)
        return;
    selected = -1;
    // 마지막에 그린 도형부터 검사한다.
    for (int i = int(items.size()) - 1; i >= 0; --i)
        if (HitRect(mouseX, mouseY, items[i].center, items[i].size))
        {
            selected = i;
            offset = items[i].center - glm::vec2(mouseX, mouseY);
            dragging = true;
            break;
        }
}
// [마우스 이동] 클릭 이후의 드래그 상태를 보고 현재 위치를 갱신한다.
void Cursor()
{
    if (!dragging || selected < 0)
        return;
    auto &a = items[selected];
    a.center = glm::vec2(mouseX, mouseY) + offset;
    // 중심만 화면 안에 두면 도형 절반이 밖으로 나간다. 양쪽에서 반크기만큼 여유를 둔다.
    a.center = glm::clamp(a.center, a.size / 2.0f, glm::vec2(800, 600) - a.size / 2.0f);
}
// [그리기] 상태를 화면에 표현한다. 같은 물체에 다른 기능을 합치면 이 함수에서 그 기능의 계산 결과를
// 사용한다.
void Draw()
{
    for (int i = 0; i < int(items.size()); ++i)
    {
        Rect(items[i].center, items[i].size, items[i].color);
        if (i == selected)
            Rect(items[i].center, items[i].size, Yellow, false);
    }
}
// [자동 검사] 기본 설정값 기준의 검사다. 시험 조건으로 설정을 바꾸면 기대값도 바꿔야 한다. 일반
// 실행에서는 호출되지 않는다.
void Check()
{
    Reset();
    mouseX = 350;
    mouseY = 280;
    Mouse(GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS);
    Require(selected == 1, "topmost selection");
    mouseX += 20;
    Cursor();
    Require(Near(items[1].center.x, 390), "drag offset");
}
} // namespace DragSelect
