// [복붙 위치] base.cpp의 FEATURE_BLOCK_END 바로 앞에 namespace 전체를 붙인다.
// [연결] 이 파일만 붙이면 실행되지 않는다. 참고서의 호출 등록부를 기존 On 함수 안에 합친다.
namespace JointModel
{
// ===== 1. 시험 조건에 맞게 바꿀 설정값 =====
constexpr float MoveStep = .2f;  // [수정] 키 입력 한 번에 탱크가 움직이는 거리.
constexpr float TurnStep = 5.0f; // [수정] 키 입력 한 번에 부품이 회전하는 각도(도).
constexpr float BarrelLength =
    2.0f; // [수정] 포신 길이. 중심 오프셋도 자동으로 절반만큼 이동해 관절에 붙는다.
constexpr float BarrelThickness = .25f; // [수정] 포신 가로·세로 두께. 길이와 독립적이다.

// ===== 2. 실행 중 바뀌는 상태와 자료 =====
// position은 탱크 전체 위치다. bodyAngle은 몸체, turretAngle은 몸체 기준 포탑 각도다. Turret()에
// 자식을 붙이면 부모 이동·회전을 따라간다.

glm::vec3 position(0);
float bodyAngle = 0, turretAngle = 0, barrelAngle = 0;
// [초기화] 시작/F5에서 호출한다. 설정값을 현재 상태로 복사한다. 조합할 때 use2D는 한 좌표계로
// 통일한다.
void Reset()
{
    use2D = false;
    position = glm::vec3(0);
    bodyAngle = turretAngle = barrelAngle = 0;
}
// [키 입력] 실행 플래그나 상태를 바꾼다. 두 기능이 같은 키를 쓰면 등록부에서 키를 분배한다.
void Key(int key, int action, int mods)
{
    if (action != GLFW_PRESS && action != GLFW_REPEAT)
        return;
    float d = (mods & GLFW_MOD_SHIFT) ? -TurnStep : TurnStep;
    if (key == GLFW_KEY_Y)
        bodyAngle += d;
    if (key == GLFW_KEY_T)
        turretAngle += d;
    if (key == GLFW_KEY_B)
        barrelAngle = std::clamp(barrelAngle + d, -45.0f, 45.0f);
    if (key == GLFW_KEY_LEFT)
        position.x -= MoveStep;
    if (key == GLFW_KEY_RIGHT)
        position.x += MoveStep;
}
glm::mat4 Turret()
{
    return T(position) * R(bodyAngle, {0, 1, 0}) * T({0, 1, 0}) * R(turretAngle, {0, 1, 0});
}
// [그리기] 상태를 화면에 표현한다. 같은 물체에 다른 기능을 합치면 이 함수에서 그 기능의 계산 결과를
// 사용한다.
void Draw()
{
    Ground();
    glm::mat4 root = T(position) * R(bodyAngle, {0, 1, 0});
    Box(root * T({0, .3f, 0}) * S({2, .6f, 3}), Green);
    glm::mat4 turret = Turret();
    Box(turret * S({1.2f, .6f, 1.2f}), Blue);
    glm::mat4 joint = turret * T({0, 0, .6f}) * R(barrelAngle, {1, 0, 0});
    Box(joint * T({0, 0, BarrelLength / 2}) * S({BarrelThickness, BarrelThickness, BarrelLength}),
        Yellow);
}
// [자동 검사] 기본 설정값 기준의 검사다. 시험 조건으로 설정을 바꾸면 기대값도 바꿔야 한다. 일반
// 실행에서는 호출되지 않는다.
void Check()
{
    Reset();
    position.x = 3;
    auto p = glm::vec3(Turret() * glm::vec4(0, 0, 0, 1));
    Require(Near(p.x, 3) && Near(p.y, 1), "parent transform");
}
} // namespace JointModel
