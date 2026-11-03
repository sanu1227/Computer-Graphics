// [복붙 위치] base.cpp의 FEATURE_BLOCK_END 바로 앞에 namespace 전체를 붙인다.
// [연결] 이 파일만 붙이면 실행되지 않는다. 참고서의 호출 등록부를 기존 On 함수 안에 합친다.
namespace RobotJump
{
// ===== 1. 시험 조건에 맞게 바꿀 설정값 =====
constexpr float WalkSpeed = 2.0f; // [수정] 초기 보행 속도(월드 거리/초). 실행 중 speed에 복사된다.
constexpr float JumpSpeed = 5.0f; // [수정] 점프 시작 상승 속도. 커지면 더 높이·오래 뛴다.
constexpr float Gravity = 9.8f;   // [수정] 중력 크기. 커지면 상승·낙하 시간이 짧아진다.
constexpr float SwingDegrees = 30.0f; // [수정] 팔·다리 최대 흔들림 각도(도).
constexpr float GaitFrequency =
    4.0f; // [수정] 보행 위상 증가량. 커지면 같은 속도에서 발을 더 자주 움직인다.

// ===== 2. 실행 중 바뀌는 상태와 자료 =====
// position.y는 발밑 높이다. verticalSpeed>0은 상승, <0은 하강이다. grounded는 착지 상태, gait는
// 팔·다리 sin 입력인 라디안 위상이다. 장애물 높이를 바꾸려면 바닥에 놓일 중심 높이도 높이의
// 절반으로 맞춘다.

glm::vec3 position(0), direction(0, 0, 1);
float speed = WalkSpeed, verticalSpeed = 0, gait = 0;
bool walking = false, grounded = true;
const glm::vec3 obstacleCenter(2, .5f, 0), obstacleSize(1.5f, 1, 1.5f);
constexpr float Radius = .45f;
bool Overlaps(glm::vec3 p)
{
    return std::abs(p.x - obstacleCenter.x) < Radius + obstacleSize.x / 2 &&
           std::abs(p.z - obstacleCenter.z) < Radius + obstacleSize.z / 2;
}
// [초기화] 시작/F5에서 호출한다. 설정값을 현재 상태로 복사한다. 조합할 때 use2D는 한 좌표계로
// 통일한다.
void Reset()
{
    use2D = false;
    position = {0, 0, 0};
    direction = {0, 0, 1};
    speed = WalkSpeed;
    verticalSpeed = gait = 0;
    walking = false;
    grounded = true;
}
// [키 입력] 실행 플래그나 상태를 바꾼다. 두 기능이 같은 키를 쓰면 등록부에서 키를 분배한다.
void Key(int key, int action, int)
{
    if (action != GLFW_PRESS && action != GLFW_REPEAT)
        return;
    if (key == GLFW_KEY_W)
    {
        direction = {0, 0, -1};
        walking = true;
    }
    if (key == GLFW_KEY_S)
    {
        direction = {0, 0, 1};
        walking = true;
    }
    if (key == GLFW_KEY_A)
    {
        direction = {-1, 0, 0};
        walking = true;
    }
    if (key == GLFW_KEY_D)
    {
        direction = {1, 0, 0};
        walking = true;
    }
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        walking = !walking;
    if (key == GLFW_KEY_J && action == GLFW_PRESS && grounded)
    {
        verticalSpeed = JumpSpeed;
        grounded = false;
    }
    if (key == GLFW_KEY_EQUAL)
        speed = std::min(5.0f, speed + .2f);
    if (key == GLFW_KEY_MINUS)
        speed = std::max(.2f, speed - .2f);
}
// [시간 갱신] dt는 지난 프레임부터 흐른 초다. 초당 속도에는 dt를 곱한다. 그리기 전에 한 번
// 호출한다.
void Update(float dt)
{
    int steps = std::max(1, int(std::ceil(dt / .005f)));
    float h = dt / steps;
    for (int i = 0; i < steps; ++i)
    {
        float oldY =
            position
                .y; // 장애물 윗면을 '위에서 내려오며' 통과했는지 판단하려고 이전 높이를 보관한다.
        verticalSpeed -= Gravity * h; // 중력이 속도를 줄이고, 다음 줄에서 그 속도로 위치를 바꾼다.
        position.y += verticalSpeed * h;
        glm::vec3 next = position;
        if (walking)
            next += direction * speed * h;
        bool blocked = std::abs(next.x) > 4.5f || std::abs(next.z) > 4.5f ||
                       (Overlaps(next) && position.y < obstacleCenter.y + obstacleSize.y / 2 &&
                        oldY < obstacleCenter.y + obstacleSize.y / 2);
        if (blocked)
            direction = -direction;
        else
        {
            position.x = next.x;
            position.z = next.z;
        }
        float support = Overlaps(position) && oldY >= obstacleCenter.y + obstacleSize.y / 2 - .001f
                            ? obstacleCenter.y + obstacleSize.y / 2
                            : 0.0f;
        grounded = false;
        if (position.y <= support && verticalSpeed <= 0)
        {
            position.y = support;
            verticalSpeed = 0;
            grounded = true;
        }
        if (walking)
            gait += speed * h * GaitFrequency;
    }
}
// [그리기] 상태를 화면에 표현한다. 같은 물체에 다른 기능을 합치면 이 함수에서 그 기능의 계산 결과를
// 사용한다.
void Draw()
{
    Ground();
    Box(T(obstacleCenter) * S(obstacleSize), Red);
    float yaw = glm::degrees(std::atan2(direction.x, direction.z));
    glm::mat4 root =
        T(position) * R(yaw, {0, 1, 0}); // 몸통·머리·팔다리가 공유하는 전체 위치와 방향.
    Box(root * T({0, 1.4f, 0}) * S({.7f, .8f, .4f}), Blue);
    Box(root * T({0, 2, 0}) * S({.5f, .5f, .5f}), Yellow);
    float swing = walking ? SwingDegrees * std::sin(gait) : 0;
    for (int side : {-1, 1})
    {
        glm::mat4 arm = root * T({side * .5f, 1.7f, 0}) * R(side * swing, {1, 0, 0});
        Box(arm * T({0, -.35f, 0}) * S({.2f, .7f, .2f}), Green);
        glm::mat4 leg = root * T({side * .2f, 1, 0}) * R(-side * swing, {1, 0, 0});
        Box(leg * T({0, -.5f, 0}) * S({.25f, 1, .25f}), Yellow);
    }
}
// [자동 검사] 기본 설정값 기준의 검사다. 시험 조건으로 설정을 바꾸면 기대값도 바꿔야 한다. 일반
// 실행에서는 호출되지 않는다.
void Check()
{
    Reset();
    Key(GLFW_KEY_J, GLFW_PRESS, 0);
    Update(.1f);
    Require(position.y > 0 && !grounded, "jump launch");
    for (int i = 0; i < 120; ++i)
        Update(1.0f / 60);
    Require(grounded && Near(position.y, 0), "jump landing");
    position = {2, 1.01f, 0};
    verticalSpeed = -1;
    Update(.05f);
    Require(grounded && Near(position.y, 1), "obstacle landing");
}
} // namespace RobotJump
