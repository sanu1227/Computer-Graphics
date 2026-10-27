#include <GL/glew.h>
#include <GL/glfw3.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

// 프로젝트 속성 > 링커 > 명령줄: opengl32.lib glew32.lib glfw3.lib
// 참고: GLSL 기초 1의 셰이더 컴파일/링크, GLSL 기초 2의 VAO/VBO와 위치/색상 속성.
constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 600;
constexpr float LANE_TOP = 80.0f;
constexpr float LANE_BOTTOM = 520.0f;
constexpr float LANE_X[2] = { 160.0f, 340.0f };
constexpr float BLOCK_WIDTH = 50.0f;
constexpr float BLOCK_HEIGHT = 28.0f;
constexpr float MATCH_TOP = 260.0f;
constexpr float MATCH_BOTTOM = 340.0f;
constexpr float STACK_X = 680.0f;
constexpr float SLIDE_TIME = 0.9f;

struct Color
{
    float red;
    float green;
    float blue;
};

struct Vertex
{
    float x;
    float y;
    float red;
    float green;
    float blue;
};

struct Block
{
    float y;
    float speed;
    Color color;
};

struct MatchedPair
{
    Color lowerColor;
    Color upperColor;
};

GLuint g_shaderProgram = 0;
GLuint g_vao = 0;
GLuint g_vbo = 0;
GLint g_windowSizeLocation = -1;
Block g_blocks[2];
std::vector<MatchedPair> g_stack;
std::vector<Vertex> g_vertices;
bool g_sliding = false;
float g_slideTimer = 0.0f;
float g_alignedY = 0.0f;

std::filesystem::path FindShaderFile(const std::string& fileName)
{
	std::filesystem::path directory = std::filesystem::current_path();

	for (int level = 0; level < 6; ++level)
	{
		std::filesystem::path candidate = directory / fileName;
		if (std::filesystem::exists(candidate))
			return candidate;

		candidate = directory / "1-4" / "1-4" / fileName;
		if (std::filesystem::exists(candidate))
			return candidate;

		if (directory == directory.root_path())
			break;
		directory = directory.parent_path();
	}

	// Visual Studio의 /FC로 기록된 소스 경로에서도 찾는다.
	const auto sourceCandidate = std::filesystem::path(__FILE__).parent_path() / fileName;
	if (std::filesystem::exists(sourceCandidate))
		return sourceCandidate;

	return {};
}

std::string ReadTextFile(const std::string& fileName)
{
	const std::filesystem::path path = FindShaderFile(fileName);
	if (path.empty())
	{
		std::cerr << "셰이더 파일을 찾을 수 없습니다: " << fileName << std::endl;
		return {};
	}

	std::ifstream file(path);
	if (!file)
	{
		std::cerr << "셰이더 파일을 열 수 없습니다: " << path.string() << std::endl;
		return {};
	}

	return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}

GLuint CompileShader(GLenum shaderType, const std::string& source, const char* shaderName)
{
	const char* sourceText = source.c_str();
	GLuint shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &sourceText, nullptr);
	glCompileShader(shader);

	GLint compileSuccess = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compileSuccess);
	if (!compileSuccess)
	{
		std::array<char, 1024> errorLog{};
		glGetShaderInfoLog(shader, static_cast<GLsizei>(errorLog.size()), nullptr, errorLog.data());
		std::cerr << shaderName << " 컴파일 실패:\n" << errorLog.data() << std::endl;
		glDeleteShader(shader);
		return 0;
	}

	return shader;
}

GLuint CreateShaderProgram()
{
	const std::string vertexSource = ReadTextFile("vertex.glsl");
	const std::string fragmentSource = ReadTextFile("fragment.glsl");
	if (vertexSource.empty() || fragmentSource.empty())
		return 0;

	GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexSource, "Vertex shader");
	GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentSource, "Fragment shader");
	if (vertexShader == 0 || fragmentShader == 0)
	{
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		return 0;
	}

	GLuint program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);

	GLint linkSuccess = GL_FALSE;
	glGetProgramiv(program, GL_LINK_STATUS, &linkSuccess);
	if (!linkSuccess)
	{
		std::array<char, 1024> errorLog{};
		glGetProgramInfoLog(program, static_cast<GLsizei>(errorLog.size()), nullptr, errorLog.data());
		std::cerr << "셰이더 프로그램 링크 실패:\n" << errorLog.data() << std::endl;
		glDeleteProgram(program);
		program = 0;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	return program;
}

// 새 도형은 서로 다른 높이와 속도로 시작한다.
void NewBlocks()
{
    const Color colors[] = {
        { 0.35f, 0.61f, 0.83f }, { 1.0f, 0.90f, 0.0f },
        { 0.55f, 0.80f, 0.28f }, { 0.95f, 0.46f, 0.15f }
    };
    const size_t colorIndex = (g_stack.size() % 2) * 2;
    g_blocks[0] = Block{ 140.0f, 165.0f, colors[colorIndex] };
    g_blocks[1] = Block{ 450.0f, -145.0f, colors[colorIndex + 1] };
    g_sliding = false;
    g_slideTimer = 0.0f;
}

void ResetGame()
{
    g_stack.clear();
    g_alignedY = 0.0f;
    NewBlocks();
}

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

void MoveBlock(Block& block, float deltaTime)
{
    const float minimumY = LANE_TOP + BLOCK_HEIGHT * 0.5f;
    const float maximumY = LANE_BOTTOM - BLOCK_HEIGHT * 0.5f;
    block.y += block.speed * deltaTime;

    // 경계를 넘은 거리도 반사하여 프레임마다 이동 속도를 유지한다.
    while (block.y < minimumY || block.y > maximumY)
    {
        if (block.y < minimumY)
        {
            block.y = minimumY * 2.0f - block.y;
            block.speed = std::abs(block.speed);
        }
        if (block.y > maximumY)
        {
            block.y = maximumY * 2.0f - block.y;
            block.speed = -std::abs(block.speed);
        }
    }
}

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

// 많이 쌓여도 화면 밖으로 나가지 않도록 쌓인 도형의 높이를 줄인다.
float StackBlockHeight()
{
    const size_t pairCount = g_stack.size() + (g_sliding ? 1 : 0);
    if (pairCount == 0)
        return BLOCK_HEIGHT;

    return std::min(BLOCK_HEIGHT,
        (LANE_BOTTOM - LANE_TOP) / (2.0f * static_cast<float>(pairCount)));
}

void KeyCallback(GLFWwindow* window, int key, int, int action, int)
{
    // 길게 눌러도 한 번만 판정한다.
    if (action != GLFW_PRESS)
        return;

    switch (key)
    {
    case GLFW_KEY_ENTER:
    case GLFW_KEY_KP_ENTER:
        TryMatch();
        break;
    case GLFW_KEY_R:
        ResetGame();
        break;
    case GLFW_KEY_Q:
    case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        break;
    }
}

void FramebufferSizeCallback(GLFWwindow*, int width, int height)
{
    // 화면 비율이 바뀌어도 800 x 600의 도형 비율을 유지한다.
    const int viewportWidth = std::min(width, height * WINDOW_WIDTH / WINDOW_HEIGHT);
    const int viewportHeight = viewportWidth * WINDOW_HEIGHT / WINDOW_WIDTH;
    glViewport((width - viewportWidth) / 2, (height - viewportHeight) / 2,
        viewportWidth, viewportHeight);
}

void AddVertex(float x, float y, Color color)
{
    g_vertices.push_back(Vertex{ x, y, color.red, color.green, color.blue });
}

void AddQuad(float left, float top, float right, float bottom, Color color)
{
    AddVertex(left, top, color);
    AddVertex(right, top, color);
    AddVertex(right, bottom, color);
    AddVertex(left, top, color);
    AddVertex(right, bottom, color);
    AddVertex(left, bottom, color);
}

void AddOutline(float left, float top, float right, float bottom, Color color)
{
    constexpr float thickness = 1.5f;
    AddQuad(left, top, right, top + thickness, color);
    AddQuad(left, bottom - thickness, right, bottom, color);
    AddQuad(left, top, left + thickness, bottom, color);
    AddQuad(right - thickness, top, right, bottom, color);
}

void AddBlock(float x, float y, float height, Color color)
{
    const float left = x - BLOCK_WIDTH * 0.5f;
    const float right = x + BLOCK_WIDTH * 0.5f;
    const float top = y - height * 0.5f;
    const float bottom = y + height * 0.5f;
    AddQuad(left, top, right, bottom, color);
    // 작은 도형도 내부 색상이 남도록 테두리 두께를 제한한다.
    const float thickness = std::min(1.0f, height * 0.15f);
    const Color border{ 0.23f, 0.40f, 0.58f };
    AddQuad(left, top, right, top + thickness, border);
    AddQuad(left, bottom - thickness, right, bottom, border);
    AddQuad(left, top, left + thickness, bottom, border);
    AddQuad(right - thickness, top, right, bottom, border);
}

void DrawScene()
{
    g_vertices.clear();
    const Color lineColor{ 0.28f, 0.49f, 0.70f };
    AddOutline(40.0f, 40.0f, 760.0f, 560.0f, lineColor);
    for (float x : LANE_X)
        AddOutline(x - 50.0f, LANE_TOP, x + 50.0f, LANE_BOTTOM, lineColor);

    // 맞출 수 있는 순간 점선 영역을 초록색으로 표시한다.
    const Color matchColor = CanMatch() ? Color{ 0.20f, 0.70f, 0.30f } : lineColor;
    for (float x = 80.0f; x < 420.0f; x += 16.0f)
    {
        AddQuad(x, MATCH_TOP, std::min(x + 9.0f, 420.0f), MATCH_TOP + 1.5f, matchColor);
        AddQuad(x, MATCH_BOTTOM - 1.5f, std::min(x + 9.0f, 420.0f), MATCH_BOTTOM, matchColor);
    }
    for (float y = MATCH_TOP; y < MATCH_BOTTOM; y += 16.0f)
    {
        AddQuad(80.0f, y, 81.5f, std::min(y + 9.0f, MATCH_BOTTOM), matchColor);
        AddQuad(418.5f, y, 420.0f, std::min(y + 9.0f, MATCH_BOTTOM), matchColor);
    }

    const float stackHeight = StackBlockHeight();
    for (size_t i = 0; i < g_stack.size(); ++i)
    {
        const float bottom = LANE_BOTTOM - static_cast<float>(i) * stackHeight * 2.0f;
        AddBlock(STACK_X, bottom - stackHeight * 0.5f, stackHeight, g_stack[i].lowerColor);
        AddBlock(STACK_X, bottom - stackHeight * 1.5f, stackHeight, g_stack[i].upperColor);
    }

    if (g_sliding)
    {
        const float t = std::min(g_slideTimer / SLIDE_TIME, 1.0f);
        const float progress = t * t * (3.0f - 2.0f * t);
        const float bottom = LANE_BOTTOM - static_cast<float>(g_stack.size()) * stackHeight * 2.0f;
        const float height = BLOCK_HEIGHT + (stackHeight - BLOCK_HEIGHT) * progress;
        for (int i = 0; i < 2; ++i)
        {
            const float targetY = bottom - stackHeight * (static_cast<float>(i) + 0.5f);
            const float x = LANE_X[i] + (STACK_X - LANE_X[i]) * progress;
            const float y = g_alignedY + (targetY - g_alignedY) * progress;
            AddBlock(x, y, height, g_blocks[i].color);
        }
    }
    else
    {
        for (int i = 0; i < 2; ++i)
            AddBlock(LANE_X[i], g_blocks[i].y, BLOCK_HEIGHT, g_blocks[i].color);
    }

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
}

// 실행 인수 --self-test: 창 없이 실제 이동/입력/쌓기 함수를 검증한다.
void RunSelfTest()
{
    const auto check = [](bool passed, const char* message)
    {
        if (!passed)
            throw std::runtime_error(message);
    };
    ResetGame();
    KeyCallback(nullptr, GLFW_KEY_ENTER, 0, GLFW_PRESS, 0);
    check(!g_sliding, "Enter outside matching area");
    UpdateGame(1.0f);
    check(CanMatch(), "Moving blocks reach a matching opportunity");
    g_blocks[0].y = 275.0f;
    g_blocks[1].y = 325.0f;
    KeyCallback(nullptr, GLFW_KEY_ENTER, 0, GLFW_PRESS, 0);
    check(g_sliding && g_alignedY == 300.0f, "Same matching area accepts different heights");
    ResetGame();
    g_blocks[0].y = 265.0f;
    g_blocks[1].y = 280.0f;
    check(CanMatch(), "Centers inside matching area");
    g_blocks[0].y = MATCH_TOP - 1.0f;
    check(!CanMatch(), "One center outside matching area");
    g_blocks[0].y = MATCH_TOP;
    g_blocks[1].y = MATCH_BOTTOM;
    check(CanMatch(), "Matching area boundaries");
    g_blocks[0].y = 290.0f;
    g_blocks[1].y = 310.0f;
    KeyCallback(nullptr, GLFW_KEY_ENTER, 0, GLFW_REPEAT, 0);
    check(!g_sliding, "Repeated key event");
    KeyCallback(nullptr, GLFW_KEY_ENTER, 0, GLFW_PRESS, 0);
    check(g_sliding && g_blocks[0].y == 300.0f && g_blocks[1].y == 300.0f,
        "Match and alignment");
    UpdateGame(SLIDE_TIME * 0.5f);
    TryMatch();
    check(g_slideTimer > 0.0f && g_stack.empty(), "No duplicate match during animation");
    UpdateGame(SLIDE_TIME);
    check(g_stack.size() == 1 && !g_sliding && g_blocks[0].y == 140.0f,
        "Stack and spawn new blocks");
    check(g_stack[0].lowerColor.blue > 0.8f && g_stack[0].upperColor.red == 1.0f,
        "Keep matched colors");
    for (int i = 0; i < 20; ++i)
    {
        g_blocks[0].y = g_blocks[1].y = 300.0f;
        TryMatch();
        UpdateGame(SLIDE_TIME);
    }
    check(StackBlockHeight() * static_cast<float>(g_stack.size()) * 2.0f <=
        LANE_BOTTOM - LANE_TOP + 0.001f, "Stack stays inside screen");
    g_blocks[0].y = g_blocks[1].y = 300.0f;
    TryMatch();
    KeyCallback(nullptr, GLFW_KEY_R, 0, GLFW_PRESS, 0);
    check(g_stack.empty() && !g_sliding && g_slideTimer == 0.0f, "Reset during animation");
    Block block{ LANE_BOTTOM - BLOCK_HEIGHT * 0.5f - 1.0f, 200.0f, {} };
    MoveBlock(block, 0.1f);
    check(block.speed < 0.0f && block.y < LANE_BOTTOM - BLOCK_HEIGHT * 0.5f,
        "Bottom bounce");
    block = Block{ LANE_TOP + BLOCK_HEIGHT * 0.5f + 1.0f, -200.0f, {} };
    MoveBlock(block, 0.1f);
    check(block.speed > 0.0f && block.y > LANE_TOP + BLOCK_HEIGHT * 0.5f, "Top bounce");
    MoveBlock(block, 10.0f);
    check(block.y >= LANE_TOP + BLOCK_HEIGHT * 0.5f &&
        block.y <= LANE_BOTTOM - BLOCK_HEIGHT * 0.5f, "Multiple bounces");
    ResetGame();
    std::cout << "1-12 self-test passed" << std::endl;
}

int main(int argc, char** argv)
{
    if (argc > 1 && std::string(argv[1]) == "--self-test")
    {
        try
        {
            RunSelfTest();
            return 0;
        }
        catch (const std::exception& error)
        {
            std::cerr << error.what() << std::endl;
            return 1;
        }
    }

    //--- GLFW 초기화
    if (!glfwInit())
    {
        std::cerr << "GLFW 초기화 실패!" << std::endl;
        return -1;
    }
    //--- OpenGL 버전 설정: 3.3 Core Profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //--- 윈도우 생성
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT,
        "1-12 Rectangle Matching | Enter: match | R: reset | Q: quit", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "윈도우 생성 실패!" << std::endl;
        glfwTerminate();
        return -1;
    }
    //--- 컨텍스트 설정
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    //--- GLEW 초기화
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "GLEW 초기화 실패!" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    //--- 외부 GLSL 파일을 컴파일하고 링크
    g_shaderProgram = CreateShaderProgram();
    if (g_shaderProgram == 0)
    {
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    //--- 위치와 색상 속성을 VAO/VBO에 연결
    glGenVertexArrays(1, &g_vao);
    glGenBuffers(1, &g_vbo);
    glBindVertexArray(g_vao);
    glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        reinterpret_cast<void*>(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    g_windowSizeLocation = glGetUniformLocation(g_shaderProgram, "windowSize");
    //--- 뷰포트와 키보드 콜백 설정
    int framebufferWidth, framebufferHeight;
    glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
    FramebufferSizeCallback(window, framebufferWidth, framebufferHeight);
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    glfwSetKeyCallback(window, KeyCallback);
    ResetGame();
    std::cout << "Enter: 점선 영역에서 도형 맞추기, r: 리셋, q: 종료" << std::endl;
    //--- 메인 루프
    double previousTime = glfwGetTime();
    while (!glfwWindowShouldClose(window))
    {
        const double currentTime = glfwGetTime();
        const float deltaTime = std::min(static_cast<float>(currentTime - previousTime), 0.05f);
        previousTime = currentTime;
        // 입력을 현재 표시된 위치에 적용하고, 다음 프레임 상태를 갱신한다.
        glfwPollEvents();
        UpdateGame(deltaTime);
        DrawScene();
        glfwSwapBuffers(window);
    }
    //--- 종료 처리
    glDeleteBuffers(1, &g_vbo);
    glDeleteVertexArrays(1, &g_vao);
    glDeleteProgram(g_shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
