# 시험 중 빠른 기능 선택

## 문제 문장에서 고르는 코드

| 문제 표현 | 먼저 복사할 기능 | 직접 바꿀 곳 |
|---|---|---|
| 키를 누르면 색이 바뀐다 | 01 ColorInput | `Key`의 키와 `background` |
| 일정 시간마다 바뀐다 | 01 ColorInput | `Update`의 누적 시간과 주기 |
| 사분면마다 몇 개까지 만든다 | 02 Quadrants | `counts`, `Quadrant`, 생성 상한 |
| 클릭한 것을 선택해서 움직인다 | 03 DragSelect | `items`, `Mouse`, `Cursor` |
| 닿은 도형을 없앤다 | 04 EraseSplit | `OverlapRect` 후 `visible=false` |
| 도형이 여러 개로 쪼개져 이동한다 | 04 EraseSplit | 생성 반복문의 위치 오프셋과 속도 |
| 점 선 삼각형 사각형 원 | 05 Primitives | `Draw`의 정점과 그리기 모드 |
| 크기가 커졌다 작아진다 | 05 또는 21 | 클릭 단위면 05, 시간에 따른 변화면 21 |
| 벽에서 반사한다 | 06 또는 17 | 2D면 06, 3D 중력이면 17 |
| 지그재그 나선으로 움직인다 | 06 MotionPath | `Update`의 mode 분기 |
| 이동하는 방향을 바라본다 | 06 또는 19 | 2D `atan2(dy,dx)`, 3D `atan2(dx,dz)` |
| 근처에 놓으면 붙는다 | 07 SnapMatch | `slot`, `threshold`, 놓을 때 검사 |
| 칸 단위 이동과 장애물 | 08 GridCollision | `Move`, 행·열, 충돌 후 동작 |
| 움직이는 두 도형의 타이밍을 맞춘다 | 09 TimedMatch | `CanMatch`, `Key`, 보간 시간 |
| 객체별 이동 회전 확대 축소 | 10 Transform | `selected`, `position`, `angle`, `size` |
| 두 물체가 자리를 교환한다 | 10 Transform | 시작 위치 저장, `glm::mix` |
| 지정한 면만 보이게 한다 | 11 FaceHinge | `visible[face]`, `indices` |
| 모서리나 꼭짓점을 중심으로 회전한다 | 11 FaceHinge | `Hinge`의 pivot과 축 |
| 면이 동시에 또는 순서대로 열린다 | 12 PyramidSequence | `angles`, 순차 조건 |
| 행성 주위를 달이 공전한다 | 13 OrbitHierarchy | 부모 프레임과 공전 반경 |
| 부모를 움직이면 자식도 따라간다 | 14 JointModel | `Turret`와 부모 행렬 |
| 카메라 이동 자전 공전 | 15 Camera | `eye`, `target`, `Key` |
| 원근 투영과 직교 투영을 바꾼다 | 15 Camera | `perspective`, 바탕의 `SetView` |
| 경로를 따라 같은 간격으로 이동한다 | 16 Conveyor | 거리 매개변수, `Path`, 개수 |
| 공간이 기울어지고 안의 물체가 떨어진다 | 17 BounceGravity | 공간 회전과 로컬 중력 |
| 원근도 평면도 정면도를 동시에 표시한다 | 18 MultiView | `Draw`의 viewport와 `Scene` |
| 로봇 팔 다리가 교차해서 흔들린다 | 19 RobotJump | `Draw`의 관절과 swing 부호 |
| 점프해서 장애물 위에 착지한다 | 19 RobotJump | `verticalSpeed`, 이전 높이와 지지면 |
| 문이 열리며 높이도 줄어든다 | 20 ShrinkingDoor | `Model`의 높이와 중심 보정 |
| 색과 크기가 부드럽게 왕복한다 | 21 PulseColor | `Blend`, `glm::mix`, 크기 범위 |
| 사각뿔을 육면체로 바꾼다 | 22 ShapeMorph | `Top`의 시작·끝 정점 |

## 수정할 함수부터 고르기

| 바꿀 요구사항 | 위치 |
|---|---|
| 초기 위치, 개수, 속도 | 기능 블록의 상태 변수와 `Reset()` |
| 키와 대소문자 구분 | `Key(key, action, mods)` |
| 클릭 생성, 선택, 놓기 | `Mouse(button, action)` |
| 마우스를 따라 이동 | `Cursor()` |
| 시간에 따라 움직임 | `Update(dt)` |
| 도형, 색, 부모 자식 관계 | `Draw()`와 모델 행렬 함수 |
| 화면 분할, 카메라, 투영 | `SetView()`와 `viewProjection` |
| 기능이 호출되지 않음 | 바탕의 `OnReset`, `OnKey`, `OnUpdate`, `OnDraw` 등록부 |

## 붙여넣기 전 세 가지 구분

1. **전체 실행 코드**에는 `main()`이 있습니다. 새 소스 하나를 통째로 채울 때 사용합니다.
2. **기능 블록**에는 `main()`이 없습니다. 기존 공통 바탕의 `FEATURE_BLOCK_END` 앞에 넣습니다.
3. **호출 등록부**는 기존 등록부와 합칩니다. `OnDraw()` 같은 함수가 두 번 정의되지 않게 합니다.

서로 다른 과제의 전체 코드를 한 파일에 이어 붙이지 않습니다. `main`, 콜백, 전역 변수, 셰이더 정의가 중복됩니다.

## 실행 결과가 이상할 때

| 증상 | 첫 확인 |
|---|---|
| `main already defined` 또는 중복 심볼 | 전체 예제 여러 개를 동시에 빌드했는지 확인 |
| `clamp`를 찾지 못함 | C++17 이상과 `<algorithm>` |
| `GL/glew.h`를 찾지 못함 | 헤더 포함 경로 |
| `unresolved external symbol` | 링크 라이브러리 세 개와 x64 일치 여부 |
| 실행 시 DLL 오류 | 동적 라이브러리 경로 |
| 배경만 보임 | Draw 등록, 카메라, 모델 위치, 크기 |
| 클릭이 엉뚱한 곳에 닿음 | 논리 화면 800×600과 마우스 변환 |
| 공전해야 하는데 자전함 | `R*T`와 `T*R` 순서 |
| 자식의 위치까지 커짐 | 부모 모델에 크기 행렬이 포함되었는지 확인 |
| 토글이 계속 켜졌다 꺼짐 | `GLFW_REPEAT`를 토글에서 제외 |
| 여러 화면 중 마지막 것만 남음 | 뷰포트마다 전체 `glClear`를 호출했는지 확인 |
| 장애물 밑에서 위로 순간이동 | 착지 검사에 이전 높이와 낙하 방향 포함 |

## 조합 연습

실제 출제를 예측한 목록이 아니라, 블록 조합을 익히기 위한 변형 연습입니다.

1. 03번에서 사각형을 5개로 늘리고, 선택한 사각형만 방향키로 이동시킵니다.
2. 07번을 슬롯 3개로 늘리고, 이미 사용한 슬롯에는 붙지 않게 합니다.
3. 10번의 직선 자리 교환을 두 반원 궤도를 따라 교환하도록 바꿉니다.
4. 14번 탱크에 15번 카메라를 합치고 키 충돌을 해결합니다. 완성 예시는 23번입니다.
5. 16번 컨베이어의 경로를 ㄱ자에서 사각형 폐곡선으로 바꿉니다.
6. 19번의 장애물을 배열 3개로 바꾸고 가장 높은 지지면을 선택합니다.
