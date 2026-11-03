// [복붙 위치] base.cpp의 FEATURE_BLOCK_END 바로 앞에 namespace 전체를 붙인다.
// [연결] 이 파일만 붙이면 실행되지 않는다. 참고서의 호출 등록부를 기존 On 함수 안에 합친다.
namespace GridCollision
{
// ===== 1. 시험 조건에 맞게 바꿀 설정값 =====
constexpr float MoveInterval =
    .2f; // [수정] 자동 한 칸 이동 간격(초). 작을수록 빨라진다. 0보다 커야 한다.
constexpr float CellSize = 60.0f;    // [수정] 칸 간격. 칸·장애물·주인공 크기를 함께 변경한다.
const glm::vec2 GridOrigin(190, 90); // [수정] (0행,0열) 칸 중심. 보드 전체를 이동시키는 값.

// ===== 2. 실행 중 바뀌는 상태와 자료 =====
// row/col은 픽셀 위치가 아닌 행·열 번호다. N은 행과 열의 개수다. 화면에 넣으려면 CellSize와
// GridOrigin도 함께 확인한다.

int row = 0, col = 0;
constexpr int N = 8;
bool automatic = false;
float timer = 0;
int obstacleRow = 3, obstacleCol = 3;
bool Move(int dr, int dc)
{
    int r = row + dr, c = col + dc;
    if (r < 0 || r >= N || c < 0 || c >= N || (r == obstacleRow && c == obstacleCol))
        return false;
    row = r;
    col = c;
    return true;
}
// [초기화] 시작/F5에서 호출한다. 설정값을 현재 상태로 복사한다. 조합할 때 use2D는 한 좌표계로
// 통일한다.
void Reset()
{
    use2D = true;
    row = col = 0;
    automatic = false;
    timer = 0;
}
// [키 입력] 실행 플래그나 상태를 바꾼다. 두 기능이 같은 키를 쓰면 등록부에서 키를 분배한다.
void Key(int key, int action, int)
{
    if (action != GLFW_PRESS && action != GLFW_REPEAT)
        return;
    if (key == GLFW_KEY_LEFT)
        Move(0, -1);
    if (key == GLFW_KEY_RIGHT)
        Move(0, 1);
    if (key == GLFW_KEY_UP)
        Move(-1, 0);
    if (key == GLFW_KEY_DOWN)
        Move(1, 0);
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        automatic = !automatic;
}
// [시간 갱신] dt는 지난 프레임부터 흐른 초다. 초당 속도에는 dt를 곱한다. 그리기 전에 한 번
// 호출한다.
void Update(float dt)
{
    if (!automatic)
        return;
    timer += dt;
    while (timer >= MoveInterval)
    {
        timer -= MoveInterval;
        int dc = row % 2 == 0 ? 1 : -1;
        if ((dc == 1 && col == N - 1) || (dc == -1 && col == 0))
        {
            if (!Move(1, 0))
                automatic = false;
        }
        else if (!Move(0, dc))
            automatic = false;
    }
}
// [그리기] 상태를 화면에 표현한다. 같은 물체에 다른 기능을 합치면 이 함수에서 그 기능의 계산 결과를
// 사용한다.
void Draw()
{
    for (int r = 0; r < N; ++r)
        for (int c = 0; c < N; ++c)
            Rect({GridOrigin.x + c * CellSize, GridOrigin.y + r * CellSize},
                 glm::vec2(CellSize - 2), {.2f, .25f, .3f});
    Rect({GridOrigin.x + obstacleCol * CellSize, GridOrigin.y + obstacleRow * CellSize},
         glm::vec2(CellSize - 10), Red);
    Rect({GridOrigin.x + col * CellSize, GridOrigin.y + row * CellSize}, glm::vec2(CellSize - 15),
         Yellow);
}
// [자동 검사] 기본 설정값 기준의 검사다. 시험 조건으로 설정을 바꾸면 기대값도 바꿔야 한다. 일반
// 실행에서는 호출되지 않는다.
void Check()
{
    Reset();
    Require(!Move(-1, 0), "grid edge");
    row = 3;
    col = 2;
    Require(!Move(0, 1) && col == 2, "grid obstacle");
}
} // namespace GridCollision
