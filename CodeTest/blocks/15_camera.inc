// [복붙 위치] base.cpp의 FEATURE_BLOCK_END 바로 앞에 namespace 전체를 붙인다.
// [연결] 이 파일만 붙이면 실행되지 않는다. 참고서의 호출 등록부를 기존 On 함수 안에 합친다.
namespace Camera
{
// ===== 1. 시험 조건에 맞게 바꿀 설정값 =====
constexpr float MoveStep =
    .3f; // [수정] 카메라가 키 한 번에 이동하는 거리. eye와 target을 같이 이동한다.
constexpr float TurnStep = 5.0f;        // [수정] 키 한 번의 자전·공전 각도(도).
const glm::vec3 InitialEye(6, 5, 9);    // [수정] 시작 카메라 위치. y가 커지면 더 높은 곳에서 본다.
const glm::vec3 InitialTarget(0, 0, 0); // [수정] 처음 바라볼 점. InitialEye와 같으면 안 된다.

// ===== 2. 실행 중 바뀌는 상태와 자료 =====
// eye·target·cameraUp·perspective는 공통 바탕의 공유 변수다. 조합에서는 Reset과 Key만 등록하고
// 예제용 Draw는 제외한다.

// [초기화] 시작/F5에서 호출한다. 설정값을 현재 상태로 복사한다. 조합할 때 use2D는 한 좌표계로
// 통일한다.
void Reset()
{
    use2D = false;
    eye = InitialEye;
    target = InitialTarget;
    cameraUp = {0, 1, 0};
    perspective = true;
}
// [키 입력] 실행 플래그나 상태를 바꾼다. 두 기능이 같은 키를 쓰면 등록부에서 키를 분배한다.
void Key(int key, int action, int mods)
{
    if (action != GLFW_PRESS && action != GLFW_REPEAT)
        return;
    float sign = (mods & GLFW_MOD_SHIFT) ? -1.0f : 1.0f;
    glm::vec3 forward = glm::normalize(target - eye),
              right = glm::normalize(glm::cross(forward, cameraUp));
    if (key == GLFW_KEY_X)
    {
        eye += right * MoveStep * sign;
        target += right * MoveStep * sign;
    }
    if (key == GLFW_KEY_Z)
    {
        eye += forward * MoveStep * sign;
        target += forward * MoveStep * sign;
    }
    if (key == GLFW_KEY_Y)
        target = eye + glm::vec3(R(TurnStep * sign, {0, 1, 0}) *
                                 glm::vec4(target - eye, 0)); // 제자리 시선 회전.
    if (key == GLFW_KEY_O)
        eye = target + glm::vec3(R(TurnStep * sign, {0, 1, 0}) *
                                 glm::vec4(eye - target, 0)); // target 주위 공전.
    if (key == GLFW_KEY_P && action == GLFW_PRESS)
        perspective = !perspective;
}
// [그리기] 상태를 화면에 표현한다. 같은 물체에 다른 기능을 합치면 이 함수에서 그 기능의 계산 결과를
// 사용한다.
void Draw()
{
    Ground();
    Box(T({-2, .5f, 0}), Red);
    Box(T({2, .5f, 0}), Blue);
}
// [자동 검사] 기본 설정값 기준의 검사다. 시험 조건으로 설정을 바꾸면 기대값도 바꿔야 한다. 일반
// 실행에서는 호출되지 않는다.
void Check()
{
    Reset();
    float distance = glm::length(target - eye);
    Key(GLFW_KEY_O, GLFW_PRESS, 0);
    Require(Near(glm::length(target - eye), distance), "camera orbit distance");
    auto d = target - eye;
    Key(GLFW_KEY_X, GLFW_PRESS, 0);
    Require(glm::length((target - eye) - d) < .001f, "camera translation direction");
}
} // namespace Camera
