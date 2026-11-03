// [복붙 위치] base.cpp의 FEATURE_BLOCK_END 바로 앞에 namespace 전체를 붙인다.
// [연결] 이 파일만 붙이면 실행되지 않는다. 참고서의 호출 등록부를 기존 On 함수 안에 합친다.
namespace MultiView
{
// ===== 1. 시험 조건에 맞게 바꿀 설정값 =====
constexpr float LeftRatio =
    2.0f / 3.0f;                // [수정] 왼쪽 원근도 폭 비율(0~1). .5면 화면 절반을 사용한다.
constexpr float MoveStep = .2f; // [수정] 모든 뷰에 공유하는 객체의 이동량.

// ===== 2. 실행 중 바뀌는 상태와 자료 =====
// position 한 개를 세 뷰에서 공유한다. Scene()은 물체를 그리고 Draw()는 카메라와 영역만 바꾼다.
// 다른 장면을 쓰려면 Scene 내용만 교체한다.

glm::vec3 position(0);
// [초기화] 시작/F5에서 호출한다. 설정값을 현재 상태로 복사한다. 조합할 때 use2D는 한 좌표계로
// 통일한다.
void Reset()
{
    use2D = false;
    position = {0, 0, 0};
}
// [키 입력] 실행 플래그나 상태를 바꾼다. 두 기능이 같은 키를 쓰면 등록부에서 키를 분배한다.
void Key(int key, int action, int)
{
    if (action != GLFW_PRESS && action != GLFW_REPEAT)
        return;
    if (key == GLFW_KEY_LEFT)
        position.x -= MoveStep;
    if (key == GLFW_KEY_RIGHT)
        position.x += MoveStep;
}
void Scene()
{
    Ground();
    Box(T(position + glm::vec3(0, .5f, 0)) * S({1, 1, 2}), Yellow);
    Box(T(position + glm::vec3(0, 1.2f, .8f)) * S({.2f, .2f, 2}), Blue);
}
// [그리기] 상태를 화면에 표현한다. 같은 물체에 다른 기능을 합치면 이 함수에서 그 기능의 계산 결과를
// 사용한다.
void Draw()
{
    int w = framebufferWidth, h = framebufferHeight, left = int(w * LeftRatio);
    if (w < 4 || h < 4)
        return;
    // 전체 화면을 한 번 지웠으므로 겹치지 않는 viewport마다 glClear를 다시 호출하지 않는다.
    SetView(0, 0, left, h, {7, 6, 9}, {0, 0, 0}, {0, 1, 0}, true);
    Scene();
    SetView(left, h / 2, w - left, h - h / 2, {0, 15, 0}, {0, 0, 0}, {0, 0, -1}, false);
    Scene();
    SetView(left, 0, w - left, h / 2, {0, 3, 15}, {0, 0, 0}, {0, 1, 0}, false);
    Scene();
}
// [자동 검사] 기본 설정값 기준의 검사다. 시험 조건으로 설정을 바꾸면 기대값도 바꿔야 한다. 일반
// 실행에서는 호출되지 않는다.
void Check()
{
    Reset();
    Key(GLFW_KEY_RIGHT, GLFW_PRESS, 0);
    Require(Near(position.x, .2f), "shared viewport state");
}
} // namespace MultiView
