#include <fstream>
#include <string>
#define main ExerciseMain
#if ASSIGNMENT == 20
#include "../1-20.cpp"
#elif ASSIGNMENT == 21
#include "../1-21.cpp"
#elif ASSIGNMENT == 22
#include "../1-22.cpp"
#else
#include "../1-23.cpp"
#endif
#undef main
#undef NDEBUG
#include <cassert>

bool Near(float a,float b) {return std::abs(a-b)<0.001f;}

#if ASSIGNMENT == 23
std::vector<unsigned char> ReadDoorFrame(const char* filename)
{
    DrawScene();
    std::vector<unsigned char> pixels(1000 * 750 * 3);
    glReadPixels(0, 0, 1000, 750, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    assert(glGetError() == GL_NO_ERROR);
    assert(glIsEnabled(GL_CULL_FACE));
    std::ofstream image(filename, std::ios::binary);
    image << "P6\n1000 750\n255\n";
    for (int y = 749; y >= 0; y--)
        image.write(reinterpret_cast<char*>(&pixels[y * 3000]), 3000);
    return pixels;
}
#endif

void Check(GLFWwindow* window)
{
#if ASSIGNMENT == 20
    ResetObjects(); UpdateAnimation(1); assert(beltDistance==0);
    KeyCallback(window,GLFW_KEY_B,0,GLFW_PRESS,0); UpdateAnimation(1); assert(Near(beltDistance,1.4f));
    KeyCallback(window,GLFW_KEY_B,0,GLFW_PRESS,0); UpdateAnimation(1); assert(Near(beltDistance,1.4f));
    assert(glm::length(glm::vec3(GetBeltTransform(0)[3])-glm::vec3(GetBeltTransform(14)[3]))<0.001f);
    assert(glm::length(glm::vec3(GetBeltTransform(6.999f)[3])-glm::vec3(GetBeltTransform(7.001f)[3]))<0.003f);
    for(int i=0;i<5;i++) for(int j=i+1;j<5;j++)
        assert(glm::length(glm::vec3(GetBeltTransform(i*2.8f)[3])-glm::vec3(GetBeltTransform(j*2.8f)[3]))>1);
    KeyCallback(window,GLFW_KEY_P,0,GLFW_PRESS,GLFW_MOD_SHIFT); assert(!perspective);
    KeyCallback(window,GLFW_KEY_R,0,GLFW_PRESS,0); assert(perspective && beltDistance==0);
#elif ASSIGNMENT == 21
    ResetObjects(); for(int i=0;i<10;i++) AddBall(); assert(balls.size()==5);
    for(int i=0;i<2000;i++) {
        UpdateAnimation(0.01f);
        for(auto b:balls) {
            assert(std::abs(b.position.x)<=3-radius+0.001f);
            assert(std::abs(b.position.y)<=3-radius+0.001f);
            assert(std::abs(b.position.z)<=4-radius+0.001f);
        }
    }
    roomRoll=30; UpdateAnimation(1); assert(cubeX[0]>0);
    floorOpen=true; for(int i=0;i<300;i++) UpdateAnimation(0.01f);
    assert(cubeY[0]<-4 && (balls.empty() || balls[0].position.y<-3));
    ResetObjects();
#elif ASSIGNMENT == 22
    ResetObjects();
    glm::vec3 initialCamera=cameraPosition;
    KeyCallback(window,GLFW_KEY_X,0,GLFW_PRESS,0);
    assert(Near(cameraPosition.x,initialCamera.x-0.3f));
    KeyCallback(window,GLFW_KEY_X,0,GLFW_PRESS,GLFW_MOD_SHIFT);
    assert(glm::length(cameraPosition-initialCamera)<0.001f);
    KeyCallback(window,GLFW_KEY_Z,0,GLFW_PRESS,0);
    assert(Near(cameraPosition.z,initialCamera.z-0.3f));
    KeyCallback(window,GLFW_KEY_Z,0,GLFW_PRESS,GLFW_MOD_SHIFT);
    assert(glm::length(cameraPosition-initialCamera)<0.001f);
    for(int i=0;i<55;i++) KeyCallback(window,GLFW_KEY_Z,0,GLFW_REPEAT,0);
    assert(cameraPosition.z<0);
    glm::mat4 view=GetViewTransform();
    for(int column=0;column<4;column++)
        for(int row=0;row<4;row++) assert(std::isfinite(view[column][row]));

    // 자전/공전으로 90도 회전한 뒤에는 전진이 +X, 왼쪽 이동이 -Z가 된다.
    for(int rotationKey : {GLFW_KEY_Y,GLFW_KEY_R})
    {
        ResetObjects();
        KeyCallback(window,rotationKey,0,GLFW_PRESS,0);
        float angularSpeed=rotationKey==GLFW_KEY_Y ? 35.0f : 25.0f;
        UpdateAnimation(90.0f/angularSpeed);
        KeyCallback(window,rotationKey,0,GLFW_PRESS,0);
        assert(cameraSpin==0 && cameraOrbit==0);
        glm::vec3 start=cameraPosition;
        glm::mat4 rotation=cameraRotation;
        KeyCallback(window,GLFW_KEY_Z,0,GLFW_PRESS,0);
        assert(glm::length(cameraPosition-start-glm::vec3(0.3f,0,0))<0.001f);
        KeyCallback(window,GLFW_KEY_Z,0,GLFW_PRESS,GLFW_MOD_SHIFT);
        KeyCallback(window,GLFW_KEY_X,0,GLFW_PRESS,0);
        assert(glm::length(cameraPosition-start-glm::vec3(0,0,-0.3f))<0.001f);
        KeyCallback(window,GLFW_KEY_X,0,GLFW_PRESS,GLFW_MOD_SHIFT);
        assert(glm::length(cameraPosition-start)<0.001f);
        for(int column=0;column<4;column++)
            assert(glm::length(cameraRotation[column]-rotation[column])<0.001f);
    }
    KeyCallback(window,GLFW_KEY_C,0,GLFW_PRESS,0);
    assert(glm::length(cameraPosition-initialCamera)<0.001f);
    assert(Near(cameraRotation[0][0],1) && Near(cameraRotation[2][2],1));
    for(int i=0;i<100;i++) KeyCallback(window,GLFW_KEY_RIGHT,0,GLFW_PRESS,0);
    assert(Near(tankPosition.x,floorHalfSize-tankMargin));
    glm::mat4 parts[9]; GetPartTransforms(parts); assert(parts[0][3].x>0);
    KeyCallback(window,GLFW_KEY_T,0,GLFW_PRESS,0); UpdateAnimation(1); assert(centerAngle>0);
    ResetObjects();
#else
    ResetObjects();
    // X/Z 이동은 시선을 바꾸지 않는다. 대문자(Shift)는 반대 방향으로 이동한다.
    glm::vec3 initialCamera=camera;
    glm::vec3 initialDirection=cameraDirection;
    KeyCallback(window,GLFW_KEY_X,0,GLFW_PRESS,0);
    assert(Near(camera.x,initialCamera.x-0.4f));
    assert(glm::length(cameraDirection-initialDirection)<0.001f);
    KeyCallback(window,GLFW_KEY_X,0,GLFW_PRESS,GLFW_MOD_SHIFT);
    assert(glm::length(camera-initialCamera)<0.001f);
    KeyCallback(window,GLFW_KEY_Z,0,GLFW_PRESS,0);
    assert(Near(camera.z,initialCamera.z-0.4f));
    KeyCallback(window,GLFW_KEY_Z,0,GLFW_PRESS,GLFW_MOD_SHIFT);
    assert(glm::length(camera-initialCamera)<0.001f);
    // 예전 목표점인 원점을 지나가도 시선이 뒤집히거나 lookAt 행렬이 망가지지 않는다.
    for(int i=0;i<60;i++) KeyCallback(window,GLFW_KEY_Z,0,GLFW_REPEAT,0);
    assert(camera.z<0 && glm::length(cameraDirection-initialDirection)<0.001f);
    glm::mat4 view=glm::lookAt(camera,camera+cameraDirection,glm::vec3(0,1,0));
    for(int column=0;column<4;column++)
        for(int row=0;row<4;row++) assert(std::isfinite(view[column][row]));
    ResetObjects();
    // 공전 후에도 화면 기준 앞/옆으로 움직여야 하며, 월드 X/Z축으로 고정 이동하면 안 된다.
    for(int i=0;i<18;i++) KeyCallback(window,GLFW_KEY_Y,0,GLFW_PRESS,0);
    glm::vec3 orbitCamera=camera;
    glm::vec3 orbitDirection=cameraDirection;
    KeyCallback(window,GLFW_KEY_Z,0,GLFW_PRESS,0);
    assert(glm::length(camera-orbitCamera-orbitDirection*0.4f)<0.001f);
    KeyCallback(window,GLFW_KEY_Z,0,GLFW_PRESS,GLFW_MOD_SHIFT);
    glm::vec3 right=glm::normalize(glm::cross(orbitDirection,glm::vec3(0,1,0)));
    KeyCallback(window,GLFW_KEY_X,0,GLFW_PRESS,0);
    assert(glm::length(camera-orbitCamera+right*0.4f)<0.001f);
    assert(glm::length(cameraDirection-orbitDirection)<0.001f);
    KeyCallback(window,GLFW_KEY_I,0,GLFW_PRESS,0);
    assert(glm::length(camera-initialCamera)<0.001f);
    assert(glm::length(cameraDirection-initialDirection)<0.001f);
    for(auto& obstacle:obstacles) assert(!Overlaps(robot,obstacle));

    // 제자리 점프 중에는 수평 이동하지 않고, 착지한 다음에는 키를 다시 누르지 않아도 걷는다.
    KeyCallback(window,GLFW_KEY_J,0,GLFW_PRESS,0);
    UpdateAnimation(0.3f);
    assert(jumping && !walking && robot.y > 0);
    assert(Near(robot.x,0) && Near(robot.z,0));
    for(int i=0;i<400 && jumping;i++) UpdateAnimation(0.005f);
    assert(!jumping && walking && Near(robot.y,0));
    float landingZ=robot.z;
    UpdateAnimation(0.1f);
    assert(robot.z < landingZ);
    // 다음 점프도 가능하고, 점프 도중 초기화해도 점프 상태가 남지 않는다.
    KeyCallback(window,GLFW_KEY_J,0,GLFW_PRESS,0);
    assert(jumping && !walking);
    KeyCallback(window,GLFW_KEY_I,0,GLFW_PRESS,0);
    assert(!jumping && walking && Near(verticalSpeed,0));

    robot={5.34f,0,0}; direction={1,0,0}; UpdateAnimation(0.05f); assert(direction.x<0);
    obstacles[0]={{0,0.5f,-2},{1,1,1}};
    robot={0,0,-0.8f}; direction={0,0,-1}; UpdateAnimation(0.1f); assert(direction.z>0);
    robot={0,0,-0.8f}; direction={0,0,-1}; verticalSpeed=0;
    KeyCallback(window,GLFW_KEY_J,0,GLFW_PRESS,0); assert(!walking);
    UpdateAnimation(0.3f); assert(robot.y>1); assert(Near(robot.z,-0.8f));
    KeyCallback(window,GLFW_KEY_W,0,GLFW_PRESS,0);
    UpdateAnimation(0.6f); walking=false;
    for(int i=0;i<200 && jumping;i++) UpdateAnimation(0.005f);
    assert(Near(robot.y,1) && Near(verticalSpeed,0) && walking && !jumping);
    float obstacleLandingZ=robot.z;
    UpdateAnimation(0.1f);
    assert(robot.z < obstacleLandingZ && Near(robot.y,1));
    direction={0,0,1}; walking=true;
    for(int i=0;i<200;i++) UpdateAnimation(0.01f);
    assert(Near(robot.y,0));
    // 로봇을 고정하여 문 자체의 렌더링 변화만 비교한다. 상태값만 바뀌는 오류도 검출한다.
    ResetObjects();
    walking=false;
    // 윗모서리는 고정되고, 문 높이가 줄어들면서 바깥쪽으로 회전한다.
    for(float progress : {0.0f, 0.5f, 1.0f})
    {
        door=progress;
        glm::mat4 model=GetDoorTransform();
        glm::vec3 topLeft=glm::vec3(model*glm::vec4(-0.5f,0.5f,0.5f,1));
        glm::vec3 topRight=glm::vec3(model*glm::vec4(0.5f,0.5f,0.5f,1));
        glm::vec3 bottomLeft=glm::vec3(model*glm::vec4(-0.5f,-0.5f,0.5f,1));
        assert(glm::length(topLeft-glm::vec3(-6,8,6))<0.001f);
        assert(glm::length(topRight-glm::vec3(6,8,6))<0.001f);
        float expectedHeight=8*(1-progress);
        assert(Near(glm::length(bottomLeft-topLeft),expectedHeight));
        float angle=glm::radians(90.0f*progress);
        assert(Near(bottomLeft.y,8-expectedHeight*std::cos(angle)));
        assert(Near(bottomLeft.z,6+expectedHeight*std::sin(angle)));
    }
    assert(doorOpen && Near(door,1));
    auto initialOpen=ReadDoorFrame("23-door-initial.ppm");
    KeyCallback(window,GLFW_KEY_O,0,GLFW_PRESS,0);
    UpdateAnimation(1);
    assert(!doorOpen && Near(door,0));
    auto closed=ReadDoorFrame("23-door-closed.ppm");
    assert(initialOpen != closed);
    KeyCallback(window,GLFW_KEY_O,0,GLFW_PRESS,0);
    KeyCallback(window,GLFW_KEY_O,0,GLFW_REPEAT,0);
    assert(doorOpen);
    UpdateAnimation(0.5f);
    auto halfOpen=ReadDoorFrame("23-door-half.ppm");
    assert(Near(door,0.5f) && closed != halfOpen);
    UpdateAnimation(0.5f);
    auto open=ReadDoorFrame("23-door-open.ppm");
    assert(Near(door,1) && halfOpen != open && closed != open);
    assert(open == initialOpen);
    KeyCallback(window,GLFW_KEY_O,0,GLFW_PRESS,0);
    UpdateAnimation(1);
    assert(Near(door,0) && !doorOpen);
    assert(ReadDoorFrame("23-door-reclosed.ppm") == closed);
    ResetObjects();
    assert(doorOpen && Near(door,1));
#endif
}

int main()
{
    assert(glfwInit()); glfwWindowHint(GLFW_VISIBLE,GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3); glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window=glfwCreateWindow(1000,750,"check",nullptr,nullptr); assert(window);
    glfwMakeContextCurrent(window); glewExperimental=GL_TRUE; assert(glewInit()==GLEW_OK);
    while(glGetError()!=GL_NO_ERROR) {}
    GLuint v=CompileShader(GL_VERTEX_SHADER,vertexShaderSource);
    GLuint f=CompileShader(GL_FRAGMENT_SHADER,fragmentShaderSource);
    assert(v && f);
    GLuint program=glCreateProgram(); glAttachShader(program,v); glAttachShader(program,f); glLinkProgram(program);
    GLint linked; glGetProgramiv(program,GL_LINK_STATUS,&linked); assert(linked); glUseProgram(program);
    GLint transform=glGetUniformLocation(program,"transform"),color=glGetUniformLocation(program,"faceColor");
    MakeVertices();
#if ASSIGNMENT != 22
    transformLocation=transform;
    colorLocation=color;
#endif
#if ASSIGNMENT == 21
    MakeSphereVertices();
    // 구 메시의 법선이 바깥쪽을 향하는지 확인한다.
    for(size_t i=24;i<vertices.size();i+=3) {
        auto a=vertices[i], b=vertices[i+1], c=vertices[i+2];
        assert(glm::dot(glm::cross(b-a,c-a),a+b+c)>-0.00001f);
    }
#endif
    GLuint vao,vbo; glGenVertexArrays(1,&vao); glGenBuffers(1,&vbo);
    glBindVertexArray(vao); glBindBuffer(GL_ARRAY_BUFFER,vbo);
    glBufferData(GL_ARRAY_BUFFER,vertices.size()*sizeof(glm::vec3),vertices.data(),GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(glm::vec3),nullptr); glEnableVertexAttribArray(0);
    Check(window);
#if ASSIGNMENT == 22
    // 카메라 이동은 왼쪽에만 반영되고, 고정 카메라를 쓰는 오른쪽 두 화면은 유지된다.
    DrawScene(transform,color);
    std::vector<unsigned char> before(1000*750*3),after(before.size());
    glReadPixels(0,0,1000,750,GL_RGB,GL_UNSIGNED_BYTE,before.data());
    for(int i=0;i<5;i++) KeyCallback(window,GLFW_KEY_X,0,GLFW_PRESS,0);
    DrawScene(transform,color);
    glReadPixels(0,0,1000,750,GL_RGB,GL_UNSIGNED_BYTE,after.data());
    int changedLeft=0;
    for(int y=0;y<750;y++) for(int x=0;x<1000;x++)
        for(int channel=0;channel<3;channel++)
        {
            int offset=(y*1000+x)*3+channel;
            if(x>=1000*2/3) assert(before[offset]==after[offset]);
            else if(before[offset]!=after[offset]) changedLeft++;
        }
    assert(changedLeft>1000);
    ResetObjects();
#endif
    for(int frame=0;frame<2;frame++) {
#if ASSIGNMENT == 22
        if(frame) {tankPosition.x=2; tankPosition.z=-2;}
        DrawScene(transform,color);
#else
        if(frame) {
#if ASSIGNMENT == 20
            beltMoving=true; UpdateAnimation(3); perspective=false;
#elif ASSIGNMENT == 21
            roomRoll=25; floorOpen=true; UpdateAnimation(0.5f);
#else
            doorOpen=true; UpdateAnimation(1);
#endif
        }
        DrawScene();
#endif
        assert(glGetError()==GL_NO_ERROR); assert(glIsEnabled(GL_DEPTH_TEST));
        std::vector<unsigned char> pixels(1000*750*3); glReadPixels(0,0,1000,750,GL_RGB,GL_UNSIGNED_BYTE,pixels.data());
        int bright=0; for(size_t i=0;i<pixels.size();i+=3) if(pixels[i]>80 || pixels[i+1]>80) bright++;
        assert(bright>10000);
        std::ofstream out(std::to_string(ASSIGNMENT)+"-"+std::to_string(frame)+".ppm",std::ios::binary);
        out<<"P6\n1000 750\n255\n";
        for(int y=749;y>=0;y--) out.write(reinterpret_cast<char*>(&pixels[y*3000]),3000);
    }
    glDeleteBuffers(1,&vbo); glDeleteVertexArrays(1,&vao); glDeleteProgram(program);
    glDeleteShader(v); glDeleteShader(f); glfwDestroyWindow(window); glfwTerminate();
    std::cout<<"PASS "<<ASSIGNMENT<<": behavior, boundaries, shader compilation, OpenGL rendering\n";
}
