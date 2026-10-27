# 1-12 화면의 도형 이동 맞추기 초보자를 위한 개발 순서와 코드 해설

이 문서는 [1-12.cpp](../1-4/1-4/1-12.cpp)를 처음 공부하는 사람을 위한 설명이다. [1-10 해설](1-10-shape-matching-walkthrough.md), [1-11 해설](1-11-board-movement-collision-walkthrough.md)처럼 **만드는 순서**, **실제 코드 읽기**, **한 줄씩 바꾸는 연습**으로 구성했다.

먼저 기억할 동작은 다음과 같다.

1. 왼쪽과 가운데의 긴 공간에서 사각형이 각각 위아래로 움직인다.
2. 두 사각형의 중심이 중앙 점선 영역에 들어오면 점선이 초록색이 된다.
3. 이때 Enter를 누르면 두 사각형이 같은 높이에 맞춰진다.
4. 두 사각형이 우측으로 이동하면서 아래위로 쌓인다.
5. 새 사각형 두 개가 나타나고 다시 움직인다.

**현재 Enter 판정은 두 중심이 같은 점선 영역 안에 있는지 확인한다.** 누르기 전에 두 사각형의 높이가 정확히 같거나 서로 겹칠 필요는 없다. 중심 높이가 275와 325여도 둘 다 점선 범위 안이므로 성공한다.

| 키 | 동작 |
| --- | --- |
| Enter 또는 숫자 키패드 Enter | 점선 영역에 들어온 두 도형을 맞춘다. |
| r | 쌓인 도형을 지우고 처음부터 시작한다. |
| q 또는 Esc | 프로그램을 종료한다. |

코드의 함수 배치 순서와 공부하기 좋은 순서는 다를 수 있다. 아래 코드 블록은 전체 파일의 일부이거나 특정 함수다. **각 블록을 따로 붙여 넣는 완성 프로그램으로 생각하지 말고, 링크로 원본을 함께 읽는다.**

## 1 빈 창부터 띄운다

[`main()`](../1-4/1-4/1-12.cpp#L470)은 `default-Gl`의 창 틀에서 시작한다. 도형을 만들기 전에 창이 열리는지 먼저 확인한다.

1. `glfwInit()`으로 GLFW를 준비한다.
2. OpenGL 버전을 3.3으로 설정한다.
3. `glfwCreateWindow()`로 800 × 600 창을 만든다.
4. `glfwMakeContextCurrent()`로 이 창에 그림을 그리도록 설정한다.
5. `glewInit()`으로 OpenGL 함수를 사용할 준비를 한다.
6. 반복문에서 입력을 받고, 위치를 바꾸고, 그림을 그린다.
7. 종료할 때 버퍼와 창을 정리한다.

GLFW는 창과 키보드를 담당한다. GLEW는 OpenGL 함수를 사용할 수 있도록 돕는다. `glew.h`를 `glfw3.h`보다 먼저 포함한다.

```cpp
#include <GL/glew.h>
#include <GL/glfw3.h>
```

창을 만든 뒤 컨텍스트를 설정하고, 그 뒤에 GLEW를 초기화해야 한다. 준비 중 하나라도 실패하면 오류를 출력하고 종료한다.

**확인:** 처음 공부할 때는 빈 창이 뜨는 것부터 확인한다. 아직 없는 함수를 호출하면 컴파일되지 않는다.

## 2 좌표와 크기를 먼저 정한다

이 프로그램은 **화면 왼쪽 위를 (0, 0)**으로 생각한다. X가 커지면 오른쪽, Y가 커지면 아래쪽이다.

```text
(0, 0)                       오른쪽으로 X 증가


아래로 Y 증가                         (800, 600)
```

화면 배치를 정하는 상수는 다음과 같다. `constexpr`는 실행 중 바꾸지 않을 값을 정하는 문법이다.

| 상수 | 현재 값 | 쉬운 뜻 |
| --- | --- | --- |
| `WINDOW_WIDTH`, `WINDOW_HEIGHT` | 800, 600 | 창 너비와 높이 |
| `LANE_X[0]`, `LANE_X[1]` | 160, 340 | 두 이동 공간의 중심 X |
| `LANE_TOP`, `LANE_BOTTOM` | 80, 520 | 이동 공간의 위와 아래 |
| `BLOCK_WIDTH`, `BLOCK_HEIGHT` | 50, 28 | 사각형 너비와 높이 |
| `MATCH_TOP`, `MATCH_BOTTOM` | 260, 340 | 중앙 점선 영역의 위와 아래 |
| `STACK_X` | 680 | 우측에 쌓을 위치의 중심 X |
| `SLIDE_TIME` | 0.9 | 우측 이동에 걸리는 시간, 초 단위 |

`LANE_X`는 숫자 두 개가 들어 있는 배열이다. 배열 번호는 0부터 시작하므로 `[0]`은 왼쪽, `[1]`은 가운데 공간이다.

`28.0f`에서 `f`는 `float`형 숫자라는 뜻이다. `float`는 28.5처럼 소수점이 있는 값을 저장할 수 있다.

**확인:** 점선 영역은 Y=260부터 Y=340까지다. X=160, Y=300인 중심은 왼쪽 공간의 점선 안에 있다.

## 3 사각형 하나의 정보를 묶는다

`struct`는 관련된 정보를 하나로 묶는 문법이다. 사각형에 필요한 값은 `Block`에 저장한다.

```cpp
struct Block
{
    float y;
    float speed;
    Color color;
};
```

- `y`: 사각형 중심의 높이.
- `speed`: 1초에 움직이는 거리와 방향.
- `color`: 사각형의 색.

`g_blocks[0]`은 왼쪽 사각형이고 `g_blocks[1]`은 가운데 사각형이다. X는 평소에 `LANE_X`로 정해지므로 `Block`에 따로 저장하지 않는다.

색은 `Color`에 빨강, 초록, 파랑 세 값으로 저장한다. 각 성분은 0부터 1까지 사용한다. 예를 들어 `{ 1.0f, 0.0f, 0.0f }`는 빨간색이다.

쌓인 두 도형은 `MatchedPair`로 묶는다.

```cpp
struct MatchedPair
{
    Color lowerColor;
    Color upperColor;
};
```

`lowerColor`는 아래 도형의 색, `upperColor`는 위 도형의 색이다. 쌓을 위치는 순번으로 계산하므로 쌓인 도형에 Y를 따로 저장하지 않는다.

## 4 지금 움직이는 도형과 쌓인 도형을 구분한다

게임에서 자주 쓰는 변수 네 개부터 이해한다.

| 변수 | 쉬운 뜻 |
| --- | --- |
| `g_blocks` | 지금 움직이는 사각형 두 개 |
| `g_stack` | 우측에 쌓인 사각형 쌍의 목록 |
| `g_sliding` | 우측 이동 중이면 `true`, 평소에는 `false` |
| `g_slideTimer` | 우측 이동을 시작한 뒤 지난 시간 |

`bool`은 참 또는 거짓을 저장하는 자료형이다. `true`는 참, `false`는 거짓이다. 따라서 `if (g_sliding)`은 “지금 우측으로 이동 중이라면”이라고 읽는다.

`std::vector`는 원소를 추가할 수 있는 목록이다. `g_stack`에 사각형 쌍을 하나씩 보관한다.

```cpp
std::vector<MatchedPair> g_stack;
```

| 코드 | 뜻 |
| --- | --- |
| `g_stack.size()` | 지금 쌓인 **쌍**의 개수 |
| `g_stack.push_back(...)` | 목록 마지막에 새 쌍을 추가한다. |
| `g_stack.clear()` | 쌓인 쌍을 모두 지운다. |

`g_stack.size()`가 3이면 도형은 6개다. 한 쌍에 도형이 두 개 들어 있기 때문이다.

## 5 새 사각형 두 개를 만든다

[`NewBlocks()`](../1-4/1-4/1-12.cpp#L173)에서 시작 높이, 속도, 색을 정한다. 실제 대입 부분은 다음과 같다.

```cpp
    const size_t colorIndex = (g_stack.size() % 2) * 2;
    g_blocks[0] = Block{ 140.0f, 165.0f, colors[colorIndex] };
    g_blocks[1] = Block{ 450.0f, -145.0f, colors[colorIndex + 1] };
    g_sliding = false;
    g_slideTimer = 0.0f;
```

왼쪽 사각형은 Y=140에서 시작하고 아래로 움직인다. 가운데 사각형은 Y=450에서 시작하고 위로 움직인다. 화면에서는 Y가 커지는 방향이 아래이므로 **양수 속도는 아래, 음수 속도는 위**다.

색 배열에는 파랑, 노랑, 초록, 주황이 들어 있다. `%`는 나머지를 구한다.

| 쌓인 쌍의 수 | `size() % 2` | `colorIndex` | 새 도형의 색 |
| --- | --- | --- | --- |
| 0 | 0 | 0 | 파랑과 노랑 |
| 1 | 1 | 2 | 초록과 주황 |
| 2 | 0 | 0 | 파랑과 노랑 |

색은 이 두 조합을 번갈아 사용한다. 새 도형이 나올 때 높이와 속도는 항상 위의 시작값으로 돌아간다.

**확인:** 한 쌍을 맞춘 뒤 초록과 주황 도형이 나타나는지 확인한다.

## 6 사각형을 삼각형 두 개로 그린다

OpenGL에는 이 코드가 사용하는 방식의 “사각형 그리기” 명령이 없다. [`AddQuad()`](../1-4/1-4/1-12.cpp#L307)에서 삼각형 두 개를 만든다.

```cpp
void AddQuad(float left, float top, float right, float bottom, Color color)
{
    AddVertex(left, top, color);
    AddVertex(right, top, color);
    AddVertex(right, bottom, color);
    AddVertex(left, top, color);
    AddVertex(right, bottom, color);
    AddVertex(left, bottom, color);
}
```

앞의 정점 세 개가 첫 삼각형, 뒤의 정점 세 개가 둘째 삼각형이다. 두 삼각형이 붙으면 사각형이 된다.

`left`, `top`, `right`, `bottom`은 각각 왼쪽, 위쪽, 오른쪽, 아래쪽 경계다. 예를 들어 중심 (160, 140), 너비 50, 높이 28인 사각형은 다음 범위를 차지한다.

```text
왼쪽 = 160 - 25 = 135
오른쪽 = 160 + 25 = 185
위쪽 = 140 - 14 = 126
아래쪽 = 140 + 14 = 154
```

[`AddBlock()`](../1-4/1-4/1-12.cpp#L326)은 이런 계산을 하고 색과 테두리를 넣는다. [`AddOutline()`](../1-4/1-4/1-12.cpp#L317)은 얇은 사각형 네 개로 긴 공간의 테두리를 만든다.

이 함수들은 즉시 화면에 그리지 않는다. `g_vertices`에 정점을 모으고, `DrawScene()`이 나중에 한꺼번에 OpenGL에 보낸다.

**확인:** `g_vertices`는 이번 화면의 정점 목록이고, `g_stack`은 게임에 쌓인 도형 목록이다. 둘을 혼동하지 않는다.

## 7 시간이 흐른 만큼 위아래로 움직인다

[`MoveBlock()`](../1-4/1-4/1-12.cpp#L221)의 핵심은 한 줄이다.

```cpp
    block.y += block.speed * deltaTime;
```

`+=`는 오른쪽 값을 기존 값에 더한다. `deltaTime`은 이전 프레임 이후 지난 시간이며 단위는 초다.

```text
현재 Y = 140
속도 = 165픽셀/초
지난 시간 = 0.1초

이동 거리 = 165 × 0.1 = 16.5픽셀
새 Y = 140 + 16.5 = 156.5
```

속도가 -145이면 같은 0.1초 동안 Y가 14.5 줄어들어 위로 움직인다. 시간 값을 곱하면 화면을 자주 그리더라도 한 초 동안 움직이는 거리를 맞출 수 있다.

`main()`에서는 실제 시간 차를 구한다.

```cpp
        const double currentTime = glfwGetTime();
        const float deltaTime = std::min(static_cast<float>(currentTime - previousTime), 0.05f);
        previousTime = currentTime;
```

`glfwGetTime()`은 GLFW가 시작된 뒤 지난 시간을 구한다. `currentTime - previousTime`이 이번에 지난 시간이다. `static_cast<float>`는 그 숫자를 `float`형으로 바꾼다.

`std::min(a, b)`는 두 값 중 작은 값을 선택한다. 현재 코드는 한 프레임에서 최대 0.05초까지만 이동에 반영한다. 실행이 크게 멈췄다가 돌아오면 긴 정지 시간을 모두 따라잡지 않으므로, 그 순간에는 실제 시간보다 적게 움직일 수 있다.

## 8 위나 아래 끝에 닿으면 방향을 바꾼다

중심이 공간 끝까지 가면 도형 절반이 밖으로 나간다. 그래서 [`MoveBlock()`](../1-4/1-4/1-12.cpp#L221)은 도형 높이의 절반을 계산에 넣는다.

```cpp
    const float minimumY = LANE_TOP + BLOCK_HEIGHT * 0.5f;
    const float maximumY = LANE_BOTTOM - BLOCK_HEIGHT * 0.5f;
```

현재 값으로 계산하면 중심의 범위는 94부터 506까지다.

```text
위쪽 중심 한계 = 80 + 14 = 94
아래쪽 중심 한계 = 520 - 14 = 506
```

아래쪽 경계를 넘으면 이렇게 처리한다.

```cpp
        if (block.y > maximumY)
        {
            block.y = maximumY * 2.0f - block.y;
            block.speed = -std::abs(block.speed);
        }
```

`std::abs()`는 숫자의 절댓값을 구한다. `-std::abs(speed)`는 속도를 음수로 만들어 위를 향하게 한다.

예를 들어 계산된 Y가 516이면 아래쪽 한계 506을 10만큼 넘었다. `506 × 2 - 516 = 496`으로 바꾸어 안쪽으로 10만큼 되돌린다. 경계 밖으로 나간 거리를 안쪽으로 반사하는 것이다.

위쪽 경계를 넘으면 같은 방식으로 위치를 되돌리고 속도를 양수로 만든다. `while`은 위치가 아직 밖에 있다면 다시 검사한다. 한 번에 여러 경계를 넘을 만큼 큰 시간 값에도 대응한다.

**확인:** 도형 전체가 긴 공간 안에서 왕복하는지 확인한다.

## 9 두 도형이 중앙 점선 영역에 있는지 확인한다

[`CanMatch()`](../1-4/1-4/1-12.cpp#L193)는 Enter를 받아들일 수 있는지 알려 준다. `Can`은 “할 수 있는가”, `Match`는 “맞추기”로 생각하면 된다.

```cpp
bool CanMatch()
{
    if (g_sliding)
        return false;

    // 두 도형의 중심이 같은 점선 영역에 들어오면 Enter로 높이를 맞춘다.
    for (const Block& block : g_blocks)
    {
        if (block.y < MATCH_TOP || block.y > MATCH_BOTTOM)
            return false;
    }

    return true;
}
```

위에서부터 다음처럼 읽는다.

1. 이미 우측으로 이동 중이면 `false`를 반환한다.
2. 사각형 두 개를 하나씩 살핀다.
3. 하나라도 중심 Y가 260보다 작거나 340보다 크면 `false`다.
4. 두 사각형이 모두 범위 안이면 `true`다.

`for (const Block& block : g_blocks)`는 “`g_blocks`의 사각형을 하나씩 `block`이라는 이름으로 살핀다”는 뜻이다. `const`는 검사 중 값을 바꾸지 않겠다는 뜻이고, `&`는 복사본 대신 원래 도형을 참조한다는 뜻이다.

`||`는 “또는”이다. 따라서 위쪽 밖 **또는** 아래쪽 밖이면 실패한다. `return`은 함수를 끝내고 결과를 돌려준다.

| 왼쪽 중심 Y | 가운데 중심 Y | Enter 가능 여부 |
| --- | --- | --- |
| 140 | 450 | 불가능. 둘 다 밖이다. |
| 300 | 450 | 불가능. 하나가 밖이다. |
| 275 | 325 | 가능. 둘 다 안이다. |
| 260 | 340 | 가능. 경계도 포함한다. |

[`DrawScene()`](../1-4/1-4/1-12.cpp#L342)도 이 함수를 사용해서 점선을 초록색으로 바꾼다. **화면의 초록색 표시와 Enter 판정이 같은 함수를 사용한다.**

**확인:** 점선이 초록색일 때만 Enter가 성공하는지 확인한다. 도형의 중심으로 판정하므로 테두리 일부가 점선 밖에 있어도 성공할 수 있다.

## 10 Enter가 눌리면 두 도형을 같은 높이에 맞춘다

[`TryMatch()`](../1-4/1-4/1-12.cpp#L208)는 실제 맞추기를 시작한다.

```cpp
void TryMatch()
{
    if (!CanMatch())
        return;

    // Enter를 누르는 순간 같은 높이에 맞추고 상하 왕복 이동을 멈춘다.
    g_alignedY = (g_blocks[0].y + g_blocks[1].y) * 0.5f;
    g_blocks[0].y = g_alignedY;
    g_blocks[1].y = g_alignedY;
    g_sliding = true;
    g_slideTimer = 0.0f;
}
```

`!CanMatch()`는 “맞출 수 없다”는 뜻이다. 조건이 안 맞으면 바로 끝낸다.

두 높이의 평균을 구해서 `g_alignedY`에 저장한다.

```text
왼쪽 중심 Y = 275
가운데 중심 Y = 325
정렬할 높이 = (275 + 325) × 0.5 = 300
```

두 `y`에 300을 넣으면 같은 높이로 맞춰진다. 그다음 `g_sliding = true`로 우측 이동 상태를 켜고, 시간을 0부터 센다.

현재 코드에서는 같은 높이로 맞추는 동작은 즉시 일어난다. 그 후 우측의 쌓을 위치까지 이동하는 부분을 애니메이션으로 보여 준다.

## 11 키보드와 함수 호출을 연결한다

[`KeyCallback()`](../1-4/1-4/1-12.cpp#L271)은 키보드 이벤트가 발생했을 때 GLFW가 호출하는 함수다. 직접 매 프레임 호출하는 함수가 아니다.

```cpp
    glfwSetKeyCallback(window, KeyCallback);
```

이 줄은 “이 창에서 키보드 이벤트가 발생하면 `KeyCallback`을 실행해 달라”는 등록이다.

콜백의 처음에서는 누른 순간만 받는다.

```cpp
    if (action != GLFW_PRESS)
        return;
```

`GLFW_PRESS`는 누른 순간, `GLFW_REPEAT`는 계속 누르고 있어서 반복된 입력, `GLFW_RELEASE`는 뗀 순간이다. 현재 코드는 PRESS만 받는다. **Enter를 누르고 기다리기보다, 점선이 초록색이 됐을 때 새로 누른다.**

`switch (key)`는 어떤 키인지 나누어 처리한다.

```cpp
    case GLFW_KEY_ENTER:
    case GLFW_KEY_KP_ENTER:
        TryMatch();
        break;
    case GLFW_KEY_R:
        ResetGame();
        break;
```

두 Enter 키는 같은 함수를 호출한다. `break`는 현재 `switch` 처리를 끝낸다. Q와 Esc는 `glfwSetWindowShouldClose()`로 종료할 상태를 설정한다.

**확인:** 키 입력은 도형이 보이는 OpenGL 창에 전달되어야 한다. 콘솔이나 코드 편집기가 선택되어 있으면 도형 창을 클릭한다.

## 12 우측 이동 시간을 센다

[`UpdateGame()`](../1-4/1-4/1-12.cpp#L243)은 매 프레임 게임 상태를 바꾼다.

```cpp
void UpdateGame(float deltaTime)
{
    if (g_sliding)
    {
        g_slideTimer += deltaTime;
        if (g_slideTimer >= SLIDE_TIME)
        {
            g_stack.push_back(MatchedPair{ g_blocks[0].color, g_blocks[1].color });
            NewBlocks();
        }
        return;
    }

    for (Block& block : g_blocks)
        MoveBlock(block, deltaTime);
}
```

여기에는 두 가지 상태만 있다.

| 상태 | 이번 프레임에 하는 일 |
| --- | --- |
| `g_sliding == false` | 두 도형에 `MoveBlock()`을 호출하여 위아래로 움직인다. |
| `g_sliding == true` | 이동 시간을 더하고, 0.9초가 지났는지 확인한다. |

우측 이동 중에는 위아래 왕복 이동을 호출하지 않는다. 그래서 맞춘 도형이 다시 튕겨 움직이지 않는다. 애니메이션을 표시할 X와 Y는 다음 단계의 `DrawScene()`에서 계산한다.

0.9초가 지나면 색 두 개를 `g_stack`에 추가하고 `NewBlocks()`를 호출한다. `NewBlocks()`가 우측 이동 상태와 시간을 초기화하기 때문에, 새 도형은 다시 위아래로 움직일 수 있다.

**확인:** 우측 이동 중 Enter를 다시 눌러도 쌓기 작업이 중복으로 시작되지 않아야 한다.

## 13 시작 위치와 쌓을 위치 사이를 조금씩 이동한다

[`DrawScene()`](../1-4/1-4/1-12.cpp#L342)에서 우측 이동의 진행률을 구한다.

```cpp
        const float t = std::min(g_slideTimer / SLIDE_TIME, 1.0f);
        const float progress = t * t * (3.0f - 2.0f * t);
```

`t`는 이동이 얼마나 진행됐는지 나타내는 비율이다.

| 이동 시간 | `t` | 뜻 |
| --- | --- | --- |
| 0초 | 0 | 출발 |
| 0.45초 | 0.5 | 중간 |
| 0.9초 | 1 | 도착 |

`progress`의 식은 처음과 끝을 부드럽게 움직이게 한다. 처음 공부할 때는 “0에서 1까지 변하는 이동 비율”로 이해하면 된다. 중간인 `t=0.5`에서는 `progress`도 0.5다.

실제 화면에 그릴 좌표는 다음 식이다.

```cpp
            const float x = LANE_X[i] + (STACK_X - LANE_X[i]) * progress;
            const float y = g_alignedY + (targetY - g_alignedY) * progress;
```

공통 모양은 **시작값 + (목표값 - 시작값) × 진행률**이다.

```text
왼쪽 도형의 시작 X = 160
목표 X = 680
진행률 = 0.5
현재 X = 160 + (680 - 160) × 0.5 = 420
```

진행률이 0이면 시작 위치, 1이면 목표 위치가 된다. Y도 같은 방식으로 계산한다. 따라서 현재 애니메이션은 오른쪽으로 가면서 우측의 쌓을 높이에 접근한다. Y를 그대로 고정한 수평 이동만 하는 것은 아니다.

## 14 한 쌍씩 아래에서 위로 쌓는다

우측에서는 한 쌍의 왼쪽 도형을 아래에, 가운데 도형을 위에 놓는다. `i`는 쌓인 쌍의 순번이고 0부터 시작한다.

```cpp
        const float bottom = LANE_BOTTOM - static_cast<float>(i) * stackHeight * 2.0f;
        AddBlock(STACK_X, bottom - stackHeight * 0.5f, stackHeight, g_stack[i].lowerColor);
        AddBlock(STACK_X, bottom - stackHeight * 1.5f, stackHeight, g_stack[i].upperColor);
```

도형 높이가 아직 28이면 한 쌍이 차지하는 높이는 `28 × 2 = 56`이다.

| 쌍 순번 | 아래 경계 `bottom` | 아래 도형 중심 | 위 도형 중심 |
| --- | --- | --- | --- |
| 0 | 520 | 506 | 478 |
| 1 | 464 | 450 | 422 |
| 2 | 408 | 394 | 366 |

첫 번째 쌍의 위에 두 번째 쌍이 놓인다. 먼저 맞춘 쌍이 아래에 남는다.

많이 쌓으면 화면 위로 나갈 수 있다. [`StackBlockHeight()`](../1-4/1-4/1-12.cpp#L261)은 높이를 줄여 모두 같은 공간에 넣는다.

```text
쌓을 수 있는 세로 길이 = 520 - 80 = 440
7쌍의 기본 높이 = 7 × 2 × 28 = 392  → 28 유지
8쌍의 기본 높이 = 8 × 2 × 28 = 448  → 높이를 440 ÷ 16 = 27.5로 줄임
```

우측 이동 중인 새 쌍도 개수에 포함한다. 그래서 새 쌍이 출발할 때 기존 도형들의 높이도 줄어들 수 있다. 개수가 늘어도 너비 50은 유지하고 높이만 줄인다.

## 15 리셋과 전체 반복 순서를 연결한다

[`ResetGame()`](../1-4/1-4/1-12.cpp#L186)은 세 줄로 처음 상태를 만든다.

```cpp
void ResetGame()
{
    g_stack.clear();
    g_alignedY = 0.0f;
    NewBlocks();
}
```

쌓인 도형을 모두 지우고 정렬 높이를 초기화한 다음, 새 도형을 만든다. `NewBlocks()`가 `g_sliding`과 `g_slideTimer`도 초기화하므로 우측 이동 도중 R을 눌러도 처음으로 돌아간다.

`main()`의 반복에서는 다음 순서가 중요하다.

```cpp
        glfwPollEvents();
        UpdateGame(deltaTime);
        DrawScene();
        glfwSwapBuffers(window);
```

1. `glfwPollEvents()`: 현재 화면을 보고 누른 키를 받아 콜백을 실행한다.
2. `UpdateGame(deltaTime)`: 이번 프레임의 이동 상태를 바꾼다.
3. `DrawScene()`: 이번 상태의 정점을 만들고 그린다.
4. `glfwSwapBuffers(window)`: 완성된 화면을 보여 준다.

창 크기가 바뀌면 [`FramebufferSizeCallback()`](../1-4/1-4/1-12.cpp#L293)이 그릴 영역을 조절한다. 게임 계산은 계속 800 × 600 기준으로 하고, 창 안에서 그 비율을 유지한다. 창 비율이 다르면 남는 부분이 생길 수 있다.

## 16 GLSL과 VAO VBO를 연결한다

여기부터는 그림을 실제 화면에 보내는 부분이다. 게임 규칙을 먼저 이해한 뒤 읽어도 된다.

| 이름 | 먼저 기억할 역할 |
| --- | --- |
| `Vertex` | 정점 한 개의 위치와 색을 묶는다. |
| VBO | GPU에 정점 데이터를 저장한다. |
| VAO | 그 데이터를 위치와 색으로 나누어 읽는 방법을 기록한다. |
| Vertex Shader | 정점의 위치를 화면 표시용 좌표로 바꾼다. |
| Fragment Shader | 화면에 칠할 색을 정한다. |

정점 하나에는 실수 다섯 개가 들어 있다.

```text
x, y, red, green, blue
위치 2개, 색 3개
```

`main()`은 다음처럼 연결한다.

```cpp
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        reinterpret_cast<void*>(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
```

0번 속성에는 위치 두 개를 넣고, 1번 속성에는 색 세 개를 넣는다. `sizeof(Vertex)`는 다음 정점까지 건너갈 크기다. 색 앞에는 위치 실수 두 개가 있으므로 `2 * sizeof(float)`만큼 건너뛰어 색을 읽는다. `reinterpret_cast<void*>`는 이 시작 위치를 OpenGL이 요구하는 형태로 전달하는 부분이다.

[vertex.glsl](../1-4/1-4/vertex.glsl)도 같은 번호와 개수를 사용한다.

```glsl
layout (location = 0) in vec2 position;
layout (location = 1) in vec3 color;
```

`vec2`는 값 두 개, `vec3`는 값 세 개를 묶는다. C++의 위치 2개와 색 3개가 셰이더의 선언과 맞아야 한다.

픽셀 좌표를 OpenGL 표시용 좌표로 바꾸는 부분은 다음과 같다.

```glsl
    vec2 normalizedPosition = position / windowSize * 2.0 - 1.0;
    normalizedPosition.y = -normalizedPosition.y;
    gl_Position = vec4(normalizedPosition, 0.0, 1.0);
```

OpenGL은 화면 가운데를 (0, 0)으로 생각한다. X=400, Y=300인 점은 이 계산을 거치면 화면 가운데가 된다. Y에 마이너스를 붙이는 이유는 C++에서 정한 “위가 0이고 아래로 커지는 좌표”를 OpenGL 화면 방향에 맞추기 위해서다.

`windowSize`는 C++에서 `glUniform2f()`로 전달한다. `uniform`은 C++ 프로그램이 셰이더에 보내는 값이다. 현재 코드는 항상 기준 크기 800, 600을 전달한다.

[fragment.glsl](../1-4/1-4/fragment.glsl)은 전달받은 색을 그대로 사용한다.

```glsl
    finalColor = vec4(vertexColor, 1.0);
```

색 세 값에 1.0을 붙여 RGBA 네 값으로 만든다. 마지막 값은 알파 값이다. 이 셰이더는 알파 1을 출력한다.

[`CreateShaderProgram()`](../1-4/1-4/1-12.cpp#L135)은 GLSL 파일을 읽고, 컴파일하고, 두 셰이더를 하나의 프로그램으로 연결한다. 파일 읽기는 `ReadTextFile()`, 컴파일은 `CompileShader()`가 맡는다. 실패하면 오류를 출력하므로, 도형이 안 보일 때 콘솔 메시지를 확인한다.

## 17 이번 화면의 정점을 보내고 그린다

[`DrawScene()`](../1-4/1-4/1-12.cpp#L342)은 이전 프레임의 정점 목록을 비우고 이번 화면의 정점을 다시 만든다.

```cpp
    g_vertices.clear();
```

화면 테두리, 두 긴 공간, 중앙 점선, 쌓인 도형, 현재 도형 순서로 정점을 추가한다. 마지막에는 다음처럼 그린다.

```cpp
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(g_shaderProgram);
    glUniform2f(g_windowSizeLocation, static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT));
    glBindVertexArray(g_vao);
    glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
    glBufferData(GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(g_vertices.size() * sizeof(Vertex)),
        g_vertices.data(), GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(g_vertices.size()));
```

| 줄 | 쉬운 뜻 |
| --- | --- |
| `glClearColor(1, 1, 1, 1)`에 해당하는 첫 줄 | 흰 배경을 정한다. |
| `glClear(...)` | 이전 화면 내용을 지운다. |
| `glUseProgram(...)` | 사용할 셰이더 프로그램을 선택한다. |
| `glUniform2f(...)` | 기준 화면 크기를 셰이더에 보낸다. |
| `glBindVertexArray(...)` | 정점을 해석할 VAO를 선택한다. |
| `glBindBuffer(...)` | 정점을 담을 VBO를 선택한다. |
| `glBufferData(...)` | 이번 프레임의 정점을 GPU에 복사한다. |
| `glDrawArrays(...)` | 정점들을 삼각형으로 그린다. |

`GL_DYNAMIC_DRAW`는 데이터가 자주 바뀔 것이라는 사용 방식의 힌트다. 이 값이 도형을 자동으로 움직이는 것은 아니다. 이동은 `UpdateGame()`이 하고, 바뀐 그림은 `DrawScene()`이 보낸다.

## 전체 흐름 첫 번째 쌍을 맞추는 예

처음 두 사각형의 Y는 140과 450이다. 아직 점선 영역 밖이므로 Enter를 눌러도 변화가 없다.

1초 동안 경계에 닿지 않고 움직였다고 생각하면 다음과 같다.

```text
왼쪽 Y = 140 + 165 × 1 = 305
가운데 Y = 450 - 145 × 1 = 305
```

둘 다 점선 범위 260~340 안에 있으므로 초록색 표시가 나온다. 이때 Enter를 누르면 정렬 높이 305를 저장하고 우측 이동을 시작한다.

첫 번째 쌍의 도착 중심은 아래 도형 (680, 506), 위 도형 (680, 478)이다. 0.9초의 이동이 끝나면 색을 목록에 저장하고, Y=140과 Y=450에서 새 도형이 움직이기 시작한다.

이 예는 위치 계산을 이해하기 위한 것이다. 실제 키를 누르는 순간의 시간과 프레임에 따라 Y는 조금 달라질 수 있다.

# 2부 핵심 코드를 읽고 한 줄씩 바꿔 보기

연습할 때는 **한 번에 하나만 바꾸고, 빌드하고, 실행 결과를 확인한다.** 아래는 연습 제안이며 현재 원본에 적용한 변경이 아니다.

## 18 속도의 숫자와 부호를 바꾼다

```cpp
g_blocks[0] = Block{ 140.0f, 165.0f, colors[colorIndex] };
```

| 바꿀 값 | 예상 결과 |
| --- | --- |
| 165를 80으로 | 왼쪽 도형이 아래로 더 천천히 움직인다. |
| 165를 -165로 | 왼쪽 도형이 처음에 위로 움직인다. |
| 140을 300으로 | 왼쪽 도형이 점선 안에서 시작한다. |

두 도형의 속도를 바꾸면 점선에 동시에 들어오는 시간도 달라진다. 속도가 바뀌면 원래 수치를 전제로 한 자체검사도 확인해야 한다.

**연습:** 왼쪽과 가운데 속도를 모두 절반으로 바꿔 본다. 맞출 기회가 유지되면서 왕복 이동이 느려지는지 살핀다.

## 19 점선 영역을 넓히고 좁힌다

```cpp
constexpr float MATCH_TOP = 260.0f;
constexpr float MATCH_BOTTOM = 340.0f;
```

위 값을 240, 아래 값을 360으로 바꾸면 영역이 넓어진다. 두 중심이 함께 들어와 있는 시간이 늘어 Enter를 누르기 쉬워진다.

280, 320으로 바꾸면 영역이 좁아져 더 정확한 타이밍이 필요하다. 위쪽 값은 아래쪽 값보다 작게 둔다.

`CanMatch()`와 점선 그리기가 같은 상수를 쓰므로, 이 두 상수를 바꾸면 화면 표시와 판정이 함께 바뀐다.

**연습:** 두 중심이 정확히 같아야 한다는 조건을 추가하면 왜 Enter를 누르기 어려워지는지 생각해 본다. 이 프로그램의 기본 규칙은 같은 점선 영역에 들어오는 것이다.

## 20 우측 이동 시간을 바꾼다

```cpp
constexpr float SLIDE_TIME = 0.9f;
```

1.8로 바꾸면 우측 이동에 두 배의 시간이 걸린다. 0.45로 바꾸면 절반의 시간에 도착한다. 위아래 왕복 속도는 바뀌지 않는다.

`SLIDE_TIME`은 0보다 큰 값으로 둔다. 진행률을 계산할 때 이 값으로 나누기 때문이다.

**연습:** `progress = t * t * (3.0f - 2.0f * t)` 대신 `progress = t`를 사용해 본다. 우측 이동이 일정한 비율로 진행되어 출발과 도착이 더 급하게 느껴지는지 비교한다.

## 21 쌓이는 순서와 색을 바꾼다

```cpp
g_stack.push_back(MatchedPair{ g_blocks[0].color, g_blocks[1].color });
```

첫 값은 아래 도형의 색, 둘째 값은 위 도형의 색이다. 둘의 순서를 바꾸면 도착한 쌍의 아래위 색이 뒤집힌다.

이 한 줄만 바꾸면 우측 이동 중의 도형 순서는 그대로이고, 도착한 뒤 색 순서가 바뀐다. 애니메이션에서도 처음부터 순서를 바꾸려면 `DrawScene()`의 `targetY` 계산까지 함께 살펴야 한다.

**연습:** `NewBlocks()`의 색 배열에서 파랑을 빨강 `{ 1.0f, 0.0f, 0.0f }`으로 바꾸어 본다. 이후 생성되는 해당 도형과 쌓인 해당 도형의 색이 함께 바뀌는지 확인한다.

## 22 반복문의 순서와 return을 읽는다

`UpdateGame()`의 우측 이동 분기 마지막에는 `return`이 있다. 이 줄 덕분에 같은 프레임에서 아래쪽 `MoveBlock()`까지 실행되지 않는다.

또한 우측 이동이 끝나 `NewBlocks()`를 호출한 프레임에서는 새 도형을 시작 위치에 그린다. 그다음 반복에서 새 도형의 왕복 이동을 시작한다.

**생각하기:** `return`을 빼면 우측 이동 시간도 증가하면서 도형의 왕복 위치도 바뀐다. 현재 애니메이션은 `g_alignedY`를 사용해 그리므로 이동 중 화면이 곧바로 왕복하는 것은 아니지만, 불필요한 상태 변경이 생긴다. 새 도형도 생성 직후 같은 프레임에서 움직인다.

## 23 직접 점검하고 자체검사를 실행한다

먼저 프로그램을 실행해서 다음 순서로 확인한다.

- [ ] 두 사각형이 각 공간 안에서 위아래로 움직인다.
- [ ] 점선 밖에서 Enter를 눌러도 쌓이지 않는다.
- [ ] 두 중심이 점선 안에 오면 점선이 초록색이다.
- [ ] 초록색일 때 Enter를 새로 누르면 두 도형이 맞춰진다.
- [ ] 우측 이동이 끝나면 아래위로 쌓이고 새 도형이 나온다.
- [ ] 여러 번 맞추면 쌍의 순서가 유지되고, 많이 쌓이면 높이가 줄어든다.
- [ ] 우측 이동 중에도 R로 처음 상태로 돌아간다.
- [ ] Q로 종료한다.

[`RunSelfTest()`](../1-4/1-4/1-12.cpp#L404)는 창 없이 게임 함수와 입력 처리 함수를 검사한다. Visual Studio에서 `1-4.slnx`를 Debug/x64로 빌드한 뒤, **Computer-Graphics 폴더**를 현재 위치로 둔 PowerShell에서 실행한다.

```powershell
& '.\1-4\x64\Debug\1-4.exe' --self-test
```

성공하면 `1-12 self-test passed`가 나온다. 이 모드는 실제 키보드를 누르거나 화면을 눈으로 보는 검사를 대신하지 않는다.

이 프로젝트의 실행 파일 이름은 `1-12.exe`가 아니라 `1-4.exe`다. 다른 과제도 `main()`을 갖고 있으므로 현재 빌드에는 `1-12.cpp` 하나만 포함한다.

## 함수가 헷갈릴 때 찾아볼 표

| 하고 싶은 일 | 읽을 함수 |
| --- | --- |
| 창을 만들고 실행 순서 확인하기 | [`main()`](../1-4/1-4/1-12.cpp#L470) |
| 새 도형의 시작 위치와 색 바꾸기 | [`NewBlocks()`](../1-4/1-4/1-12.cpp#L173) |
| 처음 상태로 되돌리기 | [`ResetGame()`](../1-4/1-4/1-12.cpp#L186) |
| 위아래 이동과 경계 반사 이해하기 | [`MoveBlock()`](../1-4/1-4/1-12.cpp#L221) |
| Enter가 가능한 조건 확인하기 | [`CanMatch()`](../1-4/1-4/1-12.cpp#L193) |
| 같은 높이에 맞추고 이동 시작하기 | [`TryMatch()`](../1-4/1-4/1-12.cpp#L208) |
| 키와 기능 연결하기 | [`KeyCallback()`](../1-4/1-4/1-12.cpp#L271) |
| 이동 시간을 세고 새 도형 만들기 | [`UpdateGame()`](../1-4/1-4/1-12.cpp#L243) |
| 쌓인 도형 높이 계산하기 | [`StackBlockHeight()`](../1-4/1-4/1-12.cpp#L261) |
| 정점 만들기와 우측 이동 좌표 확인하기 | [`DrawScene()`](../1-4/1-4/1-12.cpp#L342) |
| GLSL을 준비하기 | [`CreateShaderProgram()`](../1-4/1-4/1-12.cpp#L135) |
| 동작 검사를 읽기 | [`RunSelfTest()`](../1-4/1-4/1-12.cpp#L404) |
