// [복붙 위치] base.cpp의 FEATURE_BLOCK_END 바로 앞에 namespace 전체를 붙인다.
// [연결] 이 파일만 붙이면 실행되지 않는다. 참고서의 호출 등록부를 기존 On 함수 안에 합친다.
namespace OrbitHierarchy
{
// ===== 1. 시험 조건에 맞게 바꿀 설정값 =====
constexpr float OrbitRadius =
    3.0f; // [수정] 중심에서 행성까지 거리. 3→5면 궤도와 행성이 함께 멀어진다.
constexpr float OrbitSpeed = 30.0f; // [수정] 행성 공전 속도(도/초). 음수는 반대 방향.
constexpr float SpinSpeed =
    80.0f; // [수정] 행성 자전 속도(도/초). 공전 반경에는 영향을 주지 않는다.
constexpr float MoonSpeed = 100.0f;  // [수정] 달 공전 속도(도/초).
constexpr float PlanetSize = .65f;   // [수정] 행성 상자의 한 변 길이. .65→1.3이면 두 배 커진다.
constexpr float MoonDistance = 1.0f; // [수정] 행성 중심에서 달 중심까지 거리.
constexpr float MoonRadius = .2f;    // [수정] 달 구의 반지름. 구는 기본 반지름이 1이다.

// ===== 2. 실행 중 바뀌는 상태와 자료 =====
// orbit=행성 공전각, spin=행성 자전각, moonOrbit=행성 프레임 기준 달 공전각이다. 모두 도 단위다.
// PlanetFrame()은 크기와 자전을 제외한 행성 위치 프레임이므로 조합에 재사용한다.

float orbit = 0, spin = 0, moonOrbit = 0;
bool running = true;
float radius = OrbitRadius;
// [초기화] 시작/F5에서 호출한다. 설정값을 현재 상태로 복사한다. 조합할 때 use2D는 한 좌표계로
// 통일한다.
void Reset()
{
    use2D = false;
    orbit = spin = moonOrbit = 0;
    running = true;
    radius = OrbitRadius;
}
// [키 입력] 실행 플래그나 상태를 바꾼다. 두 기능이 같은 키를 쓰면 등록부에서 키를 분배한다.
void Key(int key, int action, int)
{
    if (action == GLFW_PRESS && key == GLFW_KEY_SPACE)
        running = !running;
}
// [시간 갱신] dt는 지난 프레임부터 흐른 초다. 초당 속도에는 dt를 곱한다. 그리기 전에 한 번
// 호출한다.
void Update(float dt)
{
    if (running)
    {
        orbit += OrbitSpeed * dt;
        spin += SpinSpeed * dt;
        moonOrbit += MoonSpeed * dt;
    }
}
glm::mat4 PlanetFrame()
{
    // 먼저 중심에서 radius만큼 떨어뜨린 뒤 Y축으로 회전시킨다.
    // R*T이므로 위치 자체가 회전한다. T*R로 바꾸면 그 자리에서 자전한다.
    // 이 함수에는 행성 크기를 넣지 않아야 달의 거리까지 커지는 것을 막을 수 있다.
    return R(orbit, {0, 1, 0}) * T({radius, 0, 0});
}
// [그리기] 상태를 화면에 표현한다. 같은 물체에 다른 기능을 합치면 이 함수에서 그 기능의 계산 결과를
// 사용한다.
void Draw()
{
    Sphere(S({.8f, .8f, .8f}), Yellow);
    glm::mat4 planet = PlanetFrame();
    // 오른쪽부터 크기 설정 → 자전 → 공전 위치 반영 순서다.
    // 색을 바꾸려면 마지막 Blue를 {1,0,0} 같은 RGB 벡터로 바꾼다.
    Box(planet * R(spin, {0, 1, 0}) * S(glm::vec3(PlanetSize)), Blue);
    // 달에는 행성의 크기와 자전 행렬을 물려주지 않는다.
    Sphere(planet * R(moonOrbit, {0, 1, 0}) * T({MoonDistance, 0, 0}) * S(glm::vec3(MoonRadius)),
           Green);
    std::vector<glm::vec3> ring;
    for (int i = 0; i < 72; ++i)
    {
        float a = glm::radians(i * 5.0f);
        ring.push_back({radius * std::cos(a), 0, radius * std::sin(a)});
    }
    Primitive(GL_LINE_LOOP, ring, glm::mat4(1), {.5f, .5f, .5f});
}
// [자동 검사] 기본 설정값 기준의 검사다. 시험 조건으로 설정을 바꾸면 기대값도 바꿔야 한다. 일반
// 실행에서는 호출되지 않는다.
void Check()
{
    Reset();
    orbit = 90;
    auto p = glm::vec3(PlanetFrame() * glm::vec4(0, 0, 0, 1));
    Require(Near(glm::length(p), radius) && Near(p.x, 0), "orbit radius");
}
} // namespace OrbitHierarchy
