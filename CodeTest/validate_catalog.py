"""기능모음집의 실제 코드 칸을 대조하고 base에 연결해 빌드·숨김 실행 검사한다."""
from pathlib import Path
import re
import shutil
import subprocess
from feature_catalog import FEATURES, SLOTS

ROOT=Path(__file__).resolve().parent
WORK=ROOT/".validation"/"catalog"
WORK.mkdir(parents=True,exist_ok=True)
(WORK/"examples").mkdir(exist_ok=True)
base=(ROOT/"base.cpp").read_text(encoding="utf-8-sig")
document=(ROOT/"Markdown"/"기능모음집.md").read_text(encoding="utf-8-sig")
fence=chr(96)*3

def function(source,name):
    match=re.search(r"^void "+name+r"\([^\n]*\)\s*\{",source,re.M)
    assert match,name
    opening=source.index("{",match.start())
    depth,end=1,opening+1
    while depth:
        depth+=(source[end]=="{")-(source[end]=="}")
        end+=1
    return source[match.start():end]

signatures={"R":"OnReset()", "K":"OnKey(int key,int action,int mods)",
            "M":"OnMouse(int button,int action)", "C":"OnCursor()",
            "U":"OnUpdate(float dt)", "D":"OnDraw()"}
definitions=[]
checks=[]
for feature in FEATURES:
    id=feature["id"]
    name="Feature"+id
    section=document.split('## '+id+' ',1)[1].split('\n## ',1)[0]
    actual=re.findall(fence+r"cpp\n(.*?)\n"+fence,section,re.S)
    expected=[feature["blocks"][slot] for slot in SLOTS if slot in feature["blocks"]]
    assert actual==expected,id
    chunks=["namespace "+name+" {"]
    if id=="D10":
        chunks += ["""
std::vector<glm::vec3> gridProbeVertices;
int gridProbeBoxes=0;
glm::mat4 gridProbeModel(1);
glm::vec3 gridProbeFloorColor(0),gridProbeLineColor(0);
void Box(glm::mat4 model,glm::vec3 color) {
    ++gridProbeBoxes;gridProbeModel=model;gridProbeFloorColor=color;
    ::Box(model,color);
}
void Primitive(GLenum mode,const std::vector<glm::vec3>& vertices,
               glm::mat4 model,glm::vec3 color) {
    Require(mode==GL_LINES,"grid uses independent lines");
    gridProbeVertices.insert(gridProbeVertices.end(),vertices.begin(),vertices.end());
    gridProbeLineColor=color;
    ::Primitive(mode,vertices,model,color);
}
"""]
    if id=="D11":
        chunks += ["""
std::vector<std::vector<glm::vec3>> checkerProbeTiles;
std::vector<glm::vec3> checkerProbeColors;
void Primitive(GLenum mode,const std::vector<glm::vec3>& vertices,
               glm::mat4 model,glm::vec3 color) {
    Require(mode==GL_TRIANGLE_FAN,"checker uses filled tiles");
    std::vector<glm::vec3> tile;
    for(auto p:vertices) tile.push_back(glm::vec3(model*glm::vec4(p,1)));
    checkerProbeTiles.push_back(tile);checkerProbeColors.push_back(color);
    ::Primitive(mode,vertices,model,color);
}
"""]
    if id=="T08":
        chunks += ["""
std::vector<glm::mat4> rotationProbeModels;
void Box(glm::mat4 model,glm::vec3 color) {
    rotationProbeModels.push_back(model);
    ::Box(model,color);
}
"""]
    if id=="I06":
        chunks += ["""
std::vector<glm::mat4> floorProbeModels;
void Box(glm::mat4 model,glm::vec3 color) {
    floorProbeModels.push_back(model);
    ::Box(model,color);
}
"""]
    chunks += [feature["blocks"].get("G","")]
    for slot,signature in signatures.items():
        chunks+=["void "+signature+" {\n"+feature["blocks"].get(slot,"")+"\n}"]
    chunks+=["void Check() {\n"+feature["check"]+"\n}","}"]
    definitions+=chunks
    checks += [
        "eye={6,5,9};target={0,0,0};cameraUp={0,1,0};perspective=true;wireframe=false;",
        "background={0.08f,0.10f,0.15f};mouseX=400;mouseY=300;",
        name+"::OnReset();"+name+"::Check();",
        name+"::OnReset();BeginFrame();"+name+"::OnUpdate(0.016f);"+name+"::OnDraw();",
        'Require(glGetError()==GL_NO_ERROR,"'+id+' draw/update GL error");',
    ]
by_id={f["id"]:f for f in FEATURES}
# Verify the two combinations described in the reference with the same state.
combo_doc=(ROOT/"Markdown"/"기능_조합.md").read_text(encoding="utf-8-sig")
release=re.findall(fence+r"cpp\n(.*?)\n"+fence,combo_doc,re.S)[0]
drag=by_id["I04"]["blocks"]
old_release="if (button==GLFW_MOUSE_BUTTON_LEFT && action==GLFW_RELEASE) dragging=false;"
assert old_release in drag["M"]
mouse=drag["M"].replace(old_release,release)
definitions += ["namespace DragSnap {",drag["G"],
                "void Reset(){"+drag["R"]+"}",
                "void Mouse(int button,int action){"+mouse+"}",
                "void Cursor(){"+drag["C"]+"}","}"]
wave=by_id["A02"]["blocks"]
tilt=by_id["T04"]["blocks"]["D"]
assert "const float angle = 15.0f;" in tilt
tilt=tilt.replace("const float angle = 15.0f;","const float angle = waveAngle;")
definitions += ["namespace WaveGroup {",wave["G"],
                "void Reset(){"+wave["R"]+"}",
                "void Update(float dt){"+wave["U"]+"}",
                "void Draw(){"+tilt+"}","}"]
checks += [
    "DragSnap::Reset();mouseX=420;mouseY=310;DragSnap::Mouse(GLFW_MOUSE_BUTTON_LEFT,GLFW_PRESS);",
    "mouseX=600;mouseY=320;DragSnap::Cursor();DragSnap::Mouse(GLFW_MOUSE_BUTTON_LEFT,GLFW_RELEASE);",
    'Require(glm::length(DragSnap::dragCenter-glm::vec2(600,300))<0.001f&&!DragSnap::dragging,"drag snaps actual object");',
    "DragSnap::Reset();mouseX=420;mouseY=310;DragSnap::Mouse(GLFW_MOUSE_BUTTON_LEFT,GLFW_PRESS);",
    "mouseX=500;mouseY=350;DragSnap::Cursor();DragSnap::Mouse(GLFW_MOUSE_BUTTON_LEFT,GLFW_RELEASE);",
    'Require(glm::length(DragSnap::dragCenter-glm::vec2(480,340))<0.001f,"far drag not snapped");',
    "WaveGroup::Reset();WaveGroup::Update(0.5f);BeginFrame();WaveGroup::Draw();",
    'Require(WaveGroup::waveAngle>0&&WaveGroup::waveAngle<=25&&glGetError()==GL_NO_ERROR,"wave group composition");'
]
base=base.replace("// FEATURE_BLOCK_END","\n".join(definitions)+"\n// FEATURE_BLOCK_END",1)
count=len(FEATURES)
logic=sum(bool(f["check"]) for f in FEATURES)
checks += [
    # Verify that multiple independent input snippets don't skip later handlers.
    "FeatureI01::OnReset();FeatureI01::OnKey(GLFW_KEY_SPACE,GLFW_PRESS,0);",
    'Require(!FeatureI01::showBox,"toggle press");',
    "FeatureI01::OnKey(GLFW_KEY_SPACE,GLFW_REPEAT,0);",
    'Require(!FeatureI01::showBox,"toggle ignores repeat");',
    "FeatureI03::OnReset();for(int i=0;i<8;++i) FeatureI03::OnMouse(GLFW_MOUSE_BUTTON_LEFT,GLFW_PRESS);",
    'Require(FeatureI03::clickItems.size()==5,"creation limit");',
    "eye={6,5,9};target={0,0,0};cameraUp={0,1,0};perspective=true;wireframe=false;",
    "background={0.08f,0.10f,0.15f};",
    'std::cout<<"PASS: '+str(count)+' pasted features compiled and rendered; '+str(logic+2)+' focused logic checks; 2 compositions.\\n";'
]
base=base.replace(function(base,"OnCheck"),"void OnCheck() {\n"+"\n".join(checks)+"\n}")
source=WORK/"examples"/"01_catalog.cpp"
source.write_text(base,encoding="utf-8-sig")
shutil.copy2(ROOT/"build-check.ps1",WORK/"build-check.ps1")
powershell=shutil.which("pwsh") or shutil.which("powershell")
assert powershell,"PowerShell not found"
subprocess.run([powershell,"-NoProfile","-File",str(WORK/"build-check.ps1"),"-Number","01","-RunChecks"],check=True)
print((WORK/".validation"/"01_catalog"/"check.log").read_text(encoding="utf-8-sig").strip())
