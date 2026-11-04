"""기능 코드와 사용법의 단일 원본. G=전역, R/K/M/C/U/D=기존 On 함수 내부."""
from textwrap import dedent

GROUPS = {
    "D": ("도형_그리기.md", "도형 그리기"),
    "T": ("변환_공전_관절.md", "변환·공전·관절"),
    "I": ("입력_선택_드래그.md", "입력·선택·드래그"),
    "M": ("이동_경로.md", "이동·경로"),
    "C": ("충돌_스냅_점프.md", "충돌·스냅·점프"),
    "A": ("시간_보간_애니메이션.md", "시간·보간·애니메이션"),
    "P": ("카메라_투영_화면분할.md", "카메라·투영·화면 분할"),
}
SLOTS = {"G": "전역: FEATURE_BLOCK_END 위", "R": "OnReset 내부",
         "K": "OnKey 내부", "M": "OnMouse 내부", "C": "OnCursor 내부",
         "U": "OnUpdate 내부", "D": "OnDraw 내부"}
FEATURES = []

def add(id, title, keywords, mode, blocks, use, change, caution, related=(), check=""):
    blocks = {k: dedent(v).strip() for k,v in blocks.items()}
    setup = "use2D = true;" if mode == "2D" else "use2D = false;"
    blocks["R"] = setup + ("\n" + blocks["R"] if blocks.get("R") else "")
    FEATURES.append(dict(id=id,title=title,keywords=keywords,mode=mode,blocks=blocks,
                         use=use,change=change,caution=caution,related=related,
                         check=dedent(check).strip()))

add("D01","점","정점 점 크기", "2D", {"D":"""
glPointSize(12.0f);
Primitive(GL_POINTS, {{400,300,0}}, glm::mat4(1), Yellow);
"""}, "화면의 한 위치를 점으로 표시합니다.", "400,300=위치, 12=점의 픽셀 크기, Yellow=색.",
    "점 크기의 지원 범위는 그래픽 환경에 따라 다릅니다.", ("D02",))
add("D02","선·연결선·테두리","직선 선분 궤적 윤곽", "2D", {"D":"""
Primitive(GL_LINES, {{100,100,0},{600,400,0}}, glm::mat4(1), Blue);
"""}, "두 점을 잇는 선분입니다.", "좌표 두 개를 바꿉니다. GL_LINE_STRIP은 여러 점을 순서대로 연결하고 GL_LINE_LOOP는 마지막과 처음도 연결합니다.",
    "GL_LINES는 두 정점씩 독립된 선을 만듭니다.", ("D03",))
add("D03","삼각형","정점 채우기", "2D", {"D":"""
Primitive(GL_TRIANGLES, {{400,150,0},{250,400,0},{550,400,0}},
          glm::mat4(1), Red);
"""}, "정점 세 개로 삼각형을 그립니다.", "정점의 x,y와 마지막 색을 바꿉니다.",
    "정점 세 개가 한 직선에 있으면 면이 보이지 않습니다.", ("D02","D08"))
add("D04","사각형","직사각형 네모 크기 중심", "2D", {"D":"""
Rect({400,300}, {200,100}, Yellow);
"""}, "중심 좌표와 전체 너비·높이를 받는 2D 사각형입니다.",
    "400,300=중심. 200,100=전체 너비·높이. 네 번째 인수 false를 추가하면 테두리입니다.",
    "왼쪽 위 좌표가 아닌 중심 좌표입니다. 반너비를 전달하지 않습니다.", ("C01","I04"))
add("D05","원","동그라미 반지름", "2D", {"D":"""
Circle({400,300}, 60.0f, Blue);
"""}, "중심과 반지름으로 원을 그립니다.", "60=반지름이므로 지름은 120입니다. 네 번째 인수 false는 원 테두리입니다.",
    "사각형의 전체 크기와 원의 반지름을 구분합니다.", ("D04","D07"))
add("D06","육면체·바닥","상자 받침대 시소 판 크기", "3D", {"D":"""
Box(T({0,0.7f,0}) * S({0.4f,1.4f,0.8f}), Yellow);
"""}, "원점 중심 한 변 1인 상자를 늘리고 옮깁니다. 받침대·판·바닥을 같은 Box로 만듭니다.",
    "S의 x,y,z=가로·높이·깊이. T의 y를 높이/2로 두면 아랫면이 y=0입니다. 바닥 예: T({0,-0.1f,0})*S({10,0.2f,10}).",
    "한 번 호출하면 상자 하나입니다. 여러 개는 Box를 여러 번 호출합니다.", ("T01","T03","T04"))
add("D07","구","공 반지름", "3D", {"D":"""
Sphere(T({0,1,0}) * S({1,1,1}), Green);
"""}, "원점 중심 반지름 1인 구를 배치합니다.", "S({2,2,2})는 반지름 2입니다. T의 y를 반지름으로 맞추면 바닥에 닿습니다.",
    "Box는 기본 한 변 1, Sphere는 기본 반지름 1입니다.", ("D06",))
add("D08","사각뿔","피라미드 꼭짓점 면", "3D", {"D":"""
const glm::vec3 b[] = {{-1,0,1},{1,0,1},{1,0,-1},{-1,0,-1}};
const glm::vec3 apex(0,2,0);
for (int i=0; i<4; ++i)
    Primitive(GL_TRIANGLES, {b[i],b[(i+1)%4],apex}, glm::mat4(1), Blue);
Primitive(GL_TRIANGLE_FAN, {b[3],b[2],b[1],b[0]}, glm::mat4(1), Yellow);
"""}, "네 개의 삼각형 옆면과 사각형 밑면을 그립니다.", "apex.y=높이. b의 x,z=밑면 모서리 위치.",
    "면 하나만 회전하려면 해당 Primitive의 glm::mat4(1)을 회전 행렬로 바꿉니다.", ("D09","T04","A07"))
add("D09","면 하나 그리기","면 선택 경첩 문 정점", "3D", {"D":"""
Primitive(GL_TRIANGLE_FAN,
          {{-1,0,0},{1,0,0},{1,2,0},{-1,2,0}}, glm::mat4(1), Yellow);
"""}, "문이나 육면체의 한 면처럼 네 정점을 따로 그립니다.",
    "좌표 네 개가 면의 모양입니다. 특정 면만 보이게 하려면 그 면의 호출을 if 조건으로 감쌉니다.",
    "정점은 면의 둘레 순서로 지정합니다. 면 선택 상태는 I01의 bool 패턴을 씁니다.", ("I01","T04"))

add("D10","격자 바닥","격자무늬 격자무뉘 그리드 바닥 평면 칸 간격 GridFloor", "3D", {"G":"""
void GridFloor(int halfCount=5, float spacing=1.0f, float floorY=0.0f,
               glm::vec3 floorColor={0.35f,0.35f,0.35f},
               glm::vec3 lineColor={0.7f,0.7f,0.7f})
{
    if (halfCount<1 || spacing<=0) return;
    float edge=halfCount*spacing;
    Box(T({0,floorY-0.1f,0})*S({edge*2,0.2f,edge*2}),floorColor);
    float lineY=floorY+0.01f;
    for (int i=-halfCount; i<=halfCount; ++i) {
        float p=i*spacing;
        Primitive(GL_LINES, {{-edge,lineY,p},{edge,lineY,p},
                             {p,lineY,-edge},{p,lineY,edge}},
                  glm::mat4(1),lineColor);
    }
}
""", "D":"GridFloor(5, 1.0f, 0.0f);"},
    "원점 중심의 XZ 평면에 색이 있는 바닥과 격자선을 그립니다. 다른 모델과 무관하게 사용할 수 있습니다.",
    "halfCount=한쪽 칸 수, spacing=한 칸 길이, floorY=바닥 윗면 높이. 기본값은 10×10칸, 전체 크기 10×10입니다. 네 번째·다섯 번째 인자를 추가하면 바닥색·선색도 바뀝니다.",
    "전역에 GridFloor 함수를 한 번 넣고 OnDraw에서 호출합니다. 같은 위치의 기존 Ground() 호출은 교체합니다. OnUpdate에는 그리기를 넣지 않습니다. 선은 바닥보다 0.01 높게 그립니다. halfCount<1 또는 spacing<=0이면 아무것도 그리지 않습니다.",
    ("D02","D06"), """
auto verifyGrid=[](int n,float spacing,float height) {
    gridProbeVertices.clear();gridProbeBoxes=0;
    GridFloor(n,spacing,height,{0.2f,0.3f,0.4f},{0.8f,0.7f,0.6f});
    Require(gridProbeBoxes==1,"one grid floor");
    Require(gridProbeVertices.size()==size_t(4*(2*n+1)),"grid line count");
    glm::vec3 top(gridProbeModel*glm::vec4(0,0.5f,0,1));
    Require(Near(top.y,height),"floor top height");
    float edge=n*spacing;
    Require(Near(gridProbeModel[0][0],2*edge)&&Near(gridProbeModel[2][2],2*edge),"grid floor extent");
    for(size_t i=0;i<gridProbeVertices.size();i+=4) {
        auto a=gridProbeVertices[i],b=gridProbeVertices[i+1];
        auto c=gridProbeVertices[i+2],d=gridProbeVertices[i+3];
        Require(Near(a.x,-edge)&&Near(b.x,edge)&&Near(a.z,b.z),"X grid line");
        Require(Near(c.z,-edge)&&Near(d.z,edge)&&Near(c.x,d.x),"Z grid line");
        Require(Near(a.y,height+0.01f)&&Near(b.y,a.y)&&Near(c.y,a.y)&&Near(d.y,a.y),"line offset");
        Require(a.z>=-edge-0.001f&&a.z<=edge+0.001f&&Near(a.z,c.x),"grid bounds");
        if(i>0) Require(Near(a.z-gridProbeVertices[i-4].z,spacing),"grid spacing");
    }
    Require(glm::length(gridProbeFloorColor-glm::vec3(0.2f,0.3f,0.4f))<0.001f,"floor color");
    Require(glm::length(gridProbeLineColor-glm::vec3(0.8f,0.7f,0.6f))<0.001f,"line color");
};
verifyGrid(5,1,0);verifyGrid(2,0.5f,1.25f);verifyGrid(1,2,-0.5f);
gridProbeVertices.clear();gridProbeBoxes=0;
GridFloor(0,1);GridFloor(5,0);GridFloor(5,-1);
Require(gridProbeBoxes==0&&gridProbeVertices.empty(),"invalid grid settings");
""")

add("D11","체커 바닥","체커보드 체커바닥 채커 체크무늬 격자무늬 두 색 번갈아 지그재그 바둑판 타일 CheckerFloor", "3D", {"G":"""
void CheckerFloor(int count=10, float spacing=1.0f, float floorY=0.0f,
                  glm::vec3 colorA={0.2f,0.2f,0.2f},
                  glm::vec3 colorB={0.8f,0.8f,0.8f})
{
    if (count<1 || spacing<=0) return;
    float edge=count*spacing/2.0f;
    for (int z=0; z<count; ++z)
        for (int x=0; x<count; ++x) {
            float left=x*spacing-edge, back=z*spacing-edge;
            glm::vec3 color=(x+z)%2==0 ? colorA : colorB;
            Primitive(GL_TRIANGLE_FAN,
                      {{left,floorY,back},{left,floorY,back+spacing},
                       {left+spacing,floorY,back+spacing},{left+spacing,floorY,back}},
                      glm::mat4(1),color);
        }
}
""", "D":"CheckerFloor(10, 1.0f, 0.0f);"},
    "XZ 평면을 정사각형 타일로 채우고 두 색을 가로·세로로 번갈아 배치합니다. 모양을 빠르게 확인하려면 첫 인자를 3으로 바꾸어 3×3칸으로 그립니다.",
    "count=한 변의 전체 칸 수, spacing=한 칸 길이, floorY=바닥 높이. 네 번째·다섯 번째 인자 colorA/colorB가 번갈아 채워집니다. (x+z)%2가 색을 결정하므로 다음 줄의 시작 색도 바뀝니다.",
    "전역에 CheckerFloor 함수를 한 번 넣고 OnDraw에서 호출합니다. 타일은 두께 없는 면이며 기존 바닥과 같은 높이에 겹쳐 그리지 않습니다. D10 GridFloor는 선 격자용으로 별도 유지됩니다. D10의 halfCount와 달리 count=3이면 전체가 3×3칸입니다.",
    ("D10","D09"), """
auto verifyChecker=[](int n,float spacing,float height) {
    checkerProbeTiles.clear();checkerProbeColors.clear();
    glm::vec3 a(0.2f,0.3f,0.4f),b(0.8f,0.7f,0.6f);
    CheckerFloor(n,spacing,height,a,b);
    Require(checkerProbeTiles.size()==size_t(n*n),"checker tile count");
    int countA=0;
    float edge=n*spacing/2.0f;
    for(int z=0;z<n;++z) for(int x=0;x<n;++x) {
        size_t index=size_t(z*n+x);
        const auto& tile=checkerProbeTiles[index];
        const auto color=checkerProbeColors[index];
        bool isA=glm::length(color-a)<0.001f;
        Require(isA||glm::length(color-b)<0.001f,"two specified colors");
        countA+=isA ? 1 : 0;
        if(x>0) Require(glm::length(color-checkerProbeColors[index-1])>0.1f,"horizontal alternation");
        if(z>0) Require(glm::length(color-checkerProbeColors[index-n])>0.1f,"vertical alternation");
        Require(tile.size()==4,"four tile vertices");
        float minX=tile[0].x,maxX=tile[0].x,minZ=tile[0].z,maxZ=tile[0].z;
        for(auto p:tile) {
            Require(Near(p.y,height),"checker height");
            minX=std::min(minX,p.x);maxX=std::max(maxX,p.x);
            minZ=std::min(minZ,p.z);maxZ=std::max(maxZ,p.z);
        }
        Require(Near(minX,x*spacing-edge)&&Near(maxX,minX+spacing),"tile X bounds");
        Require(Near(minZ,z*spacing-edge)&&Near(maxZ,minZ+spacing),"tile Z bounds");
        Require(glm::cross(tile[1]-tile[0],tile[2]-tile[0]).y>0,"upward tile winding");
    }
    Require(glm::length(checkerProbeColors[0]-a)<0.001f&&countA==(n*n+1)/2,"checker first color and balance");
};
verifyChecker(3,1,0);verifyChecker(4,0.5f,1.25f);verifyChecker(1,2,-0.5f);
checkerProbeTiles.clear();checkerProbeColors.clear();
CheckerFloor(0,1);CheckerFloor(-1,1);CheckerFloor(3,0);CheckerFloor(3,-1);
Require(checkerProbeTiles.empty(),"invalid checker settings");
""")

add("D12","방 만들기·앞면·천장·바닥 선택","방 공간 무대 벽 안쪽 면 컬링 Room 실습21 실습23", "3D", {"G":"""
void DrawRoom(glm::mat4 parent,glm::vec3 low,glm::vec3 high,
              bool front=false,bool ceiling=true,bool floor=true) {
    if (low.x>=high.x || low.y>=high.y || low.z>=high.z) return;
    glm::vec3 p[]={{low.x,low.y,high.z},{high.x,low.y,high.z},
        {high.x,high.y,high.z},{low.x,high.y,high.z},
        {low.x,low.y,low.z},{high.x,low.y,low.z},
        {high.x,high.y,low.z},{low.x,high.y,low.z}};
    const int faces[6][4]={{0,1,2,3},{1,5,6,2},{5,4,7,6},
        {4,0,3,7},{3,2,6,7},{4,5,1,0}};
    const glm::vec3 colors[]={{0.6f,0.4f,0.4f},{0.3f,0.5f,0.7f},
        {0.4f,0.6f,0.5f},{0.6f,0.5f,0.3f},{0.5f,0.5f,0.6f},{0.35f,0.4f,0.45f}};
    // 방 안쪽만 보이게 한다. 바깥에서 가까운 벽 너머로 내부를 볼 수 있다.
    GLboolean wasCulling=glIsEnabled(GL_CULL_FACE);
    GLint oldFront,oldCull;
    glGetIntegerv(GL_FRONT_FACE,&oldFront);
    glGetIntegerv(GL_CULL_FACE_MODE,&oldCull);
    glEnable(GL_CULL_FACE);glFrontFace(GL_CCW);glCullFace(GL_BACK);
    for(int f=0;f<6;++f) {
        if ((f==0&&!front)||(f==4&&!ceiling)||(f==5&&!floor)) continue;
        Primitive(GL_TRIANGLE_FAN,{p[faces[f][3]],p[faces[f][2]],
            p[faces[f][1]],p[faces[f][0]]},parent,colors[f]);
    }
    glFrontFace(oldFront);glCullFace(oldCull);
    if (!wasCulling) glDisable(GL_CULL_FACE);
}
""", "D":"""
glm::mat4 room(1);
DrawRoom(room,{-3,0,-4},{3,6,4});
Box(room*T({0,0.5f,0}),Yellow);
"""}, "안쪽이 보이는 얇은 면으로 방을 만듭니다. 기본 크기는 가로 6·높이 6·깊이 8, 바닥 높이 0입니다. 앞면(+Z)을 생략하고 나머지 다섯 면을 그립니다. 그리기 전 컬링 설정은 호출 후 복원합니다.",
    "low/high는 방 안쪽의 최소/최대 좌표이며 각 성분 low<high로 설정합니다. front/ceiling/floor는 각 면 표시 여부입니다. room은 방 전체의 부모 행렬입니다. 벽 색은 colors 배열의 앞·오른쪽·뒤·왼쪽·천장·바닥 순서로 바꿉니다.",
    "이 함수는 그리기만 합니다. 충돌 경계는 C08/C09에 같은 low/high를 전달합니다. 면을 숨겨도 충돌은 자동으로 꺼지지 않습니다. 내부 물체에도 room *를 붙이고, 위치·충돌 계산은 회전 전 방 좌표로 유지합니다. 전체 크기는 high-low로 정하며 parent에 음수 배율을 넣으면 면 방향이 뒤집힙니다.",
    ("C08","C09","C10","T08","A08"), """
const glm::vec3 low(-3,0,-4),high(3,6,4);
GLboolean saved=glIsEnabled(GL_CULL_FACE);GLint savedFront,savedCull;
glGetIntegerv(GL_FRONT_FACE,&savedFront);glGetIntegerv(GL_CULL_FACE_MODE,&savedCull);
for(bool enabled:{false,true}) {
    if(enabled) glEnable(GL_CULL_FACE);else glDisable(GL_CULL_FACE);
    glFrontFace(GL_CW);glCullFace(GL_FRONT);
    roomProbeFaces.clear();
    glm::mat4 parent=T({1,2,-1})*R(30,{0,1,0});
    DrawRoom(parent,low,high);
    Require(roomProbeFaces.size()==5,"room default omits front face");
    glm::vec3 center(parent*glm::vec4((low+high)/2.0f,1));
    for(auto face:roomProbeFaces) {
        glm::vec3 normal=glm::cross(face[1]-face[0],face[2]-face[0]);
        Require(glm::dot(normal,center-face[0])>0,"room faces inward after parent rotation");
    }
    GLint front,cull;glGetIntegerv(GL_FRONT_FACE,&front);glGetIntegerv(GL_CULL_FACE_MODE,&cull);
    Require(bool(glIsEnabled(GL_CULL_FACE))==enabled&&front==GL_CW&&cull==GL_FRONT,
        "room restores previous culling state");
}
roomProbeFaces.clear();DrawRoom(glm::mat4(1),low,high,true,true,true);
Require(roomProbeFaces.size()==6,"all room faces");
roomProbeFaces.clear();DrawRoom(glm::mat4(1),low,high,false,false,false);
Require(roomProbeFaces.size()==3,"room without front ceiling and floor");
roomProbeFaces.clear();DrawRoom(glm::mat4(1),high,low);
Require(roomProbeFaces.empty(),"invalid room bounds ignored");
glFrontFace(savedFront);glCullFace(savedCull);
if(saved) glEnable(GL_CULL_FACE);else glDisable(GL_CULL_FACE);
""")

add("T01","이동","위치 옮기기 T", "3D", {"D":"""
Box(T({2,1,0}), Yellow);
"""}, "상자 중심을 지정한 좌표로 옮깁니다.", "T의 x,y,z가 중심 위치입니다.",
    "그리기에서 매번 T({2,1,0})를 쓰면 그 위치에 고정됩니다. 계속 움직이려면 상태를 갱신합니다.", ("M01","I02"))
add("T02","제자리 회전","자전 각도 R", "3D", {"D":"""
Box(T({0,1,0}) * R(30, {0,1,0}), Blue);
"""}, "상자의 중심을 유지하며 회전합니다.", "30=도 단위. {1,0,0}=X축, {0,1,0}=Y축, {0,0,1}=Z축.",
    "행렬은 오른쪽부터 적용됩니다. T*R은 회전 후 배치입니다.", ("T04","T08","A02"))
add("T03","크기 변경","확대 축소 스케일 S", "3D", {"D":"""
Box(T({0,1.5f,0}) * S({6,0.2f,0.8f}), Red);
"""}, "각 축을 다른 비율로 늘려 긴 판을 만듭니다.", "6=길이, 0.2=두께, 0.8=앞뒤 폭.",
    "판만 늘려도 손잡이 위치는 자동으로 바뀌지 않습니다.", ("D06","T04"))
add("T04","여러 부품 함께 회전","시소 손잡이 중심 고정 경첩 pivot tilt 묶어서 기울이기", "3D", {"D":"""
const glm::vec3 pivot(0,1.5f,0);
const float angle = 15.0f;
glm::mat4 tilt = T(pivot) * R(angle,{0,0,1}) * T(-pivot);
Box(T({0,0.7f,0}) * S({0.4f,1.4f,0.8f}), Yellow);
Box(tilt * T({0,1.5f,0}) * S({6,0.2f,0.8f}), Red);
Box(tilt * T({2,1.75f,0}) * S({0.1f,0.3f,0.1f}), Blue);
"""}, "이미 배치한 판과 손잡이를 같은 중심으로 회전합니다. 마지막 Box는 손잡이 기둥 하나입니다.",
    "angle=공통 각도. pivot=고정할 중심. 함께 움직일 모든 기존 Box 행렬의 맨 앞에 tilt *를 붙입니다. 받침대에는 붙이지 않습니다.",
    "기존 좌표를 유지합니다. 높이에서 pivot을 또 빼지 않습니다. 기존 기울기 회전과 중복하지 않고, 가로봉을 눕힌 R(90,{1,0,0})는 유지합니다.",
    ("T02","T05","T08","A02"), """
const glm::vec3 pivot(0,1.5f,0);
for(float a : {0.0f,15.0f,-15.0f}) {
    glm::mat4 m=T(pivot)*R(a,{0,0,1})*T(-pivot);
    Require(glm::length(glm::vec3(m*glm::vec4(pivot,1))-pivot)<0.001f,"pivot fixed");
    glm::vec3 p(2,1.8f,0.4f), q(m*glm::vec4(p,1));
    float r=glm::radians(a);
    glm::vec3 expected(std::cos(r)*2-std::sin(r)*0.3f,
                       1.5f+std::sin(r)*2+std::cos(r)*0.3f,0.4f);
    Require(glm::length(q-expected)<0.001f,"group rotation");
}
""")
add("T05","관절에 부품 붙이기","부모 자식 팔 다리 포신 피벗", "3D", {"D":"""
glm::mat4 joint = T({0,2,0}) * R(30,{0,0,1});
Box(joint * T({0,-0.5f,0}) * S({0.2f,1,0.2f}), Blue);
Box(joint * T({0,-1.1f,0}) * S({0.4f,0.2f,0.4f}), Yellow);
"""}, "관절을 기준으로 길이 1인 팔과 그 끝의 부품을 배치합니다.",
    "joint의 T=관절 위치, R=관절 각도. 팔 중심은 관절에서 길이/2만큼 아래에 둡니다.",
    "T04는 기존 배치 좌표에 추가 회전합니다. T05는 처음부터 관절에 대한 상대 위치로 배치합니다. joint에 팔의 S를 포함하지 않습니다.", ("T04","A02"))
add("T06","공전","행성 궤도 반지름", "3D", {"G":"float orbitAngle = 0;",
"R":"orbitAngle = 0;", "U":"orbitAngle += 30.0f * dt;", "D":"""
glm::mat4 orbit = T({0,0,0}) * R(orbitAngle,{0,1,0}) * T({3,0,0});
Sphere(orbit * S({0.5f,0.5f,0.5f}), Blue);
"""}, "중심에서 거리 3인 궤도로 공전합니다.", "30=도/초, T({3,0,0})의 3=공전 반경, 맨 앞 T=궤도 중심.",
    "공전 행렬의 회전은 물체 방향에도 적용됩니다. 구는 대칭이라 방향 변화가 보이지 않습니다.", ("T07","T02"))
add("T07","부모를 따라 도는 위성","달 행성 계층 공전", "3D", {"G":"float planetAngle=0, moonAngle=0;",
"R":"planetAngle=moonAngle=0;", "U":"planetAngle += 20*dt;\nmoonAngle += 60*dt;", "D":"""
glm::mat4 planet = R(planetAngle,{0,1,0}) * T({3,0,0});
glm::mat4 moon = planet * R(moonAngle,{0,1,0}) * T({1,0,0});
Sphere(planet * S({0.5f,0.5f,0.5f}), Blue);
Sphere(moon * S({0.2f,0.2f,0.2f}), Yellow);
"""}, "행성과 달을 그리며 달은 행성의 좌표계를 따라갑니다.",
    "20=행성 공전 속도, 60=부모 기준 달 공전 속도, 3=행성 거리, 1=달 거리.",
    "planet에 행성 크기 S를 넣지 않습니다. 넣으면 달의 거리와 크기도 함께 바뀝니다.", ("T06","T05"))

add("T08","선택한 객체 축 회전·시작·정지","객체 골라서 선택 회전 X축 Y축 Z축 축 방향 중심 양 음 y Y Shift 속도 정지 그네 전체 회전", "3D", {"G":"""
float turnAngle=0, turnSpeed=0;
const float turnRate=30.0f; // 회전 속도: 도/초
glm::vec3 turnAxis(0,1,0); // X={1,0,0}, Y={0,1,0}, Z={0,0,1}
glm::vec3 turnPivot(0,1,0); // 회전축이 지나는 중심 위치
""", "R":"turnAngle=turnSpeed=0;", "K":"""
if (action==GLFW_PRESS) {
    if (key==GLFW_KEY_Y)
        turnSpeed=(mods & GLFW_MOD_SHIFT) ? -turnRate : turnRate;
    if (key==GLFW_KEY_S) turnSpeed=0;
}
""", "U":"turnAngle += turnSpeed * dt;", "D":"""
glm::mat4 turn=T(turnPivot)*R(turnAngle,turnAxis)*T(-turnPivot);
Ground();
// 고정할 객체: 기존 행렬만 사용
Box(T({3,0.25f,0})*S({0.5f,0.5f,0.5f}),Green);
// 회전할 객체: 기존 행렬의 맨 앞에 turn * 추가
Box(turn*T({0,1,0})*S({2,0.2f,0.8f}),Red);
Box(turn*T({0.7f,1.4f,0})*S({0.2f,0.6f,0.2f}),Blue);
"""}, "그리기 코드에서 turn *를 붙인 객체만 같은 축과 중심으로 회전합니다. 예제는 빨간 판과 파란 부품이 함께 회전하고 바닥과 초록 상자는 고정됩니다. y로 양의 방향, Shift+y로 음의 방향 회전을 시작하며 s로 현재 각도에서 멈춥니다. 키를 놓아도 계속 회전합니다.",
    "turnAxis={1,0,0}이면 X축, {0,1,0}이면 Y축, {0,0,1}이면 Z축입니다. turnPivot은 회전축이 지나는 장면 좌표 {x,y,z}입니다. turnRate는 도/초이며 클수록 빨라집니다. turnAngle은 현재 각도, turnSpeed는 부호가 있는 속도입니다. 원하는 기존 Box/Sphere/Primitive의 모델 행렬 맨 앞에 turn *를 붙여 대상을 고릅니다. Y축만 원점 중심으로 돌리려면 turnAxis={0,1,0}, turnPivot={0,0,0}으로 설정합니다.",
    "turnAxis에 {0,0,0}을 넣지 않습니다. 이 기능은 선택한 객체를 하나의 공통 축으로 회전합니다. 각 객체의 제자리 회전은 T02입니다. 다른 기울기와 합칠 때는 turn * tilt * 기존모델 순서입니다. tilt에 turn을 이미 포함했다면 다시 곱하지 않습니다. S키로 다른 애니메이션도 멈추려면 그 실행 상태도 함께 끕니다. 키 입력은 기존 OnKey에 합치고, turnAngle 갱신은 OnUpdate에 한 번만 넣습니다.",
    ("T02","T04","I01","A02"), """
OnReset(); OnUpdate(1);
Require(Near(turnAngle,0)&&Near(turnSpeed,0),"rotation starts stopped");
OnKey(GLFW_KEY_Y,GLFW_REPEAT,0); OnKey(GLFW_KEY_Y,GLFW_RELEASE,0);
Require(Near(turnSpeed,0),"rotation ignores repeat and release");
OnKey(GLFW_KEY_Y,GLFW_PRESS,0); OnUpdate(0.5f);
Require(Near(turnAngle,15),"positive rotation speed in seconds");
OnKey(GLFW_KEY_Y,GLFW_PRESS,GLFW_MOD_SHIFT); OnUpdate(0.25f);
Require(Near(turnAngle,7.5f),"negative rotation with shift");
OnKey(GLFW_KEY_Y,GLFW_RELEASE,0);
Require(Near(turnSpeed,-turnRate),"release preserves direction");
OnKey(GLFW_KEY_S,GLFW_PRESS,0); OnUpdate(1);
Require(Near(turnAngle,7.5f)&&Near(turnSpeed,0),"stop preserves angle");
OnKey(GLFW_KEY_Y,GLFW_PRESS,0); OnUpdate(0.25f);
Require(Near(turnAngle,15),"rotation resumes from stopped angle");
OnReset();
Require(Near(turnAngle,0)&&Near(turnSpeed,0),"reset stops and clears rotation");
const glm::mat4 models[]={T({3,0.25f,0})*S({0.5f,0.5f,0.5f}),
    T({0,1,0})*S({2,0.2f,0.8f}), T({0.7f,1.4f,0})*S({0.2f,0.6f,0.2f})};
const glm::vec3 axes[]={{1,0,0},{0,1,0},{0,0,1}};
turnPivot={1,2,3};
for(auto axis:axes) for(float degrees:{-90.0f,0.0f,90.0f}) {
    turnAxis=axis;turnAngle=degrees;rotationProbeModels.clear();
    BeginFrame();OnDraw();
    Require(rotationProbeModels.size()==3,"selected rotation draws three objects");
    float a=glm::radians(degrees),c=std::cos(a),s=std::sin(a);
    for(int i=0;i<3;++i) for(float x:{-0.5f,0.5f})
        for(float y:{-0.5f,0.5f}) for(float z:{-0.5f,0.5f}) {
            glm::vec4 corner(x,y,z,1);
            glm::vec3 p(models[i]*corner),expected=p;
            if(i>0) {
                glm::vec3 v=p-turnPivot;
                expected=turnPivot+v*c+glm::cross(axis,v)*s+axis*glm::dot(axis,v)*(1-c);
            }
            glm::vec3 actual(rotationProbeModels[i]*corner);
            Require(glm::length(actual-expected)<0.001f,"selected object axis and pivot; unselected fixed");
        }
}
turnAxis={0,1,0};turnPivot={0,1,0};OnReset();
""")

add("I01","키 한 번·켜기 끄기","PRESS 토글 배경색 표시 숨김", "3D", {"G":"bool showBox = true;",
"R":"showBox = true;", "K":"""
if (key==GLFW_KEY_SPACE && action==GLFW_PRESS) showBox = !showBox;
if (key==GLFW_KEY_C && action==GLFW_PRESS) background = Red;
""", "D":"if (showBox) Box(glm::mat4(1), Yellow);"},
    "Space로 표시 여부를 바꾸고 C로 배경색을 바꿉니다.", "GLFW_KEY_SPACE/C를 원하는 키로 바꾸고 showBox 대신 실행 상태를 토글할 수 있습니다.",
    "토글은 GLFW_PRESS에서만 처리합니다. GLFW_REPEAT까지 처리하면 누르고 있는 동안 계속 뒤집힙니다.", ("I02","A01"))
add("I02","키 누르는 동안 이동","방향키 연속 입력 속도", "3D", {"G":"glm::vec3 keyPosition(0);",
"R":"keyPosition=glm::vec3(0);", "U":"""
if (glfwGetKey(window,GLFW_KEY_LEFT)==GLFW_PRESS) keyPosition.x -= 2*dt;
if (glfwGetKey(window,GLFW_KEY_RIGHT)==GLFW_PRESS) keyPosition.x += 2*dt;
""", "D":"Box(T(keyPosition), Blue);"},
    "누르고 있는 동안 일정 속도로 이동합니다.", "2=초당 이동 거리. x를 z로 바꾸면 앞뒤 이동입니다.",
    "이 코드는 OnUpdate에 넣습니다. OnKey의 반복 입력 횟수에 의존하지 않습니다.", ("M01","P02"))
add("I03","클릭 위치에 생성","마우스 추가 개수 제한 사분면", "2D", {"G":"std::vector<glm::vec2> clickItems;",
"R":"clickItems.clear();", "M":"""
if (button==GLFW_MOUSE_BUTTON_LEFT && action==GLFW_PRESS && clickItems.size()<5)
    clickItems.push_back({mouseX,mouseY});
""", "D":"for (auto p:clickItems) Rect(p,{60,40},Yellow);"},
    "클릭한 논리 좌표에 사각형을 최대 5개 생성합니다.", "5=최대 개수, 60/40=너비/높이. 사분면은 x<400, y<300 비교로 나눕니다.",
    "mouseX/mouseY는 이미 800×600 논리 좌표입니다. 사분면별 제한은 각 영역에 속한 기존 항목 수를 세어 같은 조건에 사용합니다. 3D 바닥에 육면체를 만들 때는 I06을 사용합니다.", ("I05","I06","C06"))
add("I04","드래그","마우스 이동 오프셋 잡기 놓기", "2D", {"G":"""
glm::vec2 dragCenter(400,300), dragOffset(0);
bool dragging=false;
""", "R":"dragCenter={400,300}; dragOffset={0,0}; dragging=false;", "M":"""
if (button==GLFW_MOUSE_BUTTON_LEFT && action==GLFW_PRESS &&
    HitRect(mouseX,mouseY,dragCenter,{160,80})) {
    dragging=true;
    dragOffset=dragCenter-glm::vec2(mouseX,mouseY);
}
if (button==GLFW_MOUSE_BUTTON_LEFT && action==GLFW_RELEASE) dragging=false;
""", "C":"if (dragging) dragCenter=glm::vec2(mouseX,mouseY)+dragOffset;",
"D":"Rect(dragCenter,{160,80},Yellow);"},
    "누른 지점과 중심 사이 간격을 보존하며 사각형을 끕니다.", "160,80을 그리기와 HitRect 양쪽에서 함께 바꿉니다.",
    "C03과 조합할 때 별도의 snapCenter를 만들지 말고 dragCenter를 스냅 대상으로 사용합니다.", ("C01","C03","I05"), """
dragCenter={400,300}; mouseX=420; mouseY=310;
OnMouse(GLFW_MOUSE_BUTTON_LEFT,GLFW_PRESS);
mouseX=500; mouseY=350; OnCursor();
Require(glm::length(dragCenter-glm::vec2(480,340))<0.001f,"drag offset");
OnMouse(GLFW_MOUSE_BUTTON_LEFT,GLFW_RELEASE); mouseX=550; OnCursor();
Require(Near(dragCenter.x,480),"drag released");
""")
add("I05","겹친 도형 중 맨 위 선택","객체 배열 인덱스 클릭 선택", "2D", {"G":"""
std::vector<glm::vec2> selectItems={{300,300},{350,300},{400,300}};
int selected=-1;
""", "R":"selected=-1;", "M":"""
if (button==GLFW_MOUSE_BUTTON_LEFT && action==GLFW_PRESS) {
    selected=-1;
    for (int i=int(selectItems.size())-1; i>=0; --i)
        if (HitRect(mouseX,mouseY,selectItems[i],{100,80})) { selected=i; break; }
}
""", "D":"""
for (int i=0; i<int(selectItems.size()); ++i)
    Rect(selectItems[i],{100,80},i==selected ? Red : Blue);
"""}, "나중에 그려진 사각형부터 검사해 한 개를 선택합니다.", "selectItems의 좌표·개수와 100,80 크기를 바꿉니다.",
    "selected가 -1이면 선택 없음입니다. 배열에 접근하기 전에 0 이상인지 확인합니다.", ("I04","C01"), """
mouseX=375; mouseY=300; OnMouse(GLFW_MOUSE_BUTTON_LEFT,GLFW_PRESS);
Require(selected==2,"topmost selection");
mouseX=10; mouseY=10; OnMouse(GLFW_MOUSE_BUTTON_LEFT,GLFW_PRESS);
Require(selected==-1,"empty selection");
""")

add("I06","클릭한 3D 바닥에 육면체 생성","마우스 클릭 위치 박스 상자 생성 바닥 3D 좌표 변환 광선 피킹 MouseToFloor", "3D", {"G":"""
std::vector<glm::vec3> floorBoxes; // 생성한 박스들의 중심 위치
const float boxSize=0.5f;         // 박스 한 변의 길이
const float floorY=0.0f;          // 바닥 윗면 높이
const float floorHalfSize=5.0f;   // 바닥 한쪽 길이: 전체 10 x 10

bool MouseToFloor(glm::vec3& hit) {
    int width,height;
    glfwGetFramebufferSize(window,&width,&height);
    if (width<=0 || height<=0 || mouseX<0 || mouseX>=CanvasWidth ||
        mouseY<0 || mouseY>=CanvasHeight) return false;
    // 클릭 시점의 카메라와 창 크기로 투영을 갱신
    SetView(0,0,width,height,eye,target,cameraUp,perspective);
    float x=2.0f*mouseX/CanvasWidth-1.0f;
    float y=1.0f-2.0f*mouseY/CanvasHeight;
    glm::mat4 inverse=glm::inverse(viewProjection);
    glm::vec4 nearPoint=inverse*glm::vec4(x,y,-1,1);
    glm::vec4 farPoint=inverse*glm::vec4(x,y,1,1);
    if (std::abs(nearPoint.w)<0.000001f || std::abs(farPoint.w)<0.000001f)
        return false;
    nearPoint/=nearPoint.w;
    farPoint/=farPoint.w;
    glm::vec3 start(nearPoint),direction(farPoint-nearPoint);
    if (std::abs(direction.y)<0.000001f) return false; // 바닥과 평행
    float t=(floorY-start.y)/direction.y;
    if (t<0 || t>1) return false; // 보이는 깊이 범위 밖
    hit=start+direction*t;
    return std::abs(hit.x)<=floorHalfSize && std::abs(hit.z)<=floorHalfSize;
}
""", "R":"floorBoxes.clear();", "M":"""
if (button==GLFW_MOUSE_BUTTON_LEFT && action==GLFW_PRESS) {
    glm::vec3 hit;
    if (MouseToFloor(hit))
        floorBoxes.push_back({hit.x,floorY+boxSize/2.0f,hit.z});
}
""", "D":"""
// 바닥 윗면을 floorY에 맞춤
Box(T({0,floorY-0.1f,0})*S({floorHalfSize*2,0.2f,floorHalfSize*2}),
    {0.3f,0.35f,0.4f});
for (auto p:floorBoxes)
    Box(T(p)*S({boxSize,boxSize,boxSize}),Yellow);
"""}, "바닥 안을 왼쪽 클릭할 때마다 그 위치에 육면체 하나를 생성합니다. 화면의 클릭 방향과 수평 바닥이 만나는 3D 위치를 구해 저장합니다. 박스 아랫면은 floorY에 닿으며, F5로 모두 지웁니다. 원근·직교 투영 모두 지원합니다.",
    "boxSize=박스 한 변 길이, floorY=바닥 윗면 높이, floorHalfSize=바닥 중심에서 한쪽 끝까지의 길이입니다. boxSize와 floorHalfSize는 양수로 설정합니다. 두 크기 값은 장면 좌표 단위입니다. Yellow를 바꾸면 박스 색이 바뀝니다. 개수 제한은 없으며 필요하면 OnMouse 조건에 floorBoxes.size()<원하는개수를 추가합니다. 박스 위치는 vec3에 저장하고 OnDraw의 T(p)로 배치합니다.",
    "전역 칸의 MouseToFloor 함수까지 함께 복사합니다. mouseX/mouseY를 3D 좌표로 직접 사용하지 않습니다. 전체 창 하나의 base 카메라 기준이며 P05 화면 분할이나 별도 투영에는 해당 화면의 좌표·투영을 사용하도록 변환을 바꿔야 합니다. 바닥 밖·평행한 시선·카메라가 볼 수 없는 깊이는 무시합니다. 박스 중심의 X/Z가 바닥 안인지 검사하므로 가장자리 박스는 일부가 바깥으로 나올 수 있습니다. 같은 위치를 반복 클릭하면 겹칩니다. 물체 표면 선택·쌓기는 포함하지 않으며, 물체 위를 클릭해도 그 아래 방향의 바닥과 교차합니다.",
    ("I03","D06","D10","D11","P01"), """
glm::vec3 hit;
const glm::vec3 savedEye=eye,savedTarget=target,savedUp=cameraUp;
const bool savedPerspective=perspective;
auto pointMouseAt=[](glm::vec3 world) {
    glm::vec4 screen=viewProjection*glm::vec4(world,1);
    mouseX=(screen.x/screen.w+1)*CanvasWidth/2;
    mouseY=(1-screen.y/screen.w)*CanvasHeight/2;
};
for(bool projection:{true,false}) {
    perspective=projection;eye=savedEye;target=savedTarget;cameraUp=savedUp;
    OnReset();viewProjection=glm::mat4(1);
    mouseX=CanvasWidth/2;mouseY=CanvasHeight/2;
    Require(MouseToFloor(hit)&&glm::length(hit-glm::vec3(0,floorY,0))<0.001f,
        "floor center click refreshes camera before first frame");
    const glm::vec3 points[]={{1.5f,floorY,2},{-2,floorY,-1},{0,floorY,3},
        {1,floorY,-2},{-1,floorY,1},{2,floorY,0}};
    for(auto p:points) {
        pointMouseAt(p);
        OnMouse(GLFW_MOUSE_BUTTON_LEFT,GLFW_PRESS);
        Require(!floorBoxes.empty()&&glm::length(floorBoxes.back()-
            glm::vec3(p.x,floorY+boxSize/2,p.z))<0.001f,"floor click mapped to world position");
    }
    Require(floorBoxes.size()==6,"floor click has no five-box limit");
    OnMouse(GLFW_MOUSE_BUTTON_LEFT,GLFW_RELEASE);
    OnMouse(GLFW_MOUSE_BUTTON_LEFT,GLFW_REPEAT);
    OnMouse(GLFW_MOUSE_BUTTON_RIGHT,GLFW_PRESS);
    pointMouseAt({0,floorY,floorHalfSize+0.5f});
    Require(mouseX>=0&&mouseX<CanvasWidth&&mouseY>=0&&mouseY<CanvasHeight,
        "outside-floor test stays inside window");
    OnMouse(GLFW_MOUSE_BUTTON_LEFT,GLFW_PRESS);
    mouseX=-1;OnMouse(GLFW_MOUSE_BUTTON_LEFT,GLFW_PRESS);
    Require(floorBoxes.size()==6,"non-press and outside clicks ignored");
    floorProbeModels.clear();BeginFrame();OnDraw();
    Require(floorProbeModels.size()==7,"floor and all six boxes drawn");
    Require(Near(glm::vec3(floorProbeModels[0]*glm::vec4(0,0.5f,0,1)).y,floorY),
        "visible floor height matches picking plane");
    for(int i=0;i<6;++i) {
        glm::mat4 m=floorProbeModels[i+1];
        Require(glm::length(glm::vec3(m*glm::vec4(0,0,0,1))-floorBoxes[i])<0.001f,
            "stored center used when drawing box");
        for(float x:{-0.5f,0.5f}) for(float z:{-0.5f,0.5f})
            Require(Near(glm::vec3(m*glm::vec4(x,-0.5f,z,1)).y,floorY),"box rests on floor");
    }
    eye={0,5,9};target={0,5,0};mouseX=CanvasWidth/2;mouseY=CanvasHeight/2;
    Require(!MouseToFloor(hit),"parallel floor ray ignored");
    target={0,6,0};Require(!MouseToFloor(hit),"floor behind camera ignored");
}
eye=savedEye;target=savedTarget;cameraUp=savedUp;perspective=savedPerspective;
mouseX=CanvasWidth/2;mouseY=CanvasHeight/2;OnReset();
Require(floorBoxes.empty(),"reset clears floor boxes");
""")

add("M01","일정 속도 이동","직선 위치 속도 dt", "3D", {"G":"glm::vec3 movePosition(0), moveVelocity(1,0,0);",
"R":"movePosition={0,0,0};", "U":"movePosition += moveVelocity * dt;", "D":"Box(T(movePosition), Yellow);"},
    "위치에 초당 속도와 경과 시간을 곱해 더합니다.", "moveVelocity의 x,y,z=축별 초당 이동 거리.",
    "같은 물체라면 I02의 keyPosition과 별개로 만들지 않고 한 위치 변수에 갱신을 합칩니다.", ("I02","M07"))
add("M02","경계에서 반사","벽 튕기기 바운스", "2D", {"G":"""
glm::vec2 bounceCenter(400,300), bounceVelocity(150,100);
void ReflectAxis(float& p,float& v,float lo,float hi) {
    if(p>hi) { p=hi; v=-std::abs(v); }
    else if(p<lo) { p=lo; v=std::abs(v); }
}
""", "R":"bounceCenter={400,300}; bounceVelocity={150,100};", "U":"""
bounceCenter += bounceVelocity*dt;
ReflectAxis(bounceCenter.x,bounceVelocity.x,25,775);
ReflectAxis(bounceCenter.y,bounceVelocity.y,25,575);
""", "D":"Rect(bounceCenter,{50,50},Yellow);"},
    "중심을 경계에 보정하고 속도의 부호를 뒤집습니다.", "150/100=속도. 25/775는 화면 폭에서 사각형 반너비를 고려한 중심의 범위입니다.",
    "한 프레임의 경계 초과 거리는 버립니다. 크기를 바꾸면 중심의 최소·최대 범위도 바꿉니다.", ("M01","C04"), """
float p=790,v=100; ReflectAxis(p,v,25,775);
Require(Near(p,775)&&v<0,"right wall");
p=0; ReflectAxis(p,v,25,775);
Require(Near(p,25)&&v>0,"left wall");
""")
add("M03","지그재그 이동","왕복 경로 삼각파", "2D", {"G":"float zigTime=0;\nglm::vec2 zigCenter(50,150);",
"R":"zigTime=0; zigCenter={50,150};", "U":"""
zigTime += dt;
float x=std::fmod(zigTime*120,1400.0f), y=std::fmod(zigTime,2.0f);
zigCenter={x<=700 ? 50+x : 1450-x, 150+300*(y<=1 ? y : 2-y)};
""", "D":"Rect(zigCenter,{30,30},Yellow);"},
    "좌우 왕복과 위아래 왕복을 합쳐 지그재그로 이동합니다.", "120=가로 속도. y 식의 150=위쪽 시작, 300=상하 이동 폭.",
    "경로를 추적하려면 매 프레임 위치를 배열에 넣고 D02의 GL_LINE_STRIP으로 그립니다. 배열 길이는 제한합니다.", ("M04","D02"))
add("M04","나선 이동","소용돌이 반지름 증가", "2D", {"G":"float spiralTime=0;\nglm::vec2 spiralCenter(400,300);",
"R":"spiralTime=0; spiralCenter={400,300};", "U":"""
spiralTime += dt;
float radius=std::min(200.0f,spiralTime*20.0f);
spiralCenter={400+radius*std::cos(spiralTime*3),300+radius*std::sin(spiralTime*3)};
""", "D":"Circle(spiralCenter,12,Red);"},
    "반지름을 키우면서 중심 주위를 회전합니다.", "400/300=중심, 200=최대 반지름, 20=반지름 증가/초, 3=라디안/초.",
    "최대 반지름 이후에는 원운동입니다. 여기의 sin/cos는 라디안이고 R은 도 단위입니다.", ("T06","M07"))
add("M05","격자 이동과 장애물","행 열 칸 보드 경계", "2D", {"G":"int gridX=0, gridY=0;", "R":"gridX=gridY=0;", "K":"""
if(action==GLFW_PRESS) {
    int x=gridX,y=gridY;
    if(key==GLFW_KEY_LEFT) --x;
    if(key==GLFW_KEY_RIGHT) ++x;
    if(key==GLFW_KEY_UP) --y;
    if(key==GLFW_KEY_DOWN) ++y;
    bool blocked=(x==3 && y==2);
    if(x>=0 && x<8 && y>=0 && y<6 && !blocked) { gridX=x; gridY=y; }
}
""", "D":"""
Rect({350,250},{90,90},Red);
Rect({gridX*100.0f+50,gridY*100.0f+50},{80,80},Yellow);
"""}, "이동할 후보 칸을 검사한 뒤 실제 칸을 바꿉니다.", "8×6=칸 수, 100=칸 크기, (3,2)=장애물 칸.",
    "판정 좌표는 칸 번호, 그리기 좌표는 픽셀입니다. blocked를 장애물 목록 검사로 확장할 수 있습니다.", ("C02",), """
gridX=0;gridY=0; OnKey(GLFW_KEY_LEFT,GLFW_PRESS,0);
Require(gridX==0,"grid boundary");
gridX=2;gridY=2; OnKey(GLFW_KEY_RIGHT,GLFW_PRESS,0);
Require(gridX==2&&gridY==2,"blocked cell");
""")
add("M06","꺾인 경로와 일정 간격 배치","컨베이어 ㄱ자 경로 줄지어 이동", "3D", {"G":"""
float pathDistance=0;
glm::mat4 CornerPath(float distance) {
    float s=std::fmod(distance,10.0f); if(s<0) s+=10;
    if(s<6) return T({-3+s,0.4f,-2});
    return T({3,0.4f,-2+(s-6)})*R(-90,{0,1,0});
}
""", "R":"pathDistance=0;", "U":"pathDistance=std::fmod(pathDistance+1.5f*dt,10.0f);", "D":"""
for(int i=0;i<5;++i)
    Box(CornerPath(pathDistance+i*2.0f)*S({0.8f,0.8f,0.8f}),Yellow);
"""}, "길이 6과 4인 두 구간의 경로에 상자를 같은 간격으로 놓습니다.", "1.5=속도, 5=개수, 2=경로상의 간격. 전체 길이는 10입니다.",
    "끝에서는 시작으로 순간 복귀합니다. 한 물체만 필요하면 반복문 대신 CornerPath(pathDistance)를 사용합니다.", ("M01",), """
auto p=[](float s){return glm::vec3(CornerPath(s)*glm::vec4(0,0,0,1));};
Require(glm::length(p(6)-glm::vec3(3,0.4f,-2))<0.001f,"path corner");
Require(glm::length(p(10)-p(0))<0.001f,"path wrap");
""")
add("M07","이동 방향 바라보기","방향 각도 atan2", "2D", {"D":"""
glm::vec2 direction(1,1);
float degrees=glm::degrees(std::atan2(direction.y,direction.x));
Primitive(GL_TRIANGLES, {{25,0,0},{-20,-15,0},{-20,15,0}},
          T({400,300,0})*R(degrees,{0,0,1}),Yellow);
"""}, "기본적으로 +X를 보는 삼각형을 이동 방향으로 돌립니다.", "direction을 현재 속도 또는 현재 위치-이전 위치로 바꿉니다.",
    "방향 길이가 0이면 이전 각도를 유지합니다. 3D에서 +Z를 보는 모델의 Y축 각도는 degrees(atan2(dx,dz))입니다.", ("M01","M03"))

add("M08","기울어진 바닥에서 미끄러짐","방 기울기 경사 중력 낮은 쪽 실습21", "3D", {"G":"""
float slopeAngle=0,slideX=0,slideSpeed=0;
""", "R":"slopeAngle=slideX=slideSpeed=0;", "K":"""
if(action==GLFW_PRESS || action==GLFW_REPEAT) {
    if(key==GLFW_KEY_Q) slopeAngle=std::clamp(slopeAngle+5.0f,-45.0f,45.0f);
    if(key==GLFW_KEY_E) slopeAngle=std::clamp(slopeAngle-5.0f,-45.0f,45.0f);
}
""", "U":"""
// 양의 Z축 기울기에서는 오른쪽이 높으므로 왼쪽으로 미끄러짐
slideSpeed += -6.0f*std::sin(glm::radians(slopeAngle))*dt;
slideSpeed *= std::exp(-1.5f*dt); // 마찰에 의한 감쇠
slideX += slideSpeed*dt;
if(slideX < -2.7f) { slideX=-2.7f;slideSpeed=0; }
if(slideX >  2.7f) { slideX= 2.7f;slideSpeed=0; }
""", "D":"""
glm::mat4 slope=R(slopeAngle,{0,0,1});
Box(slope*T({0,-0.1f,0})*S({6,0.2f,8}),Blue);
Box(slope*T({slideX,0.3f,0})*S({0.6f,0.6f,0.6f}),Yellow);
"""}, "Q/E로 바닥의 Z축 기울기를 바꾸면 상자가 낮은 쪽으로 미끄러집니다. 바닥을 수평으로 되돌리면 속도가 서서히 줄어듭니다.",
    "slopeAngle=도 단위 기울기, slideX=회전 전 방 좌표의 가로 위치, slideSpeed=초당 속도입니다. 6=경사 방향 중력 크기, 1.5=마찰 감쇠, 2.7=방 반너비 3에서 상자 반너비 0.3을 뺀 경계입니다. 각도 제한·방 크기·상자 크기를 함께 맞춥니다.",
    "그리기에서 바닥과 상자에 같은 slope를 적용합니다. 양의 기울기 가속도는 -sin이므로 부호를 뒤집지 않습니다. 이 예제는 바닥에 붙어 X축으로 미끄러지는 단순 모형입니다. 공중 낙하는 A08/C04, 경계에서 반사는 C09로 별도 조합합니다.",
    ("D12","C09","T08","A08"), """
OnReset();OnUpdate(0.5f);
Require(Near(slideX,0)&&Near(slideSpeed,0),"flat slope remains still");
for(float a:{-20.0f,20.0f}) {
    OnReset();slopeAngle=a;
    glm::mat4 room=R(35,{0,1,0})*R(a,{0,0,1});
    float before=glm::vec3(room*glm::vec4(0,0.3f,0,1)).y;
    for(int i=0;i<20;++i) OnUpdate(0.01f);
    float after=glm::vec3(room*glm::vec4(slideX,0.3f,0,1)).y;
    Require(slideX*a<0&&after<before,"both slope signs move downhill in world space");
}
slideX=2.8f;slideSpeed=1;slopeAngle=0;OnUpdate(0.01f);
Require(Near(slideX,2.7f)&&Near(slideSpeed,0),"slope stops at boundary");
OnReset();
""")

add("C01","점이 사각형 안인지","클릭 히트 판정 HitRect", "2D", {"D":"""
bool inside=HitRect(mouseX,mouseY,{400,300},{200,100});
Rect({400,300},{200,100},inside ? Red : Blue);
"""}, "현재 마우스가 사각형 안에 있으면 색을 바꿉니다.", "판정과 Rect에 같은 중심·전체 크기를 넣습니다.",
    "그리기에서도 확인할 수 있지만 클릭 동작의 조건은 OnMouse에서 검사합니다.", ("I04","I05"), """
Require(HitRect(500,350,{400,300},{200,100}),"rectangle boundary included");
Require(!HitRect(501,350,{400,300},{200,100}),"rectangle outside");
""")
add("C02","사각형끼리 겹치는지","충돌 AABB 접촉", "2D", {"D":"""
glm::vec2 a(350,300), b(mouseX,mouseY);
bool hit=OverlapRect(a,{100,100},b,{60,60});
Rect(a,{100,100},Blue);
Rect(b,{60,60},hit ? Red : Yellow);
"""}, "축에 평행한 두 사각형의 겹침을 검사합니다.", "a,b=중심. 두 크기는 각각 전체 너비·높이입니다.",
    "닿는 경우도 true입니다. 회전한 사각형의 정확한 판정에는 그대로 사용할 수 없습니다.", ("C06","M05"), """
Require(OverlapRect({0,0},{100,100},{80,0},{60,60}),"touch counts");
Require(!OverlapRect({0,0},{100,100},{81,0},{60,60}),"separated");
""")
add("C03","가까우면 붙이기","스냅 거리 자석 슬롯", "2D", {"G":"glm::vec2 snapCenter(200,300);",
"R":"snapCenter={200,300};", "M":"""
if(button==GLFW_MOUSE_BUTTON_LEFT && action==GLFW_RELEASE) {
    snapCenter={mouseX,mouseY};
    if(glm::length(snapCenter-glm::vec2(600,300))<=45) snapCenter={600,300};
}
""", "D":"Rect({600,300},{100,100},Blue,false);\nRect(snapCenter,{80,80},Yellow);"},
    "마우스를 놓은 위치가 목표 중심에서 거리 45 이내면 목표점으로 맞춥니다.",
    "600/300=목표, 45=허용 거리. 드래그와 합칠 때 snapCenter 대신 실제 dragCenter를 검사합니다.",
    "이 예제 자체는 놓은 위치로 순간 이동합니다. 드래그 조합에서는 snapCenter={mouseX,mouseY} 줄을 빼고 실제 도형 중심을 사용합니다.", ("I04",), """
mouseX=570;mouseY=300;OnMouse(GLFW_MOUSE_BUTTON_LEFT,GLFW_RELEASE);
Require(Near(snapCenter.x,600),"near snap");
mouseX=500;OnMouse(GLFW_MOUSE_BUTTON_LEFT,GLFW_RELEASE);
Require(Near(snapCenter.x,500),"far no snap");
""")
add("C04","점프와 바닥 착지","중력 수직 속도 낙하", "3D", {"G":"float jumpY=0, jumpVelocity=0;",
"R":"jumpY=jumpVelocity=0;", "K":"""
if(key==GLFW_KEY_SPACE && action==GLFW_PRESS && jumpY<=0) jumpVelocity=6;
""", "U":"""
jumpVelocity -= 9.8f*dt;
jumpY += jumpVelocity*dt;
if(jumpY<=0) { jumpY=0; jumpVelocity=0; }
""", "D":"Ground();\nBox(T({0,jumpY+0.5f,0}),Yellow);"},
    "Space로 점프하고 중력으로 내려와 발밑 높이 0에 착지합니다.",
    "6=초기 상승 속도, 9.8=중력, jumpY=발밑 높이. 상자 중심은 반높이 0.5만큼 더합니다.",
    "공중에서 다시 점프하지 않도록 바닥 조건을 확인합니다. 로봇에는 Box의 위치 대신 모델 전체의 발밑 위치를 연결합니다.", ("C05","T05"), """
OnKey(GLFW_KEY_SPACE,GLFW_PRESS,0);OnUpdate(0.1f);
Require(jumpY>0&&jumpVelocity>0,"jump starts");
for(int i=0;i<200;++i) OnUpdate(0.01f);
Require(Near(jumpY,0)&&Near(jumpVelocity,0),"jump lands");
""")
add("C05","장애물 윗면 착지","발밑 이전 높이 낙하 통과", "3D", {"G":"""
glm::vec3 feet(0,3,0);
float fallVelocity=0;
bool LandOnTop(float previousY,glm::vec3& foot,float& vy,
               glm::vec3 center,glm::vec3 size,float radius) {
    float top=center.y+size.y/2;
    bool over=std::abs(foot.x-center.x)<=size.x/2+radius &&
              std::abs(foot.z-center.z)<=size.z/2+radius;
    if(vy<=0 && over && previousY>=top && foot.y<=top) {
        foot.y=top; vy=0; return true;
    }
    return false;
}
""", "R":"feet={0,3,0}; fallVelocity=0;", "U":"""
float previousY=feet.y;
fallVelocity -= 9.8f*dt; feet.y += fallVelocity*dt;
LandOnTop(previousY,feet,fallVelocity,{0,0.5f,0},{2,1,2},0.5f);
if(feet.y<0) { feet.y=0; fallVelocity=0; }
""", "D":"Box(T({0,0.5f,0})*S({2,1,2}),Blue);\nBox(T(feet+glm::vec3(0,0.5f,0)),Yellow);"},
    "위에서 내려오며 윗면을 통과한 경우에만 착지합니다.",
    "center/size=장애물 중심과 전체 크기. radius=발의 가로·깊이 반범위.",
    "여러 장애물이 겹치면 이번 프레임에 통과한 윗면 중 가장 높은 곳을 고릅니다. 옆면 충돌과 수평 이동은 별도 기능입니다.", ("C04","C02"), """
glm::vec3 p(0,0.8f,0);float v=-2;
Require(LandOnTop(1.2f,p,v,{0,0.5f,0},{2,1,2},0.5f)&&Near(p.y,1),"land on top");
p={0,0.8f,0};v=2;
Require(!LandOnTop(0.7f,p,v,{0,0.5f,0},{2,1,2},0.5f),"no upward landing");
p={4,0.8f,0};v=-2;
Require(!LandOnTop(1.2f,p,v,{0,0.5f,0},{2,1,2},0.5f),"no outside landing");
""")
add("C06","닿은 도형 지우기","삭제 지우개 충돌", "2D", {"G":"std::vector<glm::vec2> eraseItems={{200,300},{400,300},{600,300}};",
"R":"eraseItems={{200,300},{400,300},{600,300}};", "M":"""
if(button==GLFW_MOUSE_BUTTON_LEFT && action==GLFW_PRESS)
    eraseItems.erase(std::remove_if(eraseItems.begin(),eraseItems.end(),
        [](glm::vec2 p){return OverlapRect(p,{80,80},{mouseX,mouseY},{40,40});}),
        eraseItems.end());
""", "D":"""
for(auto p:eraseItems) Rect(p,{80,80},Blue);
Rect({mouseX,mouseY},{40,40},Yellow,false);
"""}, "클릭할 때 지우개 영역과 겹친 항목을 배열에서 제거합니다.",
    "80,80=도형 크기. 40,40=지우개 크기. 계속 지우려면 버튼 상태를 확인하며 갱신 함수에서 처리합니다.",
    "반복 중 인덱스를 증가시키며 erase하면 항목을 건너뛸 수 있습니다. 여기서는 remove_if와 erase를 함께 씁니다.", ("C02","I03"), """
mouseX=400;mouseY=300;OnMouse(GLFW_MOUSE_BUTTON_LEFT,GLFW_PRESS);
Require(eraseItems.size()==2,"erase collided only");
""")
add("C07","도형을 조각으로 분할","쪼개기 파편 퍼지기", "2D", {"G":"""
struct Fragment { glm::vec2 center,velocity; };
std::vector<Fragment> fragments;
bool splitDone=false;
""", "R":"fragments.clear(); splitDone=false;", "M":"""
if(button==GLFW_MOUSE_BUTTON_LEFT && action==GLFW_PRESS && !splitDone &&
   HitRect(mouseX,mouseY,{400,300},{100,100})) {
    splitDone=true;
    for(int x:{-1,1}) for(int y:{-1,1})
        fragments.push_back({{400+x*25.0f,300+y*25.0f},{x*80.0f,y*80.0f}});
}
""", "U":"for(auto& piece:fragments) piece.center += piece.velocity*dt;", "D":"""
if(!splitDone) Rect({400,300},{100,100},Yellow);
for(auto piece:fragments) Rect(piece.center,{50,50},Blue);
"""}, "사각형을 클릭하면 네 조각으로 나눠 대각선 방향으로 이동시킵니다.",
    "100=원본 한 변, 50=조각 한 변, 25=각 조각의 중심 오프셋, 80=축별 속도.",
    "원본을 숨기는 splitDone과 조각 배열을 함께 관리합니다. 크기를 바꾸면 조각 크기와 오프셋도 맞춥니다.", ("C06","M01"), """
mouseX=400;mouseY=300;OnMouse(GLFW_MOUSE_BUTTON_LEFT,GLFW_PRESS);
Require(splitDone&&fragments.size()==4,"four pieces");
OnMouse(GLFW_MOUSE_BUTTON_LEFT,GLFW_PRESS);
Require(fragments.size()==4,"split only once");
""")

add("C08","방 밖으로 나가지 않게 이동 제한","방 벽 경계 막기 제한 크기 중심 반크기 WASD", "3D", {"G":"""
glm::vec3 roomPosition(0,0.5f,0),roomHalfSize(0.5f);
const glm::vec3 moveLow(-3,0,-4),moveHigh(3,6,4);
bool KeepInRoom(glm::vec3& center,glm::vec3 halfSize,glm::vec3 low,glm::vec3 high) {
    glm::vec3 lo=low+halfSize,hi=high-halfSize;
    for(int a=0;a<3;++a)
        if(halfSize[a]<0 || low[a]>=high[a] || lo[a]>hi[a]) return false;
    center=glm::clamp(center,lo,hi);
    return true;
}
""", "R":"roomPosition={0,0.5f,0};", "U":"""
glm::vec3 input(0);
if(glfwGetKey(window,GLFW_KEY_A)==GLFW_PRESS) input.x-=1;
if(glfwGetKey(window,GLFW_KEY_D)==GLFW_PRESS) input.x+=1;
if(glfwGetKey(window,GLFW_KEY_W)==GLFW_PRESS) input.z-=1;
if(glfwGetKey(window,GLFW_KEY_S)==GLFW_PRESS) input.z+=1;
if(glm::length(input)>0) roomPosition+=glm::normalize(input)*2.0f*dt;
KeepInRoom(roomPosition,roomHalfSize,moveLow,moveHigh);
""", "D":"Box(T(roomPosition)*S(roomHalfSize*2.0f),Yellow);"},
    "WASD로 상자를 움직이고 상자의 표면이 방 밖으로 나가지 않게 중심 위치를 보정합니다. 대각선 이동도 같은 속도입니다.",
    "moveLow/moveHigh는 방 안쪽 좌표입니다. roomHalfSize는 상자 전체 크기의 절반, roomPosition은 중심입니다. 2.0은 이동 속도입니다. D12의 방 경계와 같은 값을 사용합니다.",
    "KeepInRoom은 위치만 제한하며 속도는 바꾸지 않습니다. 로봇 좌표가 발밑이면 중심=발밑+{0,반높이,0}로 바꿔 검사한 뒤 되돌립니다. 통과 가능한 문·열린 바닥의 경계는 따로 제외해야 합니다. 물체가 방에 들어갈 수 없으면 false를 반환하고 위치를 바꾸지 않습니다.",
    ("D12","C09","C10","C04","C05"), """
glm::vec3 p(99,-5,-99);
Require(KeepInRoom(p,{0.5f,1,0.25f},moveLow,moveHigh),"box fits room");
Require(glm::length(p-glm::vec3(2.5f,1,-3.75f))<0.001f,"room accounts for half size");
p={0,2,0};KeepInRoom(p,{0.5f,1,0.25f},moveLow,moveHigh);
Require(glm::length(p-glm::vec3(0,2,0))<0.001f,"interior position unchanged");
Require(!KeepInRoom(p,{4,1,1},moveLow,moveHigh)&&Near(p.y,2),"oversized box rejected");
Require(!KeepInRoom(p,{-1,1,1},moveLow,moveHigh),"negative half size rejected");
""")

add("C09","3D 방의 벽·바닥·천장에서 반사","공 구 바운스 방 튕기기 반지름 실습21", "3D", {"G":"""
glm::vec3 ballPosition(0,2,0),ballVelocity(2,1.5f,1);
const float ballRadius=0.3f;
const glm::vec3 bounceLow(-3,0,-4),bounceHigh(3,6,4);
bool BounceInRoom(glm::vec3& center,glm::vec3& velocity,float radius,
                  glm::vec3 low,glm::vec3 high) {
    if(radius<=0) return false;
    glm::vec3 lo=low+glm::vec3(radius),hi=high-glm::vec3(radius);
    for(int a=0;a<3;++a) if(low[a]>=high[a] || lo[a]>=hi[a]) return false;
    for(int a=0;a<3;++a) {
        if(center[a]<lo[a]) { center[a]=lo[a];velocity[a]= std::abs(velocity[a]); }
        if(center[a]>hi[a]) { center[a]=hi[a];velocity[a]=-std::abs(velocity[a]); }
    }
    return true;
}
""", "R":"ballPosition={0,2,0};ballVelocity={2,1.5f,1};", "U":"""
ballPosition+=ballVelocity*dt;
BounceInRoom(ballPosition,ballVelocity,ballRadius,bounceLow,bounceHigh);
""", "D":"Sphere(T(ballPosition)*S(glm::vec3(ballRadius)),Yellow);"},
    "공이 방의 여섯 경계에 닿으면 해당 축의 속도만 반대로 바꿉니다. 반지름을 고려해 공 표면이 벽에 닿게 합니다.",
    "ballVelocity의 x/y/z가 각 축의 초당 이동량입니다. ballRadius는 그리기와 충돌에 함께 사용됩니다. 여러 공은 위치·속도를 struct와 vector에 저장하고 각 공에 BounceInRoom을 호출합니다.",
    "초과 이동 거리는 버리고 중심을 경계에 보정하는 학습용 반사입니다. 매우 빠른 공의 정확한 충돌 시각 계산은 별도입니다. 바닥을 연 뒤에는 이 함수의 Y축 하한 반사를 그대로 호출하면 낙하가 막힙니다. A08처럼 바닥 지지를 해제하고, 필요한 벽만 별도로 검사합니다. 숨긴 앞면도 충돌 경계에는 포함됩니다.",
    ("D12","C08","M08","A08"), """
for(int a=0;a<3;++a) for(float sign:{-1.0f,1.0f}) {
    glm::vec3 p(0,2,0),v(2,3,4);
    p[a]=sign<0 ? bounceLow[a]-1 : bounceHigh[a]+1;
    float speed=glm::length(v);
    Require(BounceInRoom(p,v,ballRadius,bounceLow,bounceHigh),"sphere fits room");
    Require(Near(p[a],sign<0?bounceLow[a]+ballRadius:bounceHigh[a]-ballRadius)&&v[a]*sign<0,
        "all six room boundaries reflect inward");
    Require(Near(glm::length(v),speed),"wall reflection preserves speed magnitude");
}
OnReset();for(int i=0;i<400;++i) {
    OnUpdate(0.05f);
    for(int a=0;a<3;++a) Require(ballPosition[a]>=bounceLow[a]+ballRadius-0.001f&&
        ballPosition[a]<=bounceHigh[a]-ballRadius+0.001f,"sphere stays within room");
}
Require(!BounceInRoom(ballPosition,ballVelocity,9,bounceLow,bounceHigh),"oversized sphere rejected");
OnReset();
""")

add("C10","3D 장애물에 막히면 방향 바꾸기","방 로봇 걷기 장애물 옆면 충돌 AABB 실습23", "3D", {"G":"""
struct RoomObstacle { glm::vec3 center,size; };
std::vector<RoomObstacle> roomObstacles={{{0,0.5f,0},{1,1,1}},{{1.5f,0.75f,-2},{1,1.5f,1}}};
glm::vec3 walkerCenter(-2,0.5f,0),walkerVelocity(1.4f,0,0);
const glm::vec3 walkerSize(0.6f,1,0.6f);
bool BoxOverlap3D(glm::vec3 a,glm::vec3 sizeA,glm::vec3 b,glm::vec3 sizeB) {
    glm::vec3 gap=glm::abs(a-b),limit=(sizeA+sizeB)/2.0f;
    return gap.x<limit.x && gap.y<limit.y && gap.z<limit.z;
}
""", "R":"walkerCenter={-2,0.5f,0};walkerVelocity={1.4f,0,0};", "U":"""
// 기본 속도에서 장애물을 건너뛰지 않도록 작은 시간 간격으로 검사
int steps=std::max(1,int(std::ceil(dt/0.005f)));
float stepTime=dt/steps;
for(int i=0;i<steps;++i) {
    glm::vec3 next=walkerCenter+walkerVelocity*stepTime;
    bool blocked=std::abs(next.x)>3-walkerSize.x/2 || std::abs(next.z)>4-walkerSize.z/2;
    for(auto obstacle:roomObstacles)
        if(BoxOverlap3D(next,walkerSize,obstacle.center,obstacle.size)) blocked=true;
    if(blocked) walkerVelocity=-walkerVelocity;
    else walkerCenter=next;
}
""", "D":"""
for(auto obstacle:roomObstacles)
    Box(T(obstacle.center)*S(obstacle.size),Blue);
Box(T(walkerCenter)*S(walkerSize),Yellow);
"""}, "상자가 자동으로 걷다가 방의 X/Z 경계나 장애물 옆면에 막히면 반대 방향으로 움직입니다. 서로 닿기만 한 면은 겹침으로 보지 않아 장애물 윗면 위에서 수평 이동할 수 있습니다.",
    "roomObstacles의 center/size는 장애물 중심과 전체 크기입니다. walkerCenter도 몸 중심이며 walkerSize는 전체 크기입니다. walkerVelocity는 수평 속도입니다. 3/4는 방의 X/Z 반범위이며 D12와 맞춥니다.",
    "단순 축 정렬 상자 충돌입니다. 별도로 회전한 장애물의 정밀 메시 충돌은 아닙니다. 0.005초 간격 검사는 기본 크기·속도를 위한 것이며 매우 빠른 이동이나 작은 물체에는 간격을 줄이거나 연속 충돌 검사를 사용합니다. 충돌 때 멈추려면 속도 반전 대신 속도를 0으로 만듭니다. 점프·착지는 C04/C05의 발밑 높이와 연결하고 중력을 중복 갱신하지 않습니다.",
    ("D12","C08","C04","C05","M07"), """
Require(BoxOverlap3D({0,0.5f,0},walkerSize,{0,0.5f,0},{1,1,1}),"overlapping boxes blocked");
Require(!BoxOverlap3D({0,1.5f,0},walkerSize,{0,0.5f,0},{1,1,1}),"standing on top is not side collision");
Require(!BoxOverlap3D({2,0.5f,0},walkerSize,{0,0.5f,0},{1,1,1}),"separated boxes pass");
OnReset();bool reversed=false;
for(int i=0;i<600;++i) {
    OnUpdate(0.01f);if(walkerVelocity.x<0) reversed=true;
    Require(std::abs(walkerCenter.x)<=3-walkerSize.x/2+0.001f,"walker remains inside room");
    for(auto obstacle:roomObstacles)
        Require(!BoxOverlap3D(walkerCenter,walkerSize,obstacle.center,obstacle.size),"walker never enters obstacle");
}
Require(reversed,"obstacle reverses walking direction");OnReset();
""")

add("A01","일정 시간마다 실행","타이머 주기 누적", "3D", {"G":"float timerElapsed=0;\nbool timerBlue=false;",
"R":"timerElapsed=0; timerBlue=false;", "U":"""
timerElapsed += dt;
while(timerElapsed>=1.0f) { timerElapsed-=1.0f; timerBlue=!timerBlue; }
""", "D":"Box(glm::mat4(1),timerBlue ? Blue : Yellow);"},
    "1초마다 상태를 바꿉니다.", "1.0=주기(초). while 안의 코드를 도형 생성·색 변경 등의 동작으로 바꿉니다.",
    "주기는 0보다 커야 합니다. 0으로 초기화하는 대신 주기를 빼서 남은 시간을 보존합니다.", ("I01",), """
OnUpdate(2.5f);Require(Near(timerElapsed,0.5f)&&!timerBlue,"timer remainder");
OnUpdate(0.5f);Require(Near(timerElapsed,0)&&timerBlue,"timer boundary");
""")
add("A02","부드러운 왕복 각도","시소 흔들기 팔 다리 사인 진동", "3D", {"G":"float waveTime=0, waveAngle=0;",
"R":"waveTime=waveAngle=0;", "U":"waveTime+=dt;\nwaveAngle=25.0f*std::sin(waveTime*2.0f);",
"D":"Box(T({0,1.5f,0})*R(waveAngle,{0,0,1})*S({6,0.2f,0.8f}),Red);"},
    "각도를 -25도부터 +25도까지 부드럽게 왕복시킵니다.",
    "25=최대 기울기, 2=사인 함수의 각속도(라디안/초). 주기는 약 2π/2초입니다.",
    "시소 손잡이까지 움직이려면 T04의 angle 대신 waveAngle을 사용합니다. 반대쪽 팔다리는 각도 앞에 -를 붙입니다.", ("T04","T05"))
add("A03","정해진 시간 동안 보간","목표 이동 자리 교환 progress mix", "3D", {"G":"float moveProgress=0;",
"R":"moveProgress=0;", "U":"moveProgress=std::min(1.0f,moveProgress+dt/2.0f);", "D":"""
glm::vec3 p=glm::mix(glm::vec3(-3,0,0),glm::vec3(3,0,0),moveProgress);
Box(T(p),Yellow);
"""}, "시작점에서 끝점까지 2초 동안 이동합니다.",
    "2.0=소요 시간(양수), 두 vec3=시작과 끝. 자리 교환은 반대 방향 mix를 같은 progress로 계산합니다.",
    "키로 시작할 때는 시작 위치를 그 순간 한 번 저장하고 progress를 0으로 만듭니다. 매 프레임 시작점을 현재 위치로 덮어쓰지 않습니다.", ("A04","C02"), """
OnUpdate(1);Require(Near(moveProgress,0.5f),"half duration");
OnUpdate(5);Require(Near(moveProgress,1),"progress capped");
""")
add("A04","색·숫자 보간","색 변화 크기 중간값 glm mix", "3D", {"D":"""
float u=0.5f;
glm::vec3 color=glm::mix(Red,Blue,u);
float size=glm::mix(0.5f,1.5f,u);
Box(S({size,size,size}),color);
"""}, "0~1 비율로 두 색이나 두 크기 사이 값을 계산합니다.",
    "u=0은 시작값, 1은 끝값. 크기만 필요하면 size 계산만, 색만 필요하면 color 계산만 사용합니다.",
    "이 코드는 고정 중간값입니다. 시간 변화는 A03의 progress 또는 0.5+0.5*sin(time*speed)를 u로 연결합니다.", ("A02","A03"))
add("A05","순서대로 움직이기","순차 애니메이션 면 펼치기", "3D", {"G":"float sequenceTime=0;",
"R":"sequenceTime=0;", "U":"sequenceTime+=dt;", "D":"""
for(int i=0;i<4;++i) {
    float u=std::clamp((sequenceTime-i*0.5f)/1.0f,0.0f,1.0f);
    Box(T({-3+i*2.0f,u*2,0})*S({0.5f,0.5f,0.5f}),Yellow);
}
"""}, "각 부품의 시작 시간을 다르게 두고 같은 동작을 순서대로 실행합니다.",
    "4=부품 수, 0.5=시작 간격(초), 1.0=각 동작의 시간. u*2를 R(u*90,축) 등에 연결하면 순차 회전입니다.",
    "시작 간격을 0으로 두면 동시에 움직입니다. 각 부품의 회전 중심은 T04 또는 T05로 정합니다.", ("T04","A03"))
add("A06","윗부분 고정한 문 회전·축소","문 열기 경첩 높이 줄이기", "3D", {"G":"""
glm::mat4 DoorModel(float u) {
    u=std::clamp(u,0.0f,1.0f);
    float height=4*(1-u);
    return T({0,4,0})*R(-90*u,{1,0,0})*T({0,-height/2,0})*S({3,height,0.1f});
}
""", "D":"Box(DoorModel(0.5f),Yellow);"},
    "문 윗부분을 고정하고 회전하며 높이를 줄입니다.",
    "u=닫힘 0~열림 1, 4=닫힌 높이와 경첩 높이, 3=너비, 90=최대 각도.",
    "고정된 0.5 대신 A03의 progress를 넣으면 시간에 따라 열립니다. 크기가 줄어든 뒤 중심을 -height/2로 옮기는 순서를 유지합니다.", ("T04","A03"), """
for(float u:{0.0f,0.5f,1.0f})
    Require(glm::length(glm::vec3(DoorModel(u)*glm::vec4(0,0.5f,0,1))-glm::vec3(0,4,0))<0.001f,"door hinge");
""")
add("A07","정점 보간으로 모양 변경","사각뿔 육면체 변형 morph", "3D", {"D":"""
float u=0.5f;
glm::vec3 b[]={{-1,0,1},{1,0,1},{1,0,-1},{-1,0,-1}}, top[4];
for(int i=0;i<4;++i) top[i]=glm::mix(glm::vec3(0,2,0),b[i]+glm::vec3(0,2,0),u);
for(int i=0;i<4;++i) {
    int j=(i+1)%4;
    Primitive(GL_TRIANGLE_FAN,{b[i],b[j],top[j],top[i]},glm::mat4(1),Blue);
}
Primitive(GL_TRIANGLE_FAN,{top[0],top[1],top[2],top[3]},glm::mat4(1),Yellow);
Primitive(GL_TRIANGLE_FAN,{b[3],b[2],b[1],b[0]},glm::mat4(1),Red);
"""}, "위쪽 네 정점을 한 꼭짓점에서 사각형으로 벌립니다.",
    "u=0이면 사각뿔, 1이면 육면체. 2=높이. A03의 progress를 u에 연결하면 애니메이션입니다.",
    "정점 수와 연결 순서를 유지하면서 좌표만 보간합니다.", ("D08","A03"))

add("A08","바닥 열기와 아래로 낙하","방 바닥 문 경첩 열림 중력 떨어짐 실습21", "3D", {"G":"""
bool hatchOpen=false,hatchFalling=false;
float hatchAngle=0,fallCenterY=0.3f,fallSpeed=0;
glm::mat4 HatchModel(float degrees) {
    return T({-3,0,0})*R(-degrees,{0,0,1})*T({3,-0.1f,0})*S({6,0.2f,8});
}
""", "R":"hatchOpen=hatchFalling=false;hatchAngle=fallSpeed=0;fallCenterY=0.3f;", "K":"""
if(key==GLFW_KEY_O && action==GLFW_PRESS) hatchOpen=!hatchOpen;
""", "U":"""
float targetAngle=hatchOpen ? 90.0f : 0.0f;
hatchAngle+=std::clamp(targetAngle-hatchAngle,-90.0f*dt,90.0f*dt);
if(hatchAngle>5.0f) hatchFalling=true;
if(hatchFalling) {
    fallSpeed-=9.8f*dt;
    fallCenterY+=fallSpeed*dt;
}
""", "D":"""
Box(HatchModel(hatchAngle),Blue);
Box(T({0,fallCenterY,0})*S({0.6f,0.6f,0.6f}),Yellow);
"""}, "O로 바닥을 왼쪽 모서리 중심으로 아래로 열고 닫습니다. 5도를 넘겨 열면 상자가 아래로 떨어집니다. 떨어지기 시작한 상자는 바닥을 닫아도 계속 낙하하며 F5로 복원합니다.",
    "HatchModel의 {-3,0,0}=왼쪽 위 경첩, {3,-0.1,0}=경첩에서 바닥 중심까지의 거리, {6,0.2,8}=바닥 크기입니다. 90은 최대 열림 각도와 초당 회전 속도입니다. 9.8은 중력, 0.3은 상자 반높이입니다.",
    "열림 각도 5도를 기준으로 바닥 지지를 끄는 학습용 모형입니다. 움직이는 바닥 면과의 정밀 접촉·충돌은 계산하지 않습니다. 방에 넣을 때 D12의 floor=false로 고정 바닥을 생략하고 room * HatchModel(...)을 그립니다. 아래로 떨어진 물체에 C08/C09의 바닥 제한을 적용하면 낙하가 막힙니다.",
    ("D12","C09","M08","A06","C04"), """
for(float a:{0.0f,45.0f,90.0f})
    Require(glm::length(glm::vec3(HatchModel(a)*glm::vec4(-0.5f,0.5f,0,1))-
        glm::vec3(-3,0,0))<0.001f,"hatch upper left hinge stays fixed");
OnReset();OnUpdate(1);
Require(Near(fallCenterY,0.3f)&&!hatchFalling,"closed hatch supports box");
OnKey(GLFW_KEY_O,GLFW_REPEAT,0);Require(!hatchOpen,"hatch ignores repeated key");
OnKey(GLFW_KEY_O,GLFW_PRESS,0);OnUpdate(0.2f);
Require(hatchAngle>5&&hatchFalling&&fallCenterY<0.3f,"open hatch triggers fall");
float previous=fallCenterY;OnKey(GLFW_KEY_O,GLFW_PRESS,0);OnUpdate(1);
Require(Near(hatchAngle,0)&&fallCenterY<previous,"closing does not teleport fallen box");
OnReset();Require(Near(fallCenterY,0.3f)&&!hatchFalling,"hatch reset restores box");
""")

add("P01","카메라 위치·투영 전환","시점 원근 직교 perspective", "3D", {"R":"""
eye={6,5,9}; target={0,0,0}; cameraUp={0,1,0}; perspective=true;
""", "K":"if(key==GLFW_KEY_P && action==GLFW_PRESS) perspective=!perspective;",
"D":"Box(glm::mat4(1),Yellow);"},
    "base.cpp의 자동 카메라 설정을 이용하고 P로 원근/직교 투영을 전환합니다.",
    "eye=카메라 위치, target=바라보는 점, cameraUp=화면 위 방향. true=원근, false=직교입니다.",
    "OnDraw에서 SetView를 다시 호출하면 그 설정이 최종 적용됩니다. eye와 target은 서로 달라야 합니다.", ("P02","P05"))
add("P02","카메라 앞뒤·옆 이동","시점 이동 WASD 카메라 기준", "3D", {"R":"eye={6,5,9}; target={0,0,0}; cameraUp={0,1,0};",
"U":"""
glm::vec3 forward=glm::normalize(target-eye);
glm::vec3 right=glm::normalize(glm::cross(forward,cameraUp));
glm::vec3 step(0);
if(glfwGetKey(window,GLFW_KEY_W)==GLFW_PRESS) step+=forward*3.0f*dt;
if(glfwGetKey(window,GLFW_KEY_S)==GLFW_PRESS) step-=forward*3.0f*dt;
if(glfwGetKey(window,GLFW_KEY_D)==GLFW_PRESS) step+=right*3.0f*dt;
if(glfwGetKey(window,GLFW_KEY_A)==GLFW_PRESS) step-=right*3.0f*dt;
eye+=step; target+=step;
""", "D":"Ground();\nBox(glm::mat4(1),Yellow);"},
    "카메라 시선 방향으로 이동합니다. eye와 target에 같은 이동량을 더합니다.",
    "3=초당 속도. 지면 위 이동만 원하면 forward.y=0으로 만든 뒤 다시 정규화합니다.",
    "시선과 cameraUp이 평행하면 right를 계산할 수 없습니다. 두 이동 키를 동시에 누르면 이 예제에서는 대각선이 더 빠릅니다.", ("I02","P03"))
add("P03","카메라 제자리 회전","시선 자전 좌우 보기", "3D", {"R":"eye={0,2,8}; target={0,1,0}; cameraUp={0,1,0};", "K":"""
if(key==GLFW_KEY_Y && action==GLFW_PRESS) {
    float degrees=(mods&GLFW_MOD_SHIFT) ? -5.0f : 5.0f;
    target=eye+glm::vec3(R(degrees,{0,1,0})*glm::vec4(target-eye,0));
}
""", "D":"Ground();\nBox(glm::mat4(1),Yellow);"},
    "카메라 위치를 고정하고 시선 방향만 돌립니다.", "Y=5도 회전, Shift+Y=반대. R의 축으로 회전 방향을 정합니다.",
    "vec4의 마지막 값 0은 방향 벡터입니다. 위아래 회전은 시선과 up의 평행 상태를 피하도록 각도를 제한합니다.", ("P04","P01"))
add("P04","카메라 공전","장면 주위 시점 궤도", "3D", {"R":"eye={6,5,9}; target={0,0,0}; cameraUp={0,1,0};", "K":"""
if(key==GLFW_KEY_O && action==GLFW_PRESS) {
    float degrees=(mods&GLFW_MOD_SHIFT) ? -5.0f : 5.0f;
    eye=target+glm::vec3(R(degrees,{0,1,0})*glm::vec4(eye-target,0));
}
""", "D":"Ground();\nBox(glm::mat4(1),Yellow);"},
    "target을 고정하고 eye를 그 주위로 돌립니다.", "O=5도, Shift+O=반대. target은 공전 중심이면서 바라보는 점입니다.",
    "물체의 공전과 카메라의 공전은 바꾸는 값이 다릅니다. 물체를 공전시키려면 T06을 사용합니다.", ("P03","T06"), """
glm::vec3 center=target;float distance=glm::length(eye-target);
OnKey(GLFW_KEY_O,GLFW_PRESS,0);
Require(glm::length(target-center)<0.001f&&Near(glm::length(eye-target),distance),"camera orbit radius");
""")
add("P05","화면 분할","뷰포트 다중 시점 정면도 평면도", "3D", {"G":"""
void DrawViewportScene() {
    Box(T({-1,0,0}),Yellow);
    Box(T({1,0,0})*S({0.5f,2,0.5f}),Blue);
}
""", "D":"""
int w=framebufferWidth,h=framebufferHeight,left=w/2;
SetView(0,0,left,h,{6,5,9},{0,0,0},{0,1,0},true);
DrawViewportScene();
SetView(left,0,w-left,h,{0,12,0},{0,0,0},{0,0,-1},false);
DrawViewportScene();
"""}, "같은 장면을 왼쪽 원근도와 오른쪽 평면도로 그립니다.",
    "SetView의 앞 네 값은 x,y,너비,높이(픽셀)입니다. DrawViewportScene의 내용만 원하는 장면으로 바꿉니다.",
    "OnUpdate는 프레임당 한 번만 호출합니다. 화면마다 glClear를 호출하지 않습니다. 위에서 볼 때 up은 시선과 평행하지 않게 {0,0,-1}을 씁니다.", ("P01",))

assert len({f["id"] for f in FEATURES}) == len(FEATURES)
for feature in FEATURES:
    assert feature["id"][0] in GROUPS
    assert all(slot in SLOTS for slot in feature["blocks"])
    assert feature["blocks"].get("D"), feature["id"]
    assert all(any(other["id"]==id for other in FEATURES) for id in feature["related"])

# 인자 설명은 사용 참고서에만 표시한다. 기능모음집의 코드 칸은 변경하지 않는다.
ARGUMENTS = {
    "DrawRoom": ("DrawRoom(parent, low, high, front=false, ceiling=true, floor=true)", [
        ("1 · parent", "방 전체의 이동·회전 행렬입니다. glm::mat4(1)은 기본 위치, room 행렬을 내부 물체에도 적용하면 함께 움직입니다."),
        ("2 · low", "방 안쪽 X/Y/Z 최소 좌표입니다. {-3,0,-4}이면 왼쪽 -3, 바닥 0, 뒤쪽 -4입니다."),
        ("3 · high", "방 안쪽 X/Y/Z 최대 좌표입니다. {3,6,4}이면 오른쪽 3, 천장 6, 앞쪽 4입니다. 전체 크기는 high-low입니다."),
        ("4 · front", "앞면(+Z) 표시 여부입니다. 생략하면 false입니다. 충돌 경계를 해제하는 인자는 아닙니다."),
        ("5 · ceiling", "천장 표시 여부입니다. 기본 true입니다."),
        ("6 · floor", "바닥 표시 여부입니다. 기본 true입니다. A08의 여는 바닥을 붙일 때 false로 설정합니다.")]),
    "KeepInRoom": ("KeepInRoom(center, halfSize, low, high)", [
        ("1 · center (&)", "물체 중심 위치입니다. 방 안으로 보정한 결과가 원본에 저장됩니다. 설정이 유효하면 true, 크기가 들어갈 수 없으면 false입니다."),
        ("2 · halfSize", "물체의 가로·높이·깊이 절반입니다. 전체 크기가 {1,2,1}이면 {0.5,1,0.5}입니다. 음수는 허용하지 않습니다."),
        ("3 · low / 4 · high", "방 안쪽의 최소·최대 좌표입니다. 중심은 low+halfSize부터 high-halfSize 사이로 제한됩니다.")]),
    "BounceInRoom": ("BounceInRoom(center, velocity, radius, low, high)", [
        ("1 · center (&)", "공의 중심 위치입니다. 경계를 벗어나면 반지름을 고려해 원본 위치를 보정합니다."),
        ("2 · velocity (&)", "공의 X/Y/Z 초당 속도입니다. 충돌한 축의 부호를 방 안쪽으로 바꿉니다."),
        ("3 · radius", "공의 반지름입니다. 양수이며 방의 모든 변 길이가 지름보다 커야 합니다. 유효하면 true, 잘못된 설정은 false를 반환합니다."),
        ("4 · low / 5 · high", "방 안쪽 최소·최대 좌표입니다. 공을 그리는 Sphere 배율에도 같은 radius를 사용합니다.")]),
    "BoxOverlap3D": ("BoxOverlap3D(a, sizeA, b, sizeB)", [
        ("1 · a / 3 · b", "두 물체의 중심 위치입니다. 발밑 좌표라면 반높이를 더해서 중심으로 바꿉니다."),
        ("2 · sizeA / 4 · sizeB", "두 물체의 전체 가로·높이·깊이입니다. 각 성분은 양수로 설정합니다. 세 축 모두 실제로 겹치면 true이며 면이 닿기만 하면 false입니다.")]),
    "HatchModel": ("HatchModel(degrees)", [
        ("1 · degrees", "바닥의 열림 각도입니다. 예제는 0=닫힘, 90=수직으로 열린 상태입니다. {-3,0,0} 모서리가 고정되는 모델 행렬을 반환합니다.")]),
    "glm::clamp": ("glm::clamp(value, low, high)", [
        ("1 · value", "제한할 벡터입니다. C08에서는 물체 중심입니다."),
        ("2 · low / 3 · high", "각 성분별 하한·상한 벡터입니다. 각 하한은 상한 이하여야 합니다. 보정된 벡터를 반환하므로 원본 변수에 대입합니다.")]),
    "glm::abs": ("glm::abs(value)", [
        ("1 · value", "각 성분의 절댓값을 구할 벡터입니다. 충돌 검사에서는 두 중심의 좌표 차이를 축별 거리로 바꿉니다.")]),
    "glm::radians": ("glm::radians(degrees)", [
        ("1 · degrees", "라디안으로 바꿀 도 단위 각도입니다. std::sin에 도 단위를 전달하기 전에 사용합니다. R 함수에는 도 단위를 그대로 전달합니다.")]),
    "std::exp": ("std::exp(value)", [
        ("1 · value", "자연상수 e의 지수입니다. exp(-1.5*dt)는 프레임 시간에 따른 속도 감쇠 비율이며 1.5를 키우면 더 빨리 느려집니다.")]),
    "std::ceil": ("std::ceil(value)", [
        ("1 · value", "올림할 숫자입니다. dt/0.005를 올림해서 한 번의 충돌 검사 시간 간격이 0.005초 이하가 되게 합니다.")]),
    "std::max": ("std::max(a, b)", [
        ("1 · a / 2 · b", "비교할 같은 자료형의 두 값입니다. 더 큰 값을 반환합니다. max(1,steps)는 반복 횟수를 최소 1로 만듭니다.")]),
    "MouseToFloor": ("MouseToFloor(hit)", [
        ("1 · hit (&)", "결과를 받을 vec3 변수입니다. 성공 시 클릭 방향과 바닥이 만나는 {x,floorY,z}가 저장됩니다. true를 반환했을 때만 사용합니다. 창 밖·바닥 밖·평행한 시선 등에서는 false를 반환합니다.")]),
    "glm::inverse": ("glm::inverse(matrix)", [
        ("1 · matrix", "역행렬을 구할 행렬입니다. I06은 viewProjection의 역행렬로 화면 방향을 3D 공간으로 되돌립니다. 변환된 vec4를 w로 나누어 실제 좌표를 구합니다.")]),
    "glfwGetFramebufferSize": ("glfwGetFramebufferSize(window, width, height)", [
        ("1 · window", "base에서 만든 GLFW 창 포인터입니다."),
        ("2 · width (&변수)", "렌더링 영역의 가로 픽셀 수를 받을 int 변수의 주소입니다. 실제 코드는 &width를 전달합니다."),
        ("3 · height (&변수)", "렌더링 영역의 세로 픽셀 수를 받을 int 변수의 주소입니다. 최소화 등으로 크기가 0이면 생성하지 않습니다. 마우스 논리 좌표 크기와 구분합니다.")]),
    "CheckerFloor": ("CheckerFloor(count=10, spacing=1, floorY=0, colorA={0.2,0.2,0.2}, colorB={0.8,0.8,0.8})", [
        ("1 · count", "한 변의 전체 칸 수, 1 이상의 정수. 3이면 3×3=9개 타일입니다. 홀수·짝수 모두 가능하며 생략 시 10. D10의 한쪽 칸 수 halfCount와 구분합니다."),
        ("2 · spacing", "한 칸의 길이, 0보다 큰 장면 좌표 단위. 전체 한 변은 count*spacing이며 원점을 중심으로 배치합니다. 생략 시 1."),
        ("3 · floorY", "모든 타일 면의 Y 좌표. 0이면 높이 0의 바닥, 음수는 아래쪽입니다. 생략 시 0."),
        ("4 · colorA", "첫 번째 색 {r,g,b}, 각 값 0~1. x+z가 짝수인 칸을 채웁니다. 생략 시 진회색 {0.2,0.2,0.2}."),
        ("5 · colorB", "두 번째 색 {r,g,b}, 각 값 0~1. x+z가 홀수인 칸을 채웁니다. 생략 시 연회색 {0.8,0.8,0.8}. 마지막 인자부터 생략할 수 있습니다.")]),
    "GridFloor": ("GridFloor(halfCount=5, spacing=1, floorY=0, floorColor={0.35,0.35,0.35}, lineColor={0.7,0.7,0.7})", [
        ("1 · halfCount", "중심에서 한쪽 방향의 칸 수, 1 이상의 정수. 5이면 가로·세로 각각 10칸입니다. 늘리면 칸 수와 전체 크기가 함께 커집니다. 생략 시 5."),
        ("2 · spacing", "한 칸의 길이, 0보다 큰 장면 좌표 단위. 늘리면 칸 수는 같고 바닥이 커집니다. 전체 한 변은 2*halfCount*spacing입니다. 생략 시 1."),
        ("3 · floorY", "바닥 윗면의 Y 좌표. 0이면 발밑 높이 0인 물체가 닿는 바닥입니다. 음수도 가능하며 생략 시 0."),
        ("4 · floorColor", "바닥의 {r,g,b} 색, 각 값 0~1. 생략 시 {0.35,0.35,0.35}. 바닥 면의 명암은 Box에서 적용됩니다."),
        ("5 · lineColor", "격자선의 {r,g,b} 색, 각 값 0~1. 생략 시 {0.7,0.7,0.7}. 마지막 인자부터 생략할 수 있어 GridFloor()도 기본 설정으로 동작합니다.")]),
    "Primitive": ("Primitive(mode, vertices, model, color)", [
        ("1 · mode", "정점을 연결하는 방식. GL_POINTS=점, GL_LINES=두 점씩 선, GL_LINE_STRIP=연속선, GL_LINE_LOOP=닫힌 선, GL_TRIANGLES=세 점씩 삼각형, GL_TRIANGLE_FAN=첫 점 기준 부채꼴."),
        ("2 · vertices", "{x,y,z} 정점들의 목록. 개수와 순서를 바꾸면 모양이 바뀝니다. 2D 예제의 z는 0이며 x/y는 논리 화면 좌표입니다."),
        ("3 · model", "정점에 적용할 위치·회전·크기 행렬 하나. glm::mat4(1)은 모델 변형 없음입니다. 카메라·투영은 base가 별도로 적용합니다."),
        ("4 · color", "{r,g,b} 또는 Red/Blue 등의 색. 각 성분은 0~1, 예를 들어 {1,0,0}은 빨강입니다.")]),
    "glPointSize": ("glPointSize(size)", [
        ("1 · size", "점의 픽셀 크기. 12.0f면 12픽셀 크기를 요청합니다. 0보다 커야 하며 지원 크기 범위는 환경에 따라 다릅니다.")]),
    "Rect": ("Rect(center, size, color, filled=true)", [
        ("1 · center", "{x,y} 중심 좌표. 왼쪽 위 모서리 좌표가 아닙니다. 2D에서는 +x가 오른쪽, +y가 아래입니다."),
        ("2 · size", "{width,height} 전체 너비·높이. {200,100}이면 중심에서 좌우 100, 위아래 50만큼 뻗습니다."),
        ("3 · color", "{r,g,b} 색, 각 값 0~1. Yellow 같은 준비된 색 변수도 가능합니다."),
        ("4 · filled", "true=면 채우기, false=테두리. 생략하면 true입니다.")]),
    "Circle": ("Circle(center, radius, color, filled=true)", [
        ("1 · center", "{x,y} 원의 중심. 2D 논리 화면 좌표입니다."),
        ("2 · radius", "반지름. 60이면 지름은 120입니다. 양수로 지정합니다."),
        ("3 · color", "{r,g,b} 색, 각 값 0~1."),
        ("4 · filled", "true=채운 원, false=원 테두리. 생략하면 true입니다.")]),
    "Box": ("Box(model, color)", [
        ("1 · model", "위치·회전·크기를 합친 행렬 하나. T(...) * R(...) * S(...) 전체가 첫 번째 인자입니다. 기본 상자는 원점 중심, 각 변 길이 1입니다."),
        ("2 · color", "{r,g,b} 또는 Yellow/Red 등의 색. 각 값 0~1. 면별 명암은 Box 함수에서 자동으로 다르게 적용합니다.")]),
    "Sphere": ("Sphere(model, color)", [
        ("1 · model", "위치·회전·크기 행렬. 기본 구는 원점 중심 반지름 1이므로 S({2,2,2})는 반지름 2입니다."),
        ("2 · color", "{r,g,b} 또는 Green 등의 색. 각 값 0~1입니다.")]),
    "T": ("T(p)", [
        ("1 · p", "{x,y,z} 이동 벡터 하나. 세 숫자는 X/Y/Z축 이동량입니다. 단독 모델에 적용하면 중심 위치가 되고, 부모 행렬 뒤에서는 부모 좌표계에 대한 이동입니다.")]),
    "R": ("R(degrees, axis)", [
        ("1 · degrees", "회전각, 단위는 도. 15=15도, -15=반대 회전. 내부에서 라디안으로 변환하므로 직접 radians를 적용하지 않습니다."),
        ("2 · axis", "{x,y,z} 회전축 벡터 하나. {1,0,0}=X, {0,1,0}=Y, {0,0,1}=Z. {0,0,0}은 사용하지 않습니다.")]),
    "S": ("S(size)", [
        ("1 · size", "{x,y,z} 축별 크기 배율 하나. Box의 기본 한 변이 1이므로 {6,0.2,0.8}은 전체 길이·두께·폭이 됩니다. Sphere에서는 축별 반지름입니다. 0이면 해당 축이 납작해집니다.")]),
    "Ground": ("Ground()", [
        ("없음", "base에 정해진 바닥을 그립니다. 크기·위치를 인자로 받지 않으므로 바닥을 직접 조절하려면 D06의 Box를 사용합니다.")]),
    "HitRect": ("HitRect(x, y, center, size)", [
        ("1 · x", "검사할 점의 X 좌표. 마우스 판정에서는 mouseX입니다."),
        ("2 · y", "검사할 점의 Y 좌표. 마우스 판정에서는 mouseY입니다."),
        ("3 · center", "검사 대상 사각형의 중심 {x,y}. 검사할 점의 좌표와 구분합니다."),
        ("4 · size", "대상 사각형의 전체 너비·높이 {width,height}. 점이 내부 또는 경계에 있으면 true를 반환합니다.")]),
    "OverlapRect": ("OverlapRect(a, sizeA, b, sizeB)", [
        ("1 · a", "첫 번째 사각형의 중심 {x,y}."),
        ("2 · sizeA", "첫 번째 사각형의 전체 너비·높이."),
        ("3 · b", "두 번째 사각형의 중심 {x,y}."),
        ("4 · sizeB", "두 번째 사각형의 전체 너비·높이. 두 사각형이 닿거나 겹치면 true를 반환합니다.")]),
    "ReflectAxis": ("ReflectAxis(p, v, lo, hi)", [
        ("1 · p (&)", "검사할 축의 현재 중심 좌표. 경계를 넘으면 원본 변수가 lo 또는 hi로 보정됩니다."),
        ("2 · v (&)", "그 축의 속도, 좌표 단위/초. 원본 속도의 부호가 공간 안쪽을 향하도록 바뀝니다."),
        ("3 · lo", "중심의 최소 허용 좌표. 예: 너비 50 사각형의 왼쪽 경계는 25."),
        ("4 · hi", "중심의 최대 허용 좌표. 예: 화면 폭 800에서 반너비 25를 빼면 775. lo보다 작으면 안 됩니다.")]),
    "CornerPath": ("CornerPath(distance)", [
        ("1 · distance", "경로를 따라 진행한 누적 거리. 각도나 x좌표가 아닙니다. 현재 경로 길이는 10이며 양수·음수 거리 모두 경로 안으로 순환시킵니다. 위치·방향 행렬을 반환합니다.")]),
    "LandOnTop": ("LandOnTop(previousY, foot, vy, center, size, radius)", [
        ("1 · previousY", "이 프레임의 낙하 이동을 적용하기 전 발밑 Y 좌표. 물체 중심 높이가 아닙니다."),
        ("2 · foot (&)", "이동 후 발밑 위치 {x,y,z}. 착지하면 원본 foot.y가 장애물 윗면 높이로 바뀝니다."),
        ("3 · vy (&)", "수직 속도, 좌표 단위/초. 양수는 상승, 음수는 낙하이며 착지하면 원본 값이 0이 됩니다."),
        ("4 · center", "장애물 상자의 중심 {x,y,z}. 윗면 좌표가 아닙니다."),
        ("5 · size", "장애물 상자의 전체 크기 {가로,높이,깊이}. 윗면은 center.y + size.y/2입니다."),
        ("6 · radius", "발의 X/Z축 반크기. 예제의 한 변 1인 상자는 0.5입니다. 원형 거리 판정이 아닌 축별 겹침 범위이며 착지 성공 시 true를 반환합니다.")]),
    "DoorModel": ("DoorModel(u)", [
        ("1 · u", "열림 진행률. 0=닫힘, 0.5=중간, 1=완전히 열림. 함수가 0~1로 제한하고 문 행렬을 반환합니다. 시간이나 각도 자체가 아닙니다.")]),
    "SetView": ("SetView(x, y, width, height, from, at, up, usePerspective)", [
        ("1 · x", "뷰포트 왼쪽 아래 모서리의 X 좌표, 프레임버퍼 픽셀 단위."),
        ("2 · y", "뷰포트 왼쪽 아래 모서리의 Y 좌표, 프레임버퍼 픽셀 단위. +Y가 위이며 2D 마우스 논리 좌표와 방향이 다릅니다."),
        ("3 · width", "뷰포트 너비, 픽셀 단위. 0보다 커야 합니다."),
        ("4 · height", "뷰포트 높이, 픽셀 단위. 0보다 커야 하며 width/height로 종횡비를 계산합니다."),
        ("5 · from", "카메라 위치 {x,y,z}. 전체 화면 기본 설정에서는 eye입니다."),
        ("6 · at", "바라보는 점 {x,y,z}. 방향 벡터가 아닌 위치이며 from과 같으면 안 됩니다. 기본 설정에서는 target입니다."),
        ("7 · up", "화면 위쪽을 정할 방향 벡터. 보통 {0,1,0}, 바로 위에서 내려다볼 때는 {0,0,-1}. 0벡터 또는 시선과 평행한 벡터는 피합니다."),
        ("8 · usePerspective", "true=원근 투영, false=직교 투영. 현재 base의 원근 시야각은 45도이며 이 인자는 시야각 숫자가 아닙니다.")]),
    "DrawViewportScene": ("DrawViewportScene()", [
        ("없음", "같은 장면을 다시 그립니다. 호출 전에 SetView로 정한 시점이 적용되며, 다른 장면을 만들려면 함수 본문의 Box들을 바꿉니다.")]),
    "OnKey": ("OnKey(key, action, mods)", [
        ("1 · key", "입력이 발생한 키 코드. GLFW_KEY_SPACE, GLFW_KEY_LEFT 등과 비교합니다."),
        ("2 · action", "GLFW_PRESS=처음 누름, GLFW_REPEAT=누른 상태의 반복 입력, GLFW_RELEASE=놓음. 토글은 PRESS에서만 처리합니다."),
        ("3 · mods", "Shift/Ctrl/Alt 등의 수정키 비트 집합. mods & GLFW_MOD_SHIFT가 0이 아니면 Shift가 함께 눌린 상태입니다. 기본 콜백이 세 값을 전달합니다.")]),
    "OnMouse": ("OnMouse(button, action)", [
        ("1 · button", "이벤트가 발생한 마우스 버튼. GLFW_MOUSE_BUTTON_LEFT=왼쪽 버튼."),
        ("2 · action", "GLFW_PRESS=누름, GLFW_RELEASE=놓음. 위치는 인자가 아니라 base가 갱신하는 mouseX/mouseY에서 읽습니다.")]),
    "OnCursor": ("OnCursor()", [
        ("없음", "마우스가 이동하면 base가 호출합니다. 현재 위치는 전역 mouseX/mouseY를 읽으며 800×600 논리 좌표입니다.")]),
    "OnUpdate": ("OnUpdate(dt)", [
        ("1 · dt", "직전 프레임 이후 흐른 시간, 초 단위. 속도×dt=이번 프레임 이동량입니다. base가 호출하므로 직접 상수로 넘겨 반복 호출하지 않습니다.")]),
    "glfwGetKey": ("glfwGetKey(window, key)", [
        ("1 · window", "키 상태를 확인할 GLFW 창 포인터. base가 만든 전역 window를 전달합니다."),
        ("2 · key", "검사할 키 코드. GLFW_KEY_W 등. GLFW_PRESS가 반환되면 현재 눌려 있습니다.")]),
    "glm::mix": ("glm::mix(start, end, u)", [
        ("1 · start", "시작 값. 위치·색 같은 벡터 또는 크기 같은 숫자입니다."),
        ("2 · end", "끝 값. start와 호환되는 형태·단위여야 합니다."),
        ("3 · u", "단위 없는 보간 비율. 0=시작, 1=끝, 0.5=중간. 함수 자체가 0~1로 제한하지 않으므로 진행률 범위는 별도로 관리합니다.")]),
    "glm::length": ("glm::length(v)", [
        ("1 · v", "길이를 구할 벡터. 두 점 사이 거리에는 현재 위치-목표 위치를 넣습니다. 위치 자체를 넣으면 원점에서 그 위치까지의 거리입니다.")]),
    "glm::normalize": ("glm::normalize(v)", [
        ("1 · v", "방향을 유지하고 길이를 1로 만들 벡터. 길이가 0인 벡터는 넣지 않습니다.")]),
    "glm::cross": ("glm::cross(a, b)", [
        ("1 · a", "첫 번째 3D 벡터. 카메라 예제에서는 forward입니다."),
        ("2 · b", "두 번째 3D 벡터. 카메라 예제에서는 cameraUp입니다. 순서를 바꾸면 결과 방향이 반대이며 평행하면 결과가 0이 됩니다.")]),
    "glm::degrees": ("glm::degrees(radians)", [
        ("1 · radians", "도 단위로 바꿀 라디안 값. atan2의 결과를 R에 전달하기 전에 사용합니다.")]),
    "std::atan2": ("std::atan2(y, x)", [
        ("1 · y", "방향 벡터의 Y 성분. 일반 2D 방향 계산에서는 direction.y를 먼저 넣습니다."),
        ("2 · x", "방향 벡터의 X 성분. 반환 각도는 라디안입니다. 3D의 +Z 정면 모델에 대한 Y축 회전에서는 atan2(dx,dz)를 사용합니다.")]),
    "std::sin": ("std::sin(radians)", [
        ("1 · radians", "라디안 단위 각도. time*speed라면 time은 초, speed는 라디안/초입니다. 결과 범위는 -1~1입니다.")]),
    "std::cos": ("std::cos(radians)", [
        ("1 · radians", "라디안 단위 각도. 나선 예제에서 radius*cos(angle)은 중심으로부터의 X축 오프셋입니다. 결과 범위는 -1~1입니다.")]),
    "std::min": ("std::min(a, b)", [
        ("1 · a", "비교할 첫 번째 값."),
        ("2 · b", "비교할 두 번째 값. 더 작은 값을 반환합니다. 예: min(1.0f,progress)는 진행률의 상한을 1로 제한하지만 하한은 제한하지 않습니다.")]),
    "std::clamp": ("std::clamp(value, low, high)", [
        ("1 · value", "범위를 제한할 값."),
        ("2 · low", "포함되는 최솟값."),
        ("3 · high", "포함되는 최댓값. low보다 작으면 안 됩니다. value와 같은 단위·호환되는 자료형을 사용합니다.")]),
    "std::fmod": ("std::fmod(value, period)", [
        ("1 · value", "누적 시간 또는 거리."),
        ("2 · period", "반복 주기 또는 전체 경로 길이. 0이면 안 되며 예제에서는 양수입니다. value가 음수이면 결과도 음수일 수 있어 CornerPath는 음수 결과를 보정합니다.")]),
    "std::abs": ("std::abs(value)", [
        ("1 · value", "절댓값을 구할 숫자. 음수 속도를 넣어도 크기는 양수로 나오므로 앞에 -를 붙여 방향을 지정할 수 있습니다.")]),
    "glm::vec2": ("glm::vec2(x, y) / glm::vec2(s)", [
        ("1 · x / 2 · y", "두 숫자를 전달하면 각 성분으로 저장합니다. 위치이면 좌표, 속도이면 축별 이동량/초입니다."),
        ("1 · s (숫자 하나)", "두 성분을 모두 같은 값으로 만듭니다. glm::vec2(0)은 {0,0}입니다.")]),
    "glm::vec3": ("glm::vec3(x, y, z) / glm::vec3(s) / glm::vec3(v4)", [
        ("1 · x / 2 · y / 3 · z", "세 숫자를 전달하면 각 성분으로 저장합니다. 위치/방향에서는 X/Y/Z, 색으로 쓰면 R/G/B입니다."),
        ("1 · s (숫자 하나)", "세 성분을 모두 같은 값으로 만듭니다. glm::vec3(0)은 {0,0,0}입니다."),
        ("1 · v4 (vec4 하나)", "vec4의 x,y,z 성분만 가져오고 w는 제외합니다. 이 변환 자체가 w로 나누지는 않습니다.")]),
    "glm::vec4": ("glm::vec4(v3, w) / glm::vec4(x, y, z, w)", [
        ("1 · v3", "xyz에 넣을 3D 위치 또는 방향 벡터."),
        ("2 · w (v3, w 형태)", "1=위치라 이동 행렬의 영향을 받음, 0=방향이라 이동의 영향을 받지 않음. 카메라 시선 회전에는 0을 씁니다."),
        ("1 · x / 2 · y / 3 · z / 4 · w", "숫자 네 개를 전달하면 각 성분을 직접 설정합니다. I06의 화면 좌표에서는 z=-1이 가까운 면, z=1이 먼 면이며 w=1로 시작합니다.")]),
    "glm::mat4": ("glm::mat4(diagonal)", [
        ("1 · diagonal", "대각 성분 값. 1이면 대각만 1이고 나머지는 0인 단위행렬이 되어 모델 변형을 하지 않습니다. 모든 성분을 1로 채우는 의미가 아닙니다.")]),
    "push_back": ("items.push_back(value)", [
        ("1 · value", "배열 끝에 추가할 항목 하나. clickItems에는 중심 {x,y}, floorBoxes에는 3D 중심 {x,y,z}, fragments에는 {중심,속도}를 넣습니다.")]),
    "std::remove_if": ("std::remove_if(first, last, predicate)", [
        ("1 · first", "검사할 범위의 시작 반복자. eraseItems.begin()입니다."),
        ("2 · last", "검사 범위 마지막 다음 반복자. eraseItems.end()이며 이 위치는 검사하지 않습니다."),
        ("3 · predicate", "항목 하나를 받아 제거 대상이면 true를 반환하는 함수/람다. 예제는 겹침 판정을 사용합니다. remove_if만으로 배열 길이는 줄지 않습니다.")]),
    "erase": ("items.erase(first, last)", [
        ("1 · first", "실제로 지울 범위의 첫 반복자. 예제에서는 remove_if가 반환한 새 끝입니다."),
        ("2 · last", "지울 범위의 마지막 다음 반복자. items.end()까지 지우면 배열 길이도 줄어듭니다.")]),
}

# 같은 함수라도 각 기능에서 바로 읽을 수 있게 관련 인자를 해당 기능 아래에 표시한다.
FEATURE_ARGUMENTS = {
    "D01":"glPointSize Primitive glm::mat4", "D02":"Primitive glm::mat4",
    "D03":"Primitive glm::mat4", "D04":"Rect", "D05":"Circle", "D06":"Box T S",
    "D07":"Sphere T S", "D08":"Primitive glm::vec3 glm::mat4", "D09":"Primitive glm::mat4",
    "D10":"GridFloor Box T S Primitive glm::mat4",
    "D11":"CheckerFloor Primitive glm::mat4",
    "D12":"DrawRoom Primitive Box T glm::mat4 glm::vec3",
    "T01":"Box T", "T02":"Box T R", "T03":"Box T S", "T04":"Box T R S glm::vec3",
    "T05":"Box T R S", "T06":"OnUpdate Sphere T R S", "T07":"OnUpdate Sphere T R S",
    "T08":"OnKey OnUpdate Ground Box T R S glm::vec3",
    "I01":"OnKey Box glm::mat4", "I02":"OnUpdate glfwGetKey Box T glm::vec3",
    "I03":"OnMouse push_back Rect", "I04":"OnMouse OnCursor HitRect glm::vec2 Rect",
    "I05":"OnMouse HitRect Rect",
    "I06":"MouseToFloor OnMouse glfwGetFramebufferSize SetView glm::inverse glm::vec3 glm::vec4 std::abs push_back Box T S",
    "M01":"OnUpdate Box T glm::vec3", "M02":"OnUpdate ReflectAxis std::abs glm::vec2 Rect",
    "M03":"OnUpdate std::fmod glm::vec2 Rect",
    "M04":"OnUpdate std::min std::sin std::cos glm::vec2 Circle",
    "M05":"OnKey Rect", "M06":"OnUpdate CornerPath std::fmod Box T R S",
    "M07":"glm::vec2 std::atan2 glm::degrees Primitive T R",
    "M08":"OnKey OnUpdate std::clamp std::sin glm::radians std::exp Box T R S",
    "C01":"HitRect Rect", "C02":"glm::vec2 OverlapRect Rect",
    "C03":"OnMouse glm::length glm::vec2 Rect", "C04":"OnKey OnUpdate Ground Box T",
    "C05":"OnUpdate LandOnTop glm::vec3 std::abs Box T S",
    "C06":"OnMouse std::remove_if erase OverlapRect Rect",
    "C07":"OnMouse OnUpdate HitRect push_back Rect",
    "C08":"KeepInRoom OnUpdate glfwGetKey glm::clamp glm::length glm::normalize Box T S",
    "C09":"BounceInRoom OnUpdate std::abs Sphere T S",
    "C10":"BoxOverlap3D OnUpdate glm::abs std::max std::ceil std::abs Box T S",
    "A01":"OnUpdate Box glm::mat4", "A02":"OnUpdate std::sin Box T R S",
    "A03":"OnUpdate std::min glm::mix glm::vec3 Box T", "A04":"glm::mix Box S",
    "A05":"OnUpdate std::clamp Box T S", "A06":"DoorModel std::clamp Box T R S",
    "A07":"glm::mix glm::vec3 Primitive glm::mat4",
    "A08":"OnKey OnUpdate HatchModel std::clamp Box T R S",
    "P01":"OnKey Box glm::mat4", "P02":"OnUpdate glm::normalize glm::cross glfwGetKey glm::vec3 Ground Box glm::mat4",
    "P03":"OnKey R glm::vec3 glm::vec4 Ground Box glm::mat4",
    "P04":"OnKey R glm::vec3 glm::vec4 Ground Box glm::mat4",
    "P05":"SetView DrawViewportScene Box T S",
}
assert set(FEATURE_ARGUMENTS)=={f["id"] for f in FEATURES}
assert set(ARGUMENTS)=={key for names in FEATURE_ARGUMENTS.values() for key in names.split()}
for feature in FEATURES:
    names=FEATURE_ARGUMENTS[feature["id"]].split()
    assert len(names)==len(set(names))
    feature["arguments"]=[ARGUMENTS[name] for name in names]
