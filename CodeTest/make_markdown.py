"""작은 기능 코드와 같은 번호의 사용 참고서를 생성하고 연결을 검증한다."""
from pathlib import Path
from urllib.parse import unquote
import re
import zipfile
from feature_catalog import FEATURES, GROUPS, SLOTS

ROOT=Path(__file__).resolve().parent
OUT=ROOT/"Markdown"
OUT.mkdir(exist_ok=True)
BT=chr(96)
FENCE=BT*3
BY_ID={f["id"]:f for f in FEATURES}
LIB="기능모음집.md"
REF="OpenGL_기능별_통합참고서.md"

def inline(s): return BT+s+BT
def code(s): return FENCE+"cpp\n"+s.strip()+"\n"+FENCE
def anchor(id): return '<a id="'+id.lower()+'"></a>'
def link(id,library=False):
    name=LIB if library else GROUPS[id[0]][0]
    return "["+id+" "+BY_ID[id]["title"]+"]("+name+"#"+id.lower()+")"
def quick_index(library):
    return "\n\n".join("**"+title+"** · "+" · ".join(link(f["id"],library) for f in FEATURES if f["id"][0]==group)
                       for group,(_,title) in GROUPS.items())

def argument_table(feature):
    rows=["| 호출 형태 | 순서·인자 | 의미·단위·변경 결과 |","|---|---|---|"]
    for signature,arguments in feature["arguments"]:
        assert arguments,signature
        for index,(parameter,meaning) in enumerate(arguments):
            assert parameter and meaning,(feature["id"],signature)
            rows.append("| "+(inline(signature) if index==0 else "")+" | "+inline(parameter)+" | "+meaning+" |")
    return "\n".join(rows)

library=["# 기능모음집","",
         "**코드 찾기 → 기능 번호 검색 → 표시된 위치에 복사.** 모든 코드는 [base.cpp](../base.cpp) 기준입니다.",
         "설명은 [사용 참고서]("+REF+")에서 같은 번호로 찾습니다. 함수 이름을 새로 만들지 않고 기존 On 함수 안에 넣습니다.",
         "한 기능의 필요한 코드 칸을 함께 복사합니다. 다른 기능과 합칠 때 좌표계와 상태 변수를 통일합니다.",
         "",quick_index(True),""]
for f in FEATURES:
    library += [anchor(f["id"]),"## "+f["id"]+" "+f["title"],"",
                f["mode"]+" · 검색: "+f["keywords"]+" · [사용법]("+GROUPS[f["id"][0]][0]+"#"+f["id"].lower()+")",""]
    for slot,label in SLOTS.items():
        if slot in f["blocks"]:
            library += ["**"+label+"**",code(f["blocks"][slot]),""]
docs={LIB:"\n".join(library)+"\n"}

for group,(filename,title) in GROUPS.items():
    parts=["# "+title+" — 기능 사용법","",
           "[기능모음집]("+LIB+") · [처음 사용](기본코드.md) · [인자 읽는 법](기본코드.md#arguments) · [조합](기능_조합.md)",""]
    for f in FEATURES:
        if f["id"][0]!=group:continue
        parts += [anchor(f["id"]),"## "+f["id"]+" "+f["title"],"",
                  "[복사할 코드]("+LIB+"#"+f["id"].lower()+") · "+f["mode"]+" · 검색: "+f["keywords"],""
                  ,"**하는 일:** "+f["use"],
                  "**붙일 곳:** "+" → ".join(label for slot,label in SLOTS.items() if slot in f["blocks"])+".",
                  "**인자 설명:**",argument_table(f),
                  "**바꿀 값:** "+f["change"],
                  "**주의:** "+f["caution"]]
        if f["related"]: parts += ["**함께 쓰기:** "+" · ".join(link(id) for id in f["related"])]
        parts += [""]
    docs[filename]="\n\n".join(parts).replace("\n\n\n\n","\n\n")+"\n"

docs["기본코드.md"]="""# 기능모음집 사용 시작

1. [base.cpp](../base.cpp)를 작업할 cpp 파일로 복사합니다. 창·셰이더·입력·시간 계산·기본 그리기 함수가 들어 있습니다.
2. [기능모음집](기능모음집.md)에서 필요한 기능 번호를 찾습니다.
3. 기능에 표시된 전역 코드와 On 함수 내부 코드를 각각 붙입니다.
4. [사용 참고서](OpenGL_기능별_통합참고서.md)에서 같은 번호의 수정할 값과 주의점을 확인합니다.

## 붙일 위치

| 표시 | 실제 위치 |
|---|---|
| 전역 | FEATURE_BLOCK_END 주석 바로 위. 다른 함수의 중괄호 바깥 |
| OnReset 내부 | 시작/F5 때 초기화. 이곳의 기본 use2D=false는 필요한 좌표계 값으로 교체 |
| OnKey 내부 | 키 한 번·토글. key/action/mods를 사용 |
| OnMouse 내부 | 클릭/놓기. button/action과 mouseX/mouseY를 사용 |
| OnCursor 내부 | 마우스 이동 |
| OnUpdate 내부 | 시간 갱신. dt는 직전 프레임 이후 지난 초 |
| OnDraw 내부 | 현재 상태를 그리기. 기본 노란 상자 한 줄은 선택한 그리기 코드로 교체 |

코드 칸은 함수 본문에 넣을 내용입니다. OnDraw 함수 안에 또 다른 OnDraw를 만들지 않습니다.
OnDraw에는 T, R, S로 만든 행렬을 그리기 함수에 전달합니다. 같은 물체의 위치·각도는 전역에 한 번 선언하고 입력/갱신에서 바꿉니다.

<a id="arguments"></a>
## 인자 설명 읽는 법

- 인자는 함수를 호출할 때 괄호 안에 전달하는 값입니다. 참고서의 각 기능 아래에서 호출 형태와 인자 순서를 확인합니다.
- R(30, {0,1,0})의 인자는 두 개입니다. 30은 회전각, {0,1,0} 전체가 회전축 벡터 하나입니다.
- Box(T(...) * R(...) * S(...), Yellow)의 인자도 두 개입니다. 행렬 곱 전체가 첫 번째, 색이 두 번째입니다.
- 벡터의 x/y/z 순서, 전체 크기와 반지름, 도와 라디안, 초와 초당 속도를 구분합니다. 3D 좌표 단위는 이 코드가 정한 단위이며 자동으로 미터가 되는 것은 아닙니다.
- filled=true처럼 기본값이 표시된 마지막 인자는 생략할 수 있습니다.
- (&)는 원본 변수를 수정하는 참조 인자입니다. ReflectAxis의 p/v, LandOnTop의 foot/vy는 함수 안에서 값이 바뀝니다.
- OnKey/OnMouse/OnUpdate의 입력값은 base의 콜백과 실행 루프가 전달합니다. OnReset/OnCursor/OnDraw는 인자가 없습니다.
- 표의 호출 형태는 인자 이름을 설명하는 표기입니다. 실제로 복사할 코드는 기능모음집을 사용합니다.

## 좌표계

- 2D: OnReset에서 use2D=true. 논리 화면은 800×600, 왼쪽 위가 (0,0), +Y가 아래입니다.
- 3D: use2D=false. +Y가 위입니다. base가 깊이 검사와 카메라·투영을 설정합니다.
- 서로 다른 좌표계의 기능을 그대로 합치지 않습니다. 전체 장면의 좌표계를 먼저 정합니다.
- Rect의 size는 전체 너비·높이, Circle의 값은 반지름입니다.
- Box는 원점 중심 한 변 1, Sphere는 원점 중심 반지름 1입니다.

## 복사할 때 구분

- T/R/S/Box/Rect/Circle/Sphere/HitRect/OverlapRect/SetView는 base에 있습니다. 다시 정의하지 않습니다.
- Markdown 원문에서 복사할 때 코드 칸을 감싼 백틱 표시는 제외합니다.
- OnReset의 공통 use2D 설정은 합친 장면에 맞춰 한 번만 둡니다.
- 전역 코드를 두 번 붙이지 않습니다. 같은 물체에 기능 두 개를 붙일 때는 기준 상태와 그리기 호출을 하나로 정합니다.
- main이 있는 cpp는 하나만 빌드합니다. 기본 코드는 C++17 이상/x64/GLFW/GLEW/GLM 설정이 필요합니다.
- [basic.cpp](../basic.cpp)는 도우미를 직접 추가하는 빈 바탕입니다. 이 모음집을 바로 쓰려면 base.cpp를 선택합니다.
"""

docs["기능_조합.md"]="""# 기능 번호로 조합하기

코드는 [기능모음집](기능모음집.md), 각 코드의 의미는 [사용 참고서](OpenGL_기능별_통합참고서.md)에서 확인합니다.

## 시소와 손잡이를 자동으로 함께 기울이기: T04 + A02

1. A02의 전역, OnReset, OnUpdate 코드를 넣습니다. waveAngle이 매 프레임 바뀝니다.
2. 그리기는 T04의 OnDraw 코드만 사용합니다.
3. T04의 angle 초기값을 15.0f에서 waveAngle로 바꿉니다.
4. 기존에 만든 손잡이들의 행렬 맨 앞에도 tilt *를 붙입니다. 받침대에는 붙이지 않습니다.

판과 손잡이는 모두 같은 tilt를 사용합니다. 가로봉의 기존 X축 90도 회전은 봉을 눕히는 동작이므로 유지합니다.
A02의 판 그리기까지 추가하면 판이 두 개 생깁니다.

## 드래그한 같은 도형을 목표점에 붙이기: I04 + C03

1. I04 전체를 붙여 드래그를 먼저 완성합니다.
2. C03의 별도 snapCenter와 그리기는 복사하지 않습니다. 실제 드래그 상태인 dragCenter를 사용합니다.
3. I04의 OnMouse 안에서 버튼을 놓을 때 dragging=false로 만드는 한 줄을 아래 코드로 교체합니다.

""" + code("""if (button==GLFW_MOUSE_BUTTON_LEFT && action==GLFW_RELEASE) {
    if (dragging && glm::length(dragCenter-glm::vec2(600,300))<=45)
        dragCenter={600,300};
    dragging=false;
}""") + """

목표점은 (600,300), 허용 거리는 45입니다. 판정 전에 dragging을 false로 바꾸면 방금 끌던 도형인지 알 수 없으므로 순서를 유지합니다.
목표 표시가 필요하면 OnDraw에 Rect({600,300},{100,100},Blue,false);를 추가합니다.

## 고른 객체만 축 회전하기: T08

1. T08의 전역, OnReset, OnKey, OnUpdate 코드를 각각 넣습니다. 예제는 y/Shift+y로 시작·방향 변경, s로 정지합니다.
2. turnAxis를 {1,0,0}, {0,1,0}, {0,0,1} 중 하나로 설정해 X/Y/Z축을 고릅니다. turnPivot은 회전축이 지나는 위치입니다.
3. T08의 OnDraw 코드에서 turn을 만드는 첫 줄을 기존 객체들보다 앞에 넣습니다.
4. 회전할 객체의 기존 모델 행렬 앞에 `turn *`를 붙입니다. 고정할 객체에는 붙이지 않습니다. 예제의 판·부품까지 추가로 그릴 필요는 없습니다.

### 그네의 진자 운동과 전체 Y축 회전 함께 쓰기: T08 + T04 + A02

- T08의 turnAxis={0,1,0}, turnPivot={0,0,0}으로 설정합니다.
- 줄 위쪽 중심으로 만든 기존 tilt는 유지합니다. 그네는 X축, 전체 회전은 Y축입니다.
- 기둥에는 `turn * 기존모델`, 줄·좌석·사람에는 `turn * tilt * 기존모델`을 사용합니다.
- 바닥과 아래 상자는 기존 모델로 그립니다. 위의 두 행렬을 붙이지 않습니다.
- 순서는 오른쪽부터 적용됩니다. `turn * tilt`는 그네를 기울인 뒤 전체를 Y축 회전합니다.
- 기존 tilt를 `turn * T(pivot) * R(angle,{1,0,0}) * T(-pivot)`로 바꾼 경우에는 줄·좌석·사람 앞에 tilt만 붙입니다. turn을 두 번 곱하지 않습니다.
- 진자 운동은 별도 실행 상태로 제어합니다. S키 처리에서 turnSpeed=0과 진자 실행 상태=false를 함께 설정하면 둘 다 멈춥니다.

## 클릭한 3D 바닥에 육면체 만들기: I06

1. I06의 전역 칸을 통째로 복사합니다. 위치 목록 floorBoxes, 크기·높이 설정과 MouseToFloor 함수가 함께 필요합니다.
2. OnReset에서는 use2D=false와 floorBoxes.clear()를 사용합니다.
3. OnMouse에서는 MouseToFloor(hit)가 true일 때 계산된 3D 중심을 저장합니다. I03의 mouseX/mouseY 저장 코드와 5개 제한은 이 기능에 포함되지 않습니다.
4. OnDraw에서는 바닥을 한 번 그리고, 목록의 각 위치 p에 Box(T(p)*S(...),Yellow)를 호출합니다. glm::mat4(1)만 전달하면 모든 박스가 원점에 겹칩니다.
5. 기존 구의 공전 등은 그대로 조합할 수 있습니다. I06에는 OnUpdate 코드가 필요하지 않습니다.

이미 바닥이 있다면 I06의 바닥 그리기 한 줄은 추가하지 않고, 기존 바닥의 윗면 높이와 범위에 floorY/floorHalfSize를 맞춥니다.
D11 체커 바닥과 조합할 때는 CheckerFloor(count,spacing,floorY)의 floorY를 공유하고 floorHalfSize=count*spacing/2로 설정합니다.
base의 Ground() 윗면은 -0.05이므로 그 바닥을 사용할 때는 floorY=-0.05f로 맞춥니다.
박스 중심 높이는 floorY+boxSize/2입니다. 박스 크기를 변경할 때는 코드의 boxSize를 바꾼 뒤 다시 실행합니다.

## 방 문제에서 찾을 기능

| 필요한 동작 | 기능 번호 |
|---|---|
| 방의 벽·바닥·천장, 내부가 보이게 만들기 | D12 |
| WASD 이동, 물체가 방 밖으로 나가지 않게 제한 | C08 |
| 공이 방의 여섯 경계에서 튕기기 | C09 |
| 장애물 옆면에 막히면 돌아서 걷기 | C10 |
| 방을 기울이면 낮은 쪽으로 미끄러지기 | M08 |
| 바닥을 열어서 물체 떨어뜨리기 | A08 |
| 방과 내부 물체 함께 회전 | T08 + D12 |
| 점프·장애물 위 착지 | C04 + C05 |
| 위쪽 고정 문 열기·축소 | A06 + A03 + I01 |
| 방을 보는 카메라 이동·공전 | P01 ~ P04 |

공통 좌표부터 정합니다. 기본 방은 low={-3,0,-4}, high={3,6,4}여서 가로 6·높이 6·깊이 8, 바닥은 y=0입니다.
상자는 중심 높이=바닥+반높이, 구는 중심 높이=바닥+반지름입니다. C04/C05의 발밑 높이와 중심 높이를 섞지 않습니다.
실습 21 원본의 바닥은 -3입니다. 그 좌표를 복사할 때는 방과 내부 물체의 높이 기준을 함께 맞춥니다.
실습 23 크기로 만들려면 low={-6,0,-6}, high={6,8,6}을 사용하고 C10의 X/Z 반범위도 6/6으로 맞춥니다.

## 방 안에서 공 튕기기: D12 + C09

1. D12의 DrawRoom 함수와 C09의 전역·OnReset·OnUpdate 코드를 붙입니다.
2. OnDraw는 아래 코드로 구성합니다. 방과 공이 같은 room 행렬과 bounceLow/bounceHigh를 공유합니다.
3. room의 각도를 바꿔도 공의 위치·속도·충돌은 방의 로컬 좌표로 계산합니다.

""" + code("""glm::mat4 room=R(20,{0,1,0})*R(10,{0,0,1});
DrawRoom(room,bounceLow,bounceHigh);
Sphere(room*T(ballPosition)*S(glm::vec3(ballRadius)),Yellow);""") + """

앞면은 보이지 않지만 C09에서는 앞쪽 벽에도 반사합니다. 출입구를 만들면 그 부분의 충돌 조건도 따로 바꿉니다.
실습 21처럼 기울기에 따라 바닥 상자가 미끄러지게 하려면 M08의 slideX/slideSpeed를 사용하고 바닥과 상자에 같은 Z축 기울기를 적용합니다.

## 방 안에서 장애물 피하며 걷기: D12 + C10

1. D12의 DrawRoom 함수와 C10의 전역·OnReset·OnUpdate 코드를 붙입니다.
2. OnDraw는 아래처럼 방, 장애물, 이동 물체를 같은 좌표로 그립니다.

""" + code("""glm::mat4 room(1);
DrawRoom(room,{-3,0,-4},{3,6,4});
for(auto obstacle:roomObstacles)
    Box(room*T(obstacle.center)*S(obstacle.size),Blue);
Box(room*T(walkerCenter)*S(walkerSize),Yellow);""") + """

로봇으로 바꿀 때는 마지막 Box를 로봇의 몸통·팔다리 그리기로 바꿉니다. 이동 기준이 발밑이면 walkerCenter.y에서 walkerSize.y/2를 뺀 위치를 로봇 전체의 T에 사용합니다.
점프를 붙일 때는 C04/C05를 별도의 상자로 더 그리지 않습니다. 같은 로봇의 발밑 높이와 수직 속도를 갱신하고, 장애물마다 C05의 LandOnTop으로 착지를 확인합니다. 여러 지지면 중 가장 높은 곳을 선택합니다.
C10의 walkerVelocity.y는 0으로 두고 중력은 별도로 한 번만 적용합니다. 방·장애물에 막힐 때 수평 이동만 반전하고 수직 속도는 유지합니다.

## 여는 바닥·앞문을 방에 붙이기

- 바닥: D12의 마지막 인자를 false로 지정해 고정 바닥을 생략합니다. 예: DrawRoom(room,{-3,0,-4},{3,6,4},false,true,false).
- A08의 바닥은 room * HatchModel(hatchAngle), 낙하 상자는 room * T({0,fallCenterY,0}) * S({0.6f,0.6f,0.6f})로 그립니다. A08 기본 바닥은 y=0, 가로 6·깊이 8입니다.
- 낙하가 시작되면 C08/C09의 아래쪽 경계 보정을 끕니다. 바닥 아래로 완전히 나간 물체는 방 옆벽 제한도 더 이상 적용하지 않는 방식으로 확장할 수 있습니다.
- 앞문: D12의 front=false를 유지하고 A06의 DoorModel을 따로 그립니다. 모델 앞에 room * T({0,0,4})를 붙이면 앞쪽 z=4에 배치됩니다.
- 문으로 앞면 전체를 덮으려면 A06의 닫힌 높이와 경첩 높이를 6, 너비를 6으로 맞춥니다. A03의 진행률을 0~1로 바꾸어 DoorModel에 전달하고 I01 패턴으로 열림 목표를 토글합니다.
- 면을 숨기거나 문을 열어도 물리 경계는 자동으로 바뀌지 않습니다. 통과 여부는 충돌 조건에서도 함께 처리합니다.

## 조건이 맞을 때만 이동·보간 시작: C02 + A03

C02의 OverlapRect 판정은 true/false를 반환합니다. OnKey 또는 OnUpdate에서 이 조건을 검사해 시작 위치를 저장하고 진행률을 0으로 설정합니다.
OnUpdate에서는 실행 상태가 true일 때만 A03의 진행률을 갱신합니다. 시작 위치와 진행률을 매 프레임 다시 초기화하지 않습니다.

## 조합 확인

- 같은 물체의 위치·각도·진행률을 어느 변수가 소유하는지 정합니다.
- 같은 물체는 필요한 횟수만 그립니다. 예제마다 있는 그리기를 모두 더하지 않습니다.
- 키가 겹치면 변경하거나 수정키로 구분합니다.
- Reset은 서로의 상태나 좌표계를 덮어쓰지 않게 합칩니다.
- Update는 프레임당 한 번입니다. 다중 화면에서는 같은 상태를 여러 번 그리기만 합니다.
"""

docs["README.md"]="""# 먼저 열 파일 두 개

1. [기능모음집](기능모음집.md): 작은 기능의 코드만 찾고 복사합니다.
2. [사용 참고서](OpenGL_기능별_통합참고서.md): 같은 번호로 붙일 위치·각 인자·바꿀 값·주의점을 확인합니다.

[처음 사용하는 방법](기본코드.md) · [기능을 조합하는 방법](기능_조합.md)

모든 코드는 base.cpp 기준입니다. D06=육면체, T04=시소·손잡이 함께 회전, I04=드래그처럼 번호로 찾으면 빠릅니다.
"""+"\n\n"+quick_index(False)+f"""

## 자료 범위

{len(FEATURES)}개 작은 기능으로 도형·변환·입력·이동·충돌·애니메이션·카메라를 정리했습니다.
한 기능은 필요한 선언과 호출 위치를 함께 제공합니다. 실습 문제 전체 답안을 그대로 복사하는 형식은 아닙니다.
기존 22개 기능 블록과 26개 완성 예제는 CodeTest의 blocks/examples에 보존했습니다. 긴 설명은 archives의 이전판에 보관했습니다.
"""

parts=["# 기능모음집 사용 참고서","",
       "**코드는 [기능모음집](기능모음집.md), 사용법은 이 파일에서 같은 번호로 찾습니다.**",
       "[처음 사용하는 방법](기본코드.md) · [인자 읽는 법](기본코드.md#arguments) · [조합 방법](기능_조합.md)","",
       quick_index(False),""]
for filename,title in GROUPS.values():
    parts += ["---",docs[filename],""]
docs[REF]="\n".join(parts)
docs={name:text.rstrip()+"\n" for name,text in docs.items()}

for name,text in docs.items():
    (OUT/name).write_text(text,encoding="utf-8-sig")
readme="""# CodeTest

**먼저 아래 두 파일만 열면 됩니다.**

- [기능모음집](Markdown/기능모음집.md): 기능 번호·검색어·복붙 코드.
- [사용 참고서](Markdown/OpenGL_기능별_통합참고서.md): 같은 번호의 붙일 위치·인자 순서와 의미·수정할 값·조합 설명.

[처음 사용](Markdown/기본코드.md) · [기능 조합](Markdown/기능_조합.md) · [base.cpp](base.cpp)

| 찾는 기능 | 검색 번호 |
|---|---|
| 상자·받침대·판 | D06 |
| 격자무늬 바닥 | D10 |
| 두 색이 번갈아 채워지는 체커 바닥 | D11 |
| 판과 손잡이 함께 회전 | T04 |
| 고른 객체만 X·Y·Z축 회전·시작·정지 | T08 |
| 자동 왕복 기울기 | A02 |
| 클릭한 3D 바닥 위치에 육면체 생성 | I06 |
| 방 만들기·벽·바닥·천장 | D12 |
| 방 경계 제한·공 반사·장애물 충돌 | C08 / C09 / C10 |
| 경사면 미끄러짐·바닥 열기와 낙하 | M08 / A08 |
| 드래그와 스냅 | I04 + C03 |
| 점프·착지 | C04 / C05 |
| 공전·위성 | T06 / T07 |
| 카메라·투영·화면 분할 | P01 ~ P05 |

## 파일 구성

- Markdown: 기능모음집 1개와 사용 설명 문서 11개. 기능 번호는 실습 문제 번호와 다릅니다.
- base.cpp: 이 모음집의 기본 코드. basic.cpp는 도우미를 직접 추가하는 최소 바탕입니다.
- blocks/examples: 기존 기능 블록 22개와 완성 예제 26개.
- archives: 이전 상세 참고서와 압축 파일. 이전 설명이 필요할 때만 엽니다.
- OpenGL_기능별_MD_참고서.zip: 최신 Markdown, base.cpp, basic.cpp, 시작 안내를 포함한 휴대용 묶음.
- feature_catalog.py / make_markdown.py: 코드와 설명의 원본 및 생성기.
- validate_catalog.py: 기능 코드 연결·논리·숨김 OpenGL 검사 준비 도구.
- HTML 및 이전 GUIDE/INDEX/COMBINE/QUICK_REFERENCE 문서는 이전 자료로 보관합니다.

시험 PC에 C++17 이상/x64/GLFW/GLEW/GLM과 링크 라이브러리를 미리 준비합니다.
인터넷 없이 문서를 열고 코드를 복사할 수 있습니다. cpp는 main이 있는 파일 하나만 빌드합니다.
"""
(ROOT/"README.md").write_text(readme,encoding="utf-8-sig")

def verify():
    assert len(docs)==12
    for name,text in docs.items():
        assert len(re.findall(r"^"+FENCE+r".*$",text,re.M))%2==0,name
        prose=re.sub(r"^"+FENCE+r"[^\n]*\n.*?^"+FENCE+r"[ \t]*$","",text,flags=re.M|re.S)
        for target in re.findall(r"\]\(([^)]+)\)",prose):
            filename,_,fragment=unquote(target).partition("#")
            path=OUT/filename
            assert path.is_file(),(name,target)
            if fragment:assert anchor(fragment) in path.read_text(encoding="utf-8-sig"),(name,target)
    for f in FEATURES:
        assert docs[LIB].count(anchor(f["id"]))==1
        assert docs[REF].count(anchor(f["id"]))==1
        assert anchor(f["id"]) in docs[GROUPS[f["id"][0]][0]]
        for snippet in f["blocks"].values():assert code(snippet) in docs[LIB],f["id"]
        for filename in (GROUPS[f["id"][0]][0],REF):
            section=docs[filename].split(anchor(f["id"]),1)[1].split('<a id="',1)[0]
            assert argument_table(f) in section,(filename,f["id"])
    assert docs[REF].count("**인자 설명:**")==len(FEATURES)
    assert "void OnDraw()" not in docs[LIB]  # Only function-body snippets, no duplicate callbacks.
verify()
archive=ROOT/"OpenGL_기능별_MD_참고서.zip"
files=[ROOT/"README.md",ROOT/"base.cpp",ROOT/"basic.cpp",*(OUT/name for name in docs)]
with zipfile.ZipFile(archive,"w",zipfile.ZIP_DEFLATED) as z:
    for path in files:z.write(path,path.relative_to(ROOT))
with zipfile.ZipFile(archive) as z:
    assert z.testzip() is None
    for path in files:assert z.read(path.relative_to(ROOT).as_posix())==path.read_bytes()
print(f"PASS: {len(FEATURES)} feature IDs and argument tables, 1 code collection + 11 reference files; code/link/anchor/ZIP checks.")
