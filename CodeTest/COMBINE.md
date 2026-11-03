# 기능 설명을 읽고 같은 물체에 조합하기

## 코드의 읽는 순서

모든 기능 블록은 **설정값, 상태, 입력, 계산, 그리기, 검사**를 구분합니다. 없는 입력 함수는 만들지 않습니다. 각 함수 앞에 호출 시점과 역할이 있습니다.

1. `[수정]`을 검색해서 문제 조건을 넣습니다. 단위, 값을 키웠을 때의 변화, 함께 바뀌는 부분을 주석에 적었습니다.
2. 상태 설명에서 현재 위치·각도·선택 대상이 어느 변수에 들어 있는지 찾습니다.
3. 입력 함수는 상태를 바꾸고, `Update(dt)`는 시간에 따라 상태를 갱신합니다.
4. `Draw()`는 그 상태를 읽어서 그립니다. 움직임을 여기서 갱신하면 여러 뷰에서 속도가 배가 될 수 있습니다.
5. `Check()`는 `--check` 실행에 사용하는 기본 설정 검사입니다. 시험 조건으로 값을 바꾸면 일부 기대값도 바꿔야 합니다.

## 설정과 상태를 구분하기

```cpp
constexpr float OrbitSpeed = 30.0f; // 설정: 초당 30도. 시험 조건이 60도라면 이 값을 60으로.
float orbit = 0;                    // 상태: 지금 몇 도까지 공전했는지 저장.
// Update 안에서:
orbit += OrbitSpeed * dt;           // 0.5초가 흐르면 15도 증가.
```

속도를 바꾸려고 `orbit=60`을 쓰면 현재 각도만 갑자기 바뀝니다. 지속적인 속도는 `OrbitSpeed`를 바꿉니다. `constexpr` 설정값은 소스를 수정한 뒤 다시 빌드해야 합니다. 실행 중 키로 변경하려면 `float` 상태를 따로 두고 설정값을 `Reset()`에서 복사합니다.

| 바꾸고 싶은 것 | 예제와 변수 | 효과 |
|---|---|---|
| 드래그 사각형을 가로로 늘리기 | 03 `InitialSize(200,100)` | 실제 그림과 클릭 판정 너비가 함께 바뀜 |
| 클릭한 사각형만 빨갛게 | 03 `items[selected].color = Red` | 선택 인덱스가 0 이상일 때 사용 |
| 행성 크기를 두 배로 | 13 `PlanetSize=.65f`를 `1.3f`로 | 행성 상자 한 변이 두 배. 달 거리에는 영향 없음 |
| 행성 궤도를 크게 | 13 `OrbitRadius=3`을 `5`로 | 행성 위치와 표시 궤도가 함께 바뀜 |
| 달만 멀리 | 13 `MoonDistance=1`을 `2`로 | 행성 중심에서 달 중심까지 거리 증가 |
| 더 높은 점프 | 19 `JumpSpeed=5`를 `7`로 | 초기 상승 속도가 커짐. 무장애물 이론 최고 높이는 v²/(2g) |
| 더 큰 공 | 17 `BallRadius=.3f`를 `.6f`로 | 그리기, 벽 경계, 바닥 착지 높이가 함께 바뀜 |
| 더 긴 포신 | 14 `BarrelLength=2`를 `3`으로 | 길이와 중심 오프셋이 함께 변경되어 관절에 붙어 있음 |
| 목표에 쉽게 붙이기 | 07 `SnapDistance=45`를 `80`으로 | 놓았을 때 중심 거리가 80 이하면 목표에 붙음 |
| 느린 문 열기 | 20 `Duration=1`을 `2`로 | 개폐 시간이 두 배로 늘어남 |

## 조합의 두 종류

**독립 기능을 함께 실행**할 때는 함수 호출을 합칩니다. 예를 들어 배경색 타이머와 도형 드래그는 서로 다른 상태를 바꿉니다. 24번이 이 형태입니다.

**같은 물체에 두 기능을 적용**할 때는 객체의 위치를 한곳에 저장해야 합니다. 두 예제의 `Draw()`를 모두 호출하면 두 물체가 생길 수 있습니다. 한쪽의 위치·변환·보간 계산을 다른 쪽의 입력이나 그리기에 연결합니다. 25·26번이 이 형태입니다.

## 재사용할 계산과 상태

| 가져올 것 | 용도 | 원래 예제에서 빼도 되는 것 |
|---|---|---|
| `OrbitHierarchy::PlanetFrame()` | 공전 중인 물체의 위치·방향 행렬 | 별도 장면을 그리면 `OrbitHierarchy::Draw()` |
| `PulseColor::Blend()` | 현재 시간의 0~1 보간 비율 | `PulseColor::Draw()` |
| `SnapMatch::TrySnap(position,target,distance)` | 전달한 실제 위치를 목표에 붙임 | `SnapMatch::Mouse/Cursor/Draw()` |
| `DragSelect::items[i].center` | 선택된 실제 도형 위치 | 복사본 위치를 만들지 않음 |
| `JointModel::Turret()` | 포탑에 새 부품을 붙일 부모 행렬 | 포탑 계산은 유지하고 Draw에 새 부품 추가 |
| `Conveyor::Path(distance)` | 경로상의 위치·방향 | 상자 대신 구를 그리려면 기존 Draw 교체 |
| `ShrinkingDoor::Model()` | 현재 진행률의 문 행렬 | 다른 장면에서 문만 그리면 기존 Draw 제외 |
| `MultiView::Scene()` | 여러 카메라가 공유하는 장면 | Scene 안을 원하는 모델 그리기로 교체 |

행렬 계산이나 보간값을 가져오더라도 관련 `Update()` 호출은 유지해야 시간이 흐릅니다. 순수 위치 변경 함수인 `TrySnap()`은 마우스를 놓는 순간 호출합니다.

## 예제 25 공전하는 같은 행성의 색과 크기

`13_orbit_hierarchy.inc`와 `21_pulse_color.inc`를 넣습니다. `OnUpdate`에서 두 기능의 시간을 갱신합니다. `OnDraw`에서는 둘의 원래 Draw를 호출하는 대신 계산 결과를 상자 하나에 합칩니다.

```cpp
float u = PulseColor::Blend(); // 0~1
float size = glm::mix(PulseColor::MinimumRadius, PulseColor::MaximumRadius, u);
glm::vec3 color = glm::mix(PulseColor::StartColor, PulseColor::EndColor, u);
glm::mat4 model = OrbitHierarchy::PlanetFrame()
                * R(OrbitHierarchy::spin, {0,1,0})
                * S(glm::vec3(size));
Box(model, color);
```

`PulseColor`의 설정명은 구 예제를 기준으로 Radius이지만, 여기서는 같은 수치를 상자 한 변 길이로 사용합니다. 이처럼 재사용할 때 **반지름과 전체 길이의 의미**를 확인합니다.

Space는 공전, S는 색·크기를 정지합니다. 같은 키를 두 기능에 전달하지 않아 독립적으로 조작할 수 있습니다. 전체 코드는 [25번 예제](examples/25_shared_object.cpp)에 있습니다.

## 예제 26 드래그한 같은 사각형을 스냅

`DragSelect`의 `items` 배열을 실제 물체로 사용합니다. `SnapMatch::position`을 따로 움직이지 않습니다.

```cpp
bool wasDragging = DragSelect::dragging;
DragSelect::Mouse(button, action);
int i = DragSelect::selected;
if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE && wasDragging && i >= 0)
    SnapMatch::TrySnap(DragSelect::items[i].center,
                      SnapMatch::TargetCenter,
                      SnapMatch::SnapDistance);
```

`TrySnap`의 첫 매개변수는 참조(`&`)이므로 `items[i].center` 자체가 바뀝니다. 값 복사로 받으면 원래 도형이 움직이지 않습니다. 놓기 전 `dragging`을 저장하는 이유는 `DragSelect::Mouse`가 버튼을 놓으면 그 값을 false로 바꾸기 때문입니다.

이 조합은 붙인 뒤에도 다시 드래그할 수 있습니다. 여러 물체가 같은 목표에 붙는 것도 허용합니다. 문제에서 잠금·중복 점유 금지를 요구하면 객체별 잠금 상태와 슬롯 점유 인덱스를 추가합니다. 전체 코드는 [26번 예제](examples/26_shared_object.cpp)에 있습니다.

## 복사 범위와 등록부

- **전체 실행 코드**: 새 `.cpp` 하나를 통째로 채웁니다.
- **기능 블록**: `namespace 이름 { ... }` 전체를 기존 `FEATURE_BLOCK_END` 앞에 넣습니다.
- **호출 등록부**: 기존 `OnReset/OnKey/...` 함수 안에 호출문을 추가합니다. 같은 이름의 함수를 다시 정의하지 않습니다.
- **조합 설명**: 해당 기능의 설정값, 붙여넣기 순서, 주의점을 확인합니다. 설정 선언을 기존 선언 옆에 중복 붙이지 않습니다.

두 모듈이 같은 이름의 `position`을 갖는 것은 이름공간이 달라 괜찮습니다. 그러나 서로 다른 위치 변수라는 점은 그대로입니다. 같은 물체를 조작하려면 어느 이름공간의 위치를 기준으로 삼을지 정해야 합니다.

## 조합 후 확인할 순서

1. 같은 좌표계인지 확인합니다. 2D 논리 픽셀과 3D 월드 좌표는 그대로 섞지 않습니다.
2. 객체의 위치·크기를 한쪽 상태로 통일합니다.
3. `Reset`이 설정을 되돌리거나 카메라를 덮어쓰는 순서를 확인합니다.
4. 입력 키가 겹치면 다른 키 또는 Ctrl로 분리합니다. 23번 탱크·카메라 예제를 참고합니다.
5. 시간 갱신은 한 번씩, 최종 객체 그리기도 의도한 횟수만 수행합니다.
6. 바꾼 크기가 클릭·충돌·경첩 위치에도 반영되는지 확인합니다.
