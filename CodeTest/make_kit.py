"""기능 블록을 공통 바탕에 삽입해 복붙용 전체 예제와 오프라인 색인을 만든다."""
from pathlib import Path
import html
import json
import re

ROOT = Path(__file__).resolve().parent
BASE = (ROOT / 'base.cpp').read_text(encoding='utf-8-sig')
# 번호, 제목, 관련 실습, 키, 변경할 값, 합칠 때 주의할 점
INFO = [
('01','배경색과 키 입력 및 주기 실행','1','C와 Shift+C 색, Space 자동 변경','Update의 1.0f 주기, background','토글은 GLFW_PRESS에서만 처리한다.'),
('02','화면 사분면과 개수 제한 생성','2, 8','왼쪽 클릭 생성','counts[q]<5, 크기 60, 400과 300 경계','CanvasWidth=800, CanvasHeight=600을 기준으로 한다.'),
('03','겹친 도형 선택과 드래그','3, 7, 10','왼쪽 드래그','items 초기값, size, 경계 제한','마지막 도형부터 검사한다. 클릭 위치와 중심의 차이를 유지한다.'),
('04','지우개 충돌과 도형 분할','5, 6','왼쪽 클릭 4분할, 오른쪽 누른 채 지우기','분할 하한 20, 속도 40, 지우개 50','vector에 추가하기 전 원본을 복사한다. 오른쪽 버튼을 놓으면 지우기를 멈춘다.'),
('05','점 선 삼각형 사각형 원','7, 8','1~5 도형, F 채움, 오른쪽 클릭 크기','정점 목록, size, 크기 상하한','점과 선에는 채움 전환을 적용하지 않는다.'),
('06','반사 지그재그 나선 경로와 방향 회전','4, 9','1 반사, 2 지그재그, 3 나선','velocity, 경계 25~775, 경로 속도, path 상한 500','2D는 Y가 아래로 증가한다. 삼각형의 기본 앞쪽은 +X이다.'),
('07','가까운 슬롯에 스냅하고 고정','10','왼쪽 드래그 후 놓기','slot, threshold=45, 도형 크기','여러 슬롯으로 늘리면 모양 일치와 슬롯 점유 여부도 검사한다.'),
('08','격자 이동 경계 장애물 지그재그','11','방향키, Space 자동 이동','N=8, obstacleRow, obstacleCol, 주기 .2','이 예제는 장애물에서 정지한다. 원래 실습의 모양 교환은 충돌 분기에 추가한다.'),
('09','이동 도형 타이밍 맞추기와 쌓기','12','Space 맞추기','허용 오차 35, 일치 구역 220~380, 이동 시간 .8','보간 시작 위치를 저장한다. stack 상한 8을 둔다.'),
('10','객체 선택 이동 회전 크기 자리 교환','14, 17','1/2 선택, X/Y/Z 이동, S 크기, Shift 반대, R 자전, Space 교환','position, size, 회전 속도 60, 교환 시간 2','T*R*S는 물체 제자리 자전이다. 교환 중 추가 이동 입력은 다음 보간에서 덮어쓴다.'),
('11','육면체 면 선택과 모서리 경첩','13, 15','1~6 면 표시, O 앞면 개폐','pivot, 회전축, 목표각 100, 속도 80','원본 정점의 모서리 pivot을 T(pivot)*R*T(-pivot)으로 고정한다.'),
('12','사각뿔 동시 및 순차 펼치기','16','O 개폐, S 순차 전환','목표각 120, 속도 60, 면 순서','120도는 이 예제의 목표각이다. 바닥과 정확히 일치해야 하면 사각뿔 높이와 밑변으로 각도를 계산한다.'),
('13','자전 공전 달 계층 궤도','17, 18','Space 정지','radius, orbit/spin/moonOrbit 속도, 크기','달은 행성 위치 프레임을 상속한다. 행성 자전과 크기는 상속하지 않는다.'),
('14','탱크 부모 자식과 포신 관절','19, 22','좌우 이동, Y 몸체, T 포탑, B 포신, Shift 반대','Turret, 관절 위치, Box 크기','자식 부모에는 크기 적용 전 행렬을 전달한다.'),
('15','카메라 이동 자전 공전 투영','19, 20, 22, 23','X 옆 이동, Z 앞 이동, Y 자전, O 공전, Shift 반대, P 투영','eye, target, 이동 .3, 회전 5','카메라만 가져오면 Draw는 등록하지 않는다. eye와 target이 같거나 시선과 up이 평행하면 안 된다.'),
('16','꺾인 경로와 일정 간격 객체','20','Space 정지','First=6, Second=4, 개수 5, 속도 1.5','끝에서 시작으로 순간 재배치한다. 연속 폐곡선이 필요하면 복귀 구간을 추가한다.'),
('17','공 반사 중력 기울어진 공간','21','B 공 추가, 왼쪽 드래그 기울이기, O 바닥','반지름 .3, 경계 2.7, 중력 9.8, 반발계수 .8','공끼리 충돌은 포함하지 않는다. 로컬 위치와 월드 중력을 구분한다.'),
('18','한 장면을 여러 뷰포트에 표시','22','좌우 객체 이동','Draw의 화면 분할, SetView 카메라','위에서 볼 때 up=(0,0,-1). 상태 갱신은 프레임당 한 번만 한다.'),
('19','로봇 보행 장애물 점프 착지','23','WASD 보행, Space 정지, J 점프, +/- 속도','speed, verticalSpeed=5, 중력, 장애물 크기','position.y는 발밑 높이이다. 장애물 위 착지는 이전 높이도 검사한다.'),
('20','경첩 고정 문 회전과 높이 축소','23','O 개폐','Model의 문 높이 4, 폭 3, 각도 -90','크기가 줄면 중심 오프셋도 바꾼다. 윗모서리를 고정한다.'),
('21','크기와 색의 부드러운 왕복 변화','4, 18','Space 정지','sin(time*2) 주기, 크기 .5~1.5, 색 양 끝','sin의 -1~1을 .5+.5*sin으로 0~1 보간 계수로 바꾼다.'),
('22','사각뿔에서 육면체로 정점 보간','16','M 모양 전환','Top의 정점, Update 진행 속도','위쪽 정점 4개를 꼭짓점 하나에서 사각형으로 펼친다.'),
]
HOOKS = {
 'Reset': ('void OnReset()', ''),
 'Key': ('void OnKey(int key,int action,int mods)', 'key,action,mods'),
 'Mouse': ('void OnMouse(int button,int action)', 'button,action'),
 'Cursor': ('void OnCursor()', ''),
 'Update': ('void OnUpdate(float dt)', 'dt'),
 'Draw': ('void OnDraw()', ''),
 'Check': ('void OnCheck()', ''),
}

def insert(section, value, source):
    pattern = rf'// {section}_BEGIN\n.*?// {section}_END'
    result, count = re.subn(pattern, lambda _: f'// {section}_BEGIN\n{value}\n// {section}_END', source, flags=re.S)
    assert count == 1, section
    return result

def hooks(blocks, omit_draw=()):
    result=[]
    explanations={'Reset':'초기 상태 설정. 시작과 F5에서 호출한다.',
      'Key':'키 전달. 키가 겹치면 if/else로 한 기능에만 전달한다.',
      'Mouse':'마우스 버튼 눌림/놓임 전달.', 'Cursor':'마우스를 움직일 때 호출한다.',
      'Update':'dt는 초 단위. 각 기능을 프레임당 한 번 갱신한다.',
      'Draw':'장면을 그린다. 보조 기능의 예제 물체가 필요 없으면 해당 Draw를 빼면 된다.',
      'Check':'--check 실행에서만 사용하는 기본 설정 검사.'}
    for name,(signature,args) in HOOKS.items():
        calls=[]
        for namespace, code in blocks:
            if name=='Draw' and namespace in omit_draw:
                continue
            if re.search(rf'\bvoid\s+{name}\s*\(',code):
                calls.append(f'{namespace}::{name}({args});')
        result.append('// '+explanations[name]+'\n'+signature+'\n{\n'+
                      ('\n'.join('    '+call for call in calls) if calls else '    // 이 예제에서는 연결할 기능이 없다.')+'\n}')
    return '\n\n'.join(result)

def full_code(blocks, registration):
    source=insert('FEATURE_BLOCK','\n\n'.join(code for _,code in blocks),BASE)
    return insert('HOOK_BLOCK',registration,source)

examples=ROOT/'examples';examples.mkdir(exist_ok=True)
records=[]
for number,title,exercise,keys,knobs,caution in INFO:
    paths=list((ROOT/'blocks').glob(number+'_*.inc'))
    assert len(paths)==1, number
    code=paths[0].read_text(encoding='utf-8-sig')
    namespace=re.search(r'^namespace\s+(\w+)',code,re.M)[1]
    registration=hooks([(namespace,code)])
    full=full_code([(namespace,code)],registration)
    assert code in full and 'int main(' in full
    filename=paths[0].stem+'.cpp'
    (examples/filename).write_text(full,encoding='utf-8-sig')
    records.append(dict(number=number,title=title,exercise=exercise,keys=keys,knobs=knobs,caution=caution,
                        namespace=namespace,block=code,hooks=registration,full=full,filename=filename))

for number,title,indices,omit in [('23','조합 예제 탱크와 카메라',[13,14],('Camera',)),
                                ('24','조합 예제 드래그와 배경색 타이머',[0,2],('ColorInput',))]:
    chosen=[records[i] for i in indices]
    blocks=[(r['namespace'],r['block']) for r in chosen]
    registration=hooks(blocks,omit)
    if number=='23':
        registration=re.sub(r'void OnKey[^\{]+\{[^}]*\}',
            'void OnKey(int key,int action,int mods)\n{\n    // Ctrl로 카메라 입력을 분리해 Y키 충돌을 막는다.\n    if(mods & GLFW_MOD_CONTROL)\n        Camera::Key(key,action,mods);\n    else\n        JointModel::Key(key,action,mods);\n}',registration)
    full=full_code(blocks,registration)
    filename=number+'_combined.cpp';(examples/filename).write_text(full,encoding='utf-8-sig')
    keys=' / '.join(r['keys'] for r in chosen)
    if number=='23':
        keys=chosen[0]['keys']+' / Ctrl를 함께 누르면 카메라: '+chosen[1]['keys']
    records.append(dict(number=number,title=title,exercise='조합',keys=keys,
        knobs='선택한 기능 블록의 값을 수정한다.',caution='보조 기능의 Draw는 등록에서 제외해 중복 장면을 없앴다.',
        namespace='+'.join(r['namespace'] for r in chosen),block='\n\n'.join(c for _,c in blocks),
        hooks=registration,full=full,filename=filename))

# 같은 장면에 예제를 나란히 놓는 것뿐 아니라, 한 물체의 상태와 계산을 공유하는 조합.
shared_combinations=[('25','같은 행성에 공전과 색 크기 변화 적용',[12,20],
'''// 두 계산의 시간을 초기화한다. PulseColor의 예제 구는 그리지 않는다.
void OnReset() { OrbitHierarchy::Reset(); PulseColor::Reset(); }
void OnKey(int key,int action,int mods)
{
    OrbitHierarchy::Key(key,action,mods); // Space: 공전 정지/재개.
    if(key==GLFW_KEY_S && action==GLFW_PRESS)
        PulseColor::running=!PulseColor::running; // S: 색·크기 변화만 정지/재개.
}
void OnMouse(int button,int action) {}
void OnCursor() {}
void OnUpdate(float dt) { OrbitHierarchy::Update(dt); PulseColor::Update(dt); }
void OnDraw()
{
    Sphere(S(glm::vec3(.6f)), Yellow); // 중심별. 이 크기는 조합 장면에서만 사용하는 값.
    float u=PulseColor::Blend(); // 다른 기능의 계산 결과(0~1)를 가져온다.
    float size=glm::mix(PulseColor::MinimumRadius,PulseColor::MaximumRadius,u);
    glm::vec3 color=glm::mix(PulseColor::StartColor,PulseColor::EndColor,u);
    // 위치는 공전 기능, 크기·색은 맥동 기능에서 얻어 '같은 상자 하나'에 적용한다.
    Box(OrbitHierarchy::PlanetFrame()*R(OrbitHierarchy::spin,{0,1,0})*S(glm::vec3(size)),color);
}
void OnCheck()
{
    OrbitHierarchy::Check(); PulseColor::Check(); OnReset();
    OnUpdate(.5f);
    Require(Near(OrbitHierarchy::orbit,15)&&Near(PulseColor::time,.5f),"shared orbit and pulse update");
    OnKey(GLFW_KEY_S,GLFW_PRESS,0); OnUpdate(.5f);
    Require(Near(OrbitHierarchy::orbit,30)&&Near(PulseColor::time,.5f),"independent animation controls");
}''','Space 공전 정지, S 색·크기 정지','OrbitRadius와 PulseColor의 최소·최대 크기를 바꾼다. 이 조합에서는 크기를 상자의 한 변으로 사용한다.'),
('26','드래그로 선택한 같은 물체에 스냅 적용',[2,6],
'''void OnReset() { DragSelect::Reset(); SnapMatch::Reset(); }
void OnKey(int key,int action,int mods) {}
void OnMouse(int button,int action)
{
    // 놓기 전 드래그 여부를 저장한다. DragSelect::Mouse가 놓기에서 dragging을 끄기 때문이다.
    bool wasDragging=DragSelect::dragging;
    DragSelect::Mouse(button,action);
    int i=DragSelect::selected;
    if(button==GLFW_MOUSE_BUTTON_LEFT && action==GLFW_RELEASE && wasDragging && i>=0)
        SnapMatch::TrySnap(DragSelect::items[i].center,SnapMatch::TargetCenter,SnapMatch::SnapDistance);
    // SnapMatch::Mouse/Cursor는 등록하지 않는다. 별도 position을 움직이지 않도록 한다.
}
void OnCursor() { DragSelect::Cursor(); }
void OnUpdate(float dt) {}
void OnDraw()
{
    Rect(SnapMatch::TargetCenter,DragSelect::InitialSize+glm::vec2(10),Yellow,false);
    DragSelect::Draw(); // DragSelect의 실제 items 배열만 그린다.
}
void OnCheck()
{
    DragSelect::Check(); SnapMatch::Check(); OnReset();
    mouseX=DragSelect::FirstCenter.x-DragSelect::InitialSize.x/3;
    mouseY=DragSelect::FirstCenter.y;OnMouse(GLFW_MOUSE_BUTTON_LEFT,GLFW_PRESS);
    Require(DragSelect::selected==0,"select exposed part of first object");
    mouseX=SnapMatch::TargetCenter.x-DragSelect::offset.x+10;
    mouseY=SnapMatch::TargetCenter.y-DragSelect::offset.y;
    OnCursor();OnMouse(GLFW_MOUSE_BUTTON_LEFT,GLFW_RELEASE);
    Require(glm::length(DragSelect::items[0].center-SnapMatch::TargetCenter)<.001f,"selected object snaps");
    Require(glm::length(DragSelect::items[1].center-DragSelect::SecondCenter)<.001f,"other object unchanged");
}''','왼쪽 드래그 후 목표 근처에서 놓기','InitialSize는 그림·클릭·목표 테두리를 함께 바꾼다. SnapDistance를 키우면 쉽게 붙는다. 여러 물체의 목표 중복 점유는 허용한다.')]
for number,title,indices,registration,keys,caution in shared_combinations:
    chosen=[records[i] for i in indices]
    blocks=[(r['namespace'],r['block']) for r in chosen]
    full=full_code(blocks,registration);filename=number+'_shared_object.cpp'
    (examples/filename).write_text(full,encoding='utf-8-sig')
    records.append(dict(number=number,title=title,exercise='상태 공유 조합',keys=keys,
        knobs='각 namespace 상단의 [수정] 설정값과 OnDraw/OnMouse를 참고한다.',caution=caution,
        namespace='+'.join(r['namespace'] for r in chosen),block='\n\n'.join(c for _,c in blocks),
        hooks=registration,full=full,filename=filename))

# 코드 안의 실제 설정 선언을 설명 화면에도 사용한다. 별도 설명과 코드가 어긋나지 않게 한다.
for r in records:
    sections=re.findall(r'namespace\s+(\w+)\s*\{\s*// ===== 1[^\n]*\n(.*?)// ===== 2',r['block'],re.S)
    settings=[]
    for name,body in sections:
        settings.extend(['// '+name,*body.strip().splitlines(),''])
    namespaces=re.findall(r'^namespace\s+(\w+)',r['block'],re.M)
    r['settings']='\n'.join(settings)
    r['guide']='\n'.join([
        '// [조합 카드] '+r['title'],
        '// 1. 전체 실행 코드는 새 .cpp 하나에 붙인다. 기존 main 옆에 추가하지 않는다.',
        '// 2. 추가 기능은 기능 블록의 namespace 전체를 FEATURE_BLOCK_END 앞에 붙인다.',
        '// 3. 호출 등록부의 함수가 이미 있다면 함수 내부 호출만 합친다.',
        '// 4. 같은 물체에 적용하려면 위치·크기 상태를 하나로 정하고 계산 결과를 가져온다.',
        '// 5. 두 Reset이 use2D나 카메라를 덮어쓰지 않는지, 키가 겹치는지 확인한다.',
        '// 이 코드의 이름공간: '+', '.join(namespaces),
        '// 키: '+r['keys'], '// 주의: '+r['caution'],
        '// 아래 설정의 오른쪽 숫자를 수정한다. 바뀌는 결과와 단위는 각 줄 주석에 있다.',
        *['// '+line for line in settings],
        '// 같은 물체를 공유하는 완성 예제: 25번(행성), 26번(드래그와 스냅).'])

table=['# OpenGL 기능별 복붙 색인','',
       '먼저 `index.html`을 브라우저에서 연다. 인터넷 없이 검색하고 전체 코드, 기능 블록, 호출 등록부를 복사할 수 있다.',
       '사용법과 조합 규칙은 `GUIDE.md`를 참고한다. 모든 예제는 자체 셰이더를 포함한다. GLFW, GLEW, GLM 설치와 링크 설정은 필요하다.','',
       '| 번호 | 기능 | 관련 실습 | 키 | 수정할 값 |','|---|---|---|---|---|']
for r in records:
    table.append(f"| {r['number']} | [{r['title']}](examples/{r['filename']}) | {r['exercise']} | {r['keys']} | {r['knobs']} |")
table += ['', '## 기능별 주의점','']
for r in records:
    table += [f"### {r['number']} {r['title']}",'',r['caution'],'']
(ROOT/'INDEX.md').write_text('\n'.join(table),encoding='utf-8-sig')

static_parts=['''<!doctype html><html lang="ko"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>OpenGL 스크립트 없는 열람판</title><style>body{max-width:1000px;margin:auto;padding:18px;font:16px/1.7 system-ui,sans-serif;color:#182335;background:#fff}a{color:#145ca5}pre{padding:14px;background:#f0f3f7;white-space:pre-wrap;overflow-wrap:anywhere;font:13px/1.6 Consolas,monospace}section{border-top:2px solid #ccd5e0;margin-top:32px}li{margin:8px 0}</style>
<h1 id="top">OpenGL 스크립트 없는 열람판</h1><p>인터넷과 JavaScript 없이 목차와 전체 코드를 읽는 자료입니다. 아래 목차를 누르거나 브라우저의 <b>페이지에서 찾기</b>로 검색하세요. 자체 검색창과 복사 버튼은 없으며 코드를 직접 선택해 복사합니다.</p>
<p><b>공전:</b> <a href="#feature-13">13 행성과 달 공전</a> · <a href="#feature-15">15 카메라 공전</a> · <a href="#feature-23">23 탱크와 카메라 조합</a></p>
<p>전체 코드 하나를 새 .cpp에 붙입니다. 기능을 추가할 때는 namespace 블록과 OnReset·OnKey·OnUpdate·OnDraw 호출을 합칩니다. C++17 이상, GLFW·GLEW·GLM 설치가 필요합니다.</p><ol>''']
for r in records:
    static_parts.append(f'<li><a href="#feature-{r["number"]}">{html.escape(r["number"]+" "+r["title"])}</a></li>')
static_parts.append('</ol>')
for r in records:
    static_parts.append(f'<section id="feature-{r["number"]}"><h2>{html.escape(r["number"]+" "+r["title"])}</h2><p>조작: {html.escape(r["keys"])}</p><p>변경: {html.escape(r["knobs"])}</p><p>주의: {html.escape(r["caution"])}</p><p><a href="#top">목차로</a></p><h3>설정과 조합 설명</h3><pre>{html.escape(r["guide"])}</pre><h3>전체 실행 코드</h3><pre><code>{html.escape(r["full"])}</code></pre></section>')
static_parts.append('</html>')
static_html='\n'.join(static_parts)
(ROOT/'offline.html').write_text('\n'.join(line.rstrip() for line in static_html.splitlines())+'\n',encoding='utf-8')

data=json.dumps(records,ensure_ascii=False).replace('</','<\\/')
page='''<!doctype html><html lang="ko"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>OpenGL 기능별 복붙 참고서</title>
<style>
*{box-sizing:border-box}body{margin:0;background:#101722;color:#e6edf6;font:16px/1.6 system-ui,sans-serif}
header{padding:22px 28px;border-bottom:1px solid #344155}h1{font-size:24px;margin:0}a{color:#9bd5ff}small{color:#b2c2d7}
.layout{display:grid;grid-template-columns:310px minmax(0,1fr);gap:20px;padding:20px}nav{max-height:82vh;overflow:auto}
input{width:100%;padding:12px;margin-bottom:12px;background:#172336;color:white;border:1px solid #6b829f;border-radius:8px;font:inherit}
button{cursor:pointer;font:inherit;border:1px solid #617993;border-radius:6px;padding:8px 12px;background:#20324a;color:white}
button:focus-visible,input:focus-visible,textarea:focus-visible{outline:3px solid #f7cf67;outline-offset:2px}
nav button{display:block;width:100%;text-align:left;margin-bottom:7px}button[aria-pressed=true]{background:#254d6a;border-color:#88cef7}
h2{margin:0;font-size:22px}.info{background:#172336;border-radius:8px;padding:16px;margin:12px 0}.info p{margin:4px 0}
.tools{display:flex;gap:8px;flex-wrap:wrap;align-items:center;margin:12px 0}textarea{width:100%;height:58vh;resize:vertical;white-space:pre;tab-size:4;background:#0a1019;color:#dce9f9;border:1px solid #435672;padding:16px;font:14px/1.6 Consolas,monospace}
#status{color:#f7cf67}summary{cursor:pointer}details{margin-top:12px}code{color:#b8e0ff}@media(max-width:850px){.layout{grid-template-columns:1fr}nav{max-height:260px}textarea{height:55vh}}
</style>
<header><h1>OpenGL 기능별 복붙 참고서</h1><small>실습 1~23의 패턴 · 설명이 포함된 기능 블록 22개 · 조합 예제 4개 · 오프라인 사용</small>
<p><a href="COMBINE.md">같은 물체에 기능을 합치는 방법</a></p>
<p><a href="offline.html">휴대폰·미리보기용 스크립트 없는 열람판</a></p>
<p id="runtimeNotice" role="status">검색 기능을 시작하지 못했다면 이 미리보기에서 스크립트가 제한될 수 있습니다. 위의 열람판을 사용하세요. 인터넷은 필요하지 않습니다.</p>
<details><summary>복붙 순서와 조합 규칙</summary><ol><li>가장 가까운 기능을 선택하고 <b>전체 실행 코드</b>를 새 .cpp 하나에 복사한다.</li><li>다른 기능은 <b>기능 블록</b>의 namespace 전체를 FEATURE_BLOCK_END 앞에 붙인다.</li><li><b>호출 등록부</b>의 호출문을 기존 OnReset / OnKey / OnUpdate / OnDraw 등에 합친다. 함수 정의 자체를 중복해서 붙이지 않는다.</li><li>카메라처럼 보조 기능만 필요하면 그 기능의 Draw 호출을 제외한다. 조합 예제 23·24 참고.</li><li>2D와 3D의 Reset을 무작정 섞지 않는다. 같은 화면의 좌표계와 카메라 설정은 하나로 결정한다.</li></ol><p>F5 초기화 · Esc 종료. x64, C++17 이상. 링크: opengl32.lib glew32.lib glfw3.lib. 기존 실습과 키는 다를 수 있으므로 각 항목의 키를 확인한다.</p><p><a href="GUIDE.md">상세 사용법</a> · <a href="QUICK_REFERENCE.md">문제별 기능 선택표</a> · <a href="INDEX.md">텍스트 색인</a> · <a href="base.cpp">공통 바탕</a></p></details></header>
<div class="layout"><aside><label for="search">기능 검색</label><input id="search" disabled placeholder="예: 드래그, 공전, 점프, 카메라"><p id="resultCount" role="status" aria-live="polite"></p><nav id="list" aria-label="기능 목록"></nav></aside>
<main><h2 id="title"></h2><div class="info"><p id="keys"></p><p id="knobs"></p><p id="caution"></p><details open><summary>이 값을 바꾸면 무엇이 달라지나</summary><pre id="settings" style="white-space:pre-wrap;overflow-wrap:anywhere"></pre></details></div>
<div class="tools"><button id="full" type="button">전체 실행 코드</button><button id="block" type="button">기능 블록</button><button id="hooks" type="button">호출 등록부</button><button id="guide" type="button">조합 설명</button><button id="copy" type="button">코드 복사</button><a id="download">.cpp 파일</a><span id="status" role="status" aria-live="polite"></span></div>
<label for="code">선택한 코드 — 직접 선택 후 Ctrl+C도 가능</label><textarea id="code" spellcheck="false" readonly></textarea></main></div>
<script>
const records=__DATA__;let selected=0,mode='full';const $=id=>document.getElementById(id);
function show(){const r=records[selected];for(const m of ['full','block','hooks','guide','copy'])$(m).disabled=!r;$('status').textContent='';if(!r){$('title').textContent='검색 결과 없음';for(const id of ['keys','knobs','caution','settings'])$(id).textContent='';$('code').value='';$('download').removeAttribute('href');return;}$('title').textContent=r.number+' '+r.title;$('keys').textContent='조작: '+r.keys;$('knobs').textContent='변경: '+r.knobs;$('caution').textContent='주의: '+r.caution;$('settings').textContent=r.settings;$('code').value=r[mode];$('download').href='examples/'+r.filename;$('download').download=r.filename;for(const m of ['full','block','hooks','guide'])$(m).setAttribute('aria-pressed',m===mode?'true':'false');for(const b of $('list').children)b.setAttribute('aria-pressed',Number(b.dataset.index)===selected?'true':'false');}
function findMatches(query){query=query.trim().toLowerCase();return records.filter(r=>(r.title+' '+r.exercise+' '+r.keys+' '+r.knobs+' '+r.namespace+' '+r.settings).toLowerCase().includes(query));}
function list(){const matches=findMatches($('search').value);$('list').replaceChildren();selected=matches.length?records.indexOf(matches[0]):-1;$('resultCount').textContent=matches.length+'개 결과'+(matches.length?' · 첫 결과가 아래 코드에 표시됩니다.':' · 다른 단어로 검색하세요.');matches.forEach(r=>{const i=records.indexOf(r),b=document.createElement('button');b.type='button';b.dataset.index=i;b.textContent=r.number+' '+r.title;b.onclick=()=>{selected=i;show();};$('list').append(b);});show();}
$('search').addEventListener('input',list);for(const m of ['full','block','hooks','guide'])$(m).onclick=()=>{mode=m;show();};
$('copy').onclick=async()=>{try{if(!navigator.clipboard)throw Error('fallback');await navigator.clipboard.writeText($('code').value);$('status').textContent='복사 완료';}catch(e){$('code').focus();$('code').select();let ok=false;try{ok=document.execCommand('copy');}catch(ignore){}$('status').textContent=ok?'복사 완료':'선택된 코드를 직접 복사하세요';}};list();$('search').disabled=false;$('runtimeNotice').hidden=true;
</script></html>'''.replace('__DATA__',data)
(ROOT/'index.html').write_text(page,encoding='utf-8')
assert len(records)==26
assert all('ReadShaderFile(' not in r['full'] for r in records)
print('Generated: 22 annotated feature blocks, 26 complete examples, settings and composition cards.')
