// [복붙 위치] base.cpp의 FEATURE_BLOCK_END 바로 앞에 namespace 전체를 붙인다.
// [연결] 이 파일만 붙이면 실행되지 않는다. 참고서의 호출 등록부를 기존 On 함수 안에 합친다.
namespace Primitives
{
// ===== 1. 시험 조건에 맞게 바꿀 설정값 =====
constexpr float InitialSize =
    100.0f;                       // [수정] 사각형 한 변, 원의 지름, 삼각형의 가로·세로 배율 기준.
constexpr float SizeStep = 10.0f; // [수정] 오른쪽 클릭 한 번의 크기 변화량.
constexpr float MinimumSize = 30.0f,
                MaximumSize = 150.0f; // [수정] 왕복 범위. 최소 < 초기 <= 최대로 둔다.

// ===== 2. 실행 중 바뀌는 상태와 자료 =====
// shape는 0=점, 1=선, 2=삼각형, 3=사각형, 4=원이다. direction=1은 확대, -1은 축소다. 점 크기는
// glPointSize(8)에서 따로 바꾼다.

int shape = 3;
bool filled = true;
float size = InitialSize, direction = 1;
// [초기화] 시작/F5에서 호출한다. 설정값을 현재 상태로 복사한다. 조합할 때 use2D는 한 좌표계로
// 통일한다.
void Reset()
{
    use2D = true;
    shape = 3;
    filled = true;
    size = InitialSize;
    direction = 1;
}
// [키 입력] 실행 플래그나 상태를 바꾼다. 두 기능이 같은 키를 쓰면 등록부에서 키를 분배한다.
void Key(int key, int action, int)
{
    if (action != GLFW_PRESS)
        return;
    if (key >= GLFW_KEY_1 && key <= GLFW_KEY_5)
        shape = key - GLFW_KEY_1;
    if (key == GLFW_KEY_F)
        filled = !filled;
}
// [마우스 버튼] 눌림과 놓임을 구분한다. mouseX/Y는 800×600 논리 좌표다.
void Mouse(int button, int action)
{
    if (button != GLFW_MOUSE_BUTTON_RIGHT || action != GLFW_PRESS)
        return;
    size += direction * SizeStep;
    if (size >= MaximumSize)
    {
        size = MaximumSize;
        direction = -1;
    }
    if (size <= MinimumSize)
    {
        size = MinimumSize;
        direction = 1;
    }
}
// [그리기] 상태를 화면에 표현한다. 같은 물체에 다른 기능을 합치면 이 함수에서 그 기능의 계산 결과를
// 사용한다.
void Draw()
{
    glm::mat4 model = T({400, 300, 0}) * S({size, size, 1});
    if (shape == 0)
    {
        glPointSize(8);
        Primitive(GL_POINTS, {{0, 0, 0}}, model, Yellow);
    }
    if (shape == 1)
        Primitive(GL_LINES, {{-.5f, 0, 0}, {.5f, 0, 0}}, model, Yellow);
    if (shape == 2)
        Primitive(filled ? GL_TRIANGLES : GL_LINE_LOOP,
                  {{0, -.5f, 0}, {-.5f, .5f, 0}, {.5f, .5f, 0}}, model, Green);
    if (shape == 3)
        Rect({400, 300}, {size, size}, Blue, filled);
    if (shape == 4)
        Circle({400, 300}, size / 2, Red, filled);
}
// [자동 검사] 기본 설정값 기준의 검사다. 시험 조건으로 설정을 바꾸면 기대값도 바꿔야 한다. 일반
// 실행에서는 호출되지 않는다.
void Check()
{
    Reset();
    size = 150;
    direction = 1;
    Mouse(GLFW_MOUSE_BUTTON_RIGHT, GLFW_PRESS);
    Require(Near(size, 150) && direction == -1, "size reversal");
}
} // namespace Primitives
