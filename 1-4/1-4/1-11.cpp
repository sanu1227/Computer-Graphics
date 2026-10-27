#include <GL/glew.h>
#include <GL/glfw3.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

constexpr int WINDOW_WIDTH = 1000;
constexpr int WINDOW_HEIGHT = 900;
constexpr int MIN_BOARD_SIZE = 5;
constexpr int MAX_BOARD_SIZE = 30;
constexpr float MOVE_INTERVAL = 0.12f;
constexpr float COLLISION_EFFECT_TIME = 0.70f;

struct Vertex
{
	float x;
	float y;
	float red;
	float green;
	float blue;
};

struct Color
{
	float red;
	float green;
	float blue;
};

enum class ShapeType
{
	Square,
	Triangle,
	InvertedTriangle
};

struct Obstacle
{
	int row;
	int column;
	ShapeType shape;
	float sizeRatio;
	Color color;
};

struct Player
{
	int row = 0;
	int column = 0;
	ShapeType shape = ShapeType::Square;
	Color color{ 1.0f, 0.85f, 0.15f };
};

GLFWwindow* g_window = nullptr;
GLuint g_shaderProgram = 0;
GLuint g_vao = 0;
GLuint g_vbo = 0;
GLint g_windowSizeLocation = -1;

int g_boardSize = 20;
int g_windowWidth = WINDOW_WIDTH;
int g_windowHeight = WINDOW_HEIGHT;
int g_playerPathIndex = 0;
double g_lastMoveTime = -MOVE_INTERVAL;
float g_collisionTimer = 0.0f;
int g_collisionRow = -1;
int g_collisionColumn = -1;
Player g_player;
std::vector<Obstacle> g_obstacles;
std::vector<Vertex> g_vertices;
std::mt19937 g_random(std::random_device{}());

float RandomFloat(float minimum, float maximum)
{
	std::uniform_real_distribution<float> distribution(minimum, maximum);
	return distribution(g_random);
}

int RandomInt(int minimum, int maximum)
{
	std::uniform_int_distribution<int> distribution(minimum, maximum);
	return distribution(g_random);
}

Color RandomColor()
{
	return Color{ RandomFloat(0.25f, 1.0f), RandomFloat(0.25f, 1.0f), RandomFloat(0.25f, 1.0f) };
}

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

void AddTriangle(float centerX, float centerY, float size, ShapeType shape, Color color)
{
	const float halfSize = size * 0.5f;

	if (shape == ShapeType::Triangle)
	{
		AddVertex(centerX, centerY - halfSize, color);
		AddVertex(centerX - halfSize, centerY + halfSize, color);
		AddVertex(centerX + halfSize, centerY + halfSize, color);
	}
	else
	{
		AddVertex(centerX - halfSize, centerY - halfSize, color);
		AddVertex(centerX + halfSize, centerY - halfSize, color);
		AddVertex(centerX, centerY + halfSize, color);
	}
}

void AddShape(float centerX, float centerY, float size, ShapeType shape, Color color)
{
	if (shape == ShapeType::Square)
	{
		const float halfSize = size * 0.5f;
		AddQuad(centerX - halfSize, centerY - halfSize, centerX + halfSize, centerY + halfSize, color);
	}
	else
	{
		AddTriangle(centerX, centerY, size, shape, color);
	}
}

float BoardSide()
{
	return static_cast<float>(std::min(g_windowWidth, g_windowHeight) - 48);
}

float CellSize()
{
	return BoardSide() / static_cast<float>(g_boardSize);
}

float BoardLeft()
{
	return (static_cast<float>(g_windowWidth) - BoardSide()) * 0.5f;
}

float BoardTop()
{
	return (static_cast<float>(g_windowHeight) - BoardSide()) * 0.5f;
}

std::pair<float, float> CellCenter(int row, int column)
{
	const float cellSize = CellSize();
	return {
		BoardLeft() + (static_cast<float>(column) + 0.5f) * cellSize,
		BoardTop() + (static_cast<float>(row) + 0.5f) * cellSize
	};
}

Obstacle* FindObstacle(int row, int column)
{
	for (Obstacle& obstacle : g_obstacles)
	{
		if (obstacle.row == row && obstacle.column == column)
			return &obstacle;
	}
	return nullptr;
}

void CreateObstacles()
{
	g_obstacles.clear();

	std::vector<std::pair<int, int>> cells;
	for (int row = 0; row < g_boardSize; ++row)
	{
		for (int column = 0; column < g_boardSize; ++column)
		{
			if (row != 0 || column != 0)
				cells.emplace_back(row, column);
		}
	}
	std::shuffle(cells.begin(), cells.end(), g_random);

	const int requestedCount = std::max(12, (g_boardSize * g_boardSize) / 6);
	const int obstacleCount = std::min(requestedCount, static_cast<int>(cells.size()));
	for (int i = 0; i < obstacleCount; ++i)
	{
		Obstacle obstacle{};
		obstacle.row = cells[i].first;
		obstacle.column = cells[i].second;
		obstacle.shape = static_cast<ShapeType>(RandomInt(0, 2));
		obstacle.sizeRatio = RandomFloat(0.45f, 0.78f);
		obstacle.color = RandomColor();
		g_obstacles.push_back(obstacle);
	}

	assert(!g_obstacles.empty());
	for (const Obstacle& obstacle : g_obstacles)
	{
		assert(obstacle.row >= 0 && obstacle.row < g_boardSize);
		assert(obstacle.column >= 0 && obstacle.column < g_boardSize);
		assert(obstacle.sizeRatio <= 1.0f);
	}
}

void ResetGame()
{
	g_player = Player{};
	g_playerPathIndex = 0;
	g_lastMoveTime = -MOVE_INTERVAL;
	g_collisionTimer = 0.0f;
	g_collisionRow = -1;
	g_collisionColumn = -1;
	CreateObstacles();
}

void ResolveCollision()
{
	if (Obstacle* obstacle = FindObstacle(g_player.row, g_player.column))
	{
		std::swap(g_player.shape, obstacle->shape);
		g_player.color = obstacle->color;
		g_collisionTimer = COLLISION_EFFECT_TIME;
		g_collisionRow = g_player.row;
		g_collisionColumn = g_player.column;
	}
}

bool MovePlayerToCell(int row, int column)
{
	if (row < 0 || row >= g_boardSize || column < 0 || column >= g_boardSize)
		return false;

	const double currentTime = glfwGetTime();
	if (currentTime - g_lastMoveTime < MOVE_INTERVAL)
		return false;

	g_lastMoveTime = currentTime;
	g_player.row = row;
	g_player.column = column;
	g_playerPathIndex = row * g_boardSize +
		((row % 2 == 0) ? column : g_boardSize - 1 - column);
	ResolveCollision();
	return true;
}

void MovePlayer(int pathDelta)
{
	const int lastPathIndex = g_boardSize * g_boardSize - 1;
	const int nextPathIndex = g_playerPathIndex + pathDelta;
	if (nextPathIndex < 0 || nextPathIndex > lastPathIndex)
		return;

	const int row = nextPathIndex / g_boardSize;
	const int offset = nextPathIndex % g_boardSize;
	const int column = (row % 2 == 0) ? offset : g_boardSize - 1 - offset;
	MovePlayerToCell(row, column);
}

void MovePlayerByCell(int rowDelta, int columnDelta)
{
	MovePlayerToCell(g_player.row + rowDelta, g_player.column + columnDelta);
}

void KeyCallback(GLFWwindow* window, int key, int, int action, int)
{
	if (action != GLFW_PRESS && action != GLFW_REPEAT)
		return;

	if (key == GLFW_KEY_ESCAPE)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	else if (key == GLFW_KEY_RIGHT || key == GLFW_KEY_D)
		MovePlayerByCell(0, 1);
	else if (key == GLFW_KEY_LEFT || key == GLFW_KEY_A)
		MovePlayerByCell(0, -1);
	else if (key == GLFW_KEY_UP || key == GLFW_KEY_W)
		MovePlayerByCell(-1, 0);
	else if (key == GLFW_KEY_DOWN || key == GLFW_KEY_S)
		MovePlayerByCell(1, 0);
	else if (key == GLFW_KEY_SPACE)
		MovePlayer(1);
	else if (key == GLFW_KEY_R)
		ResetGame();
}

void FramebufferSizeCallback(GLFWwindow*, int width, int height)
{
	g_windowWidth = std::max(width, 1);
	g_windowHeight = std::max(height, 1);
	glViewport(0, 0, g_windowWidth, g_windowHeight);
}

void DrawBoard()
{
	const float side = BoardSide();
	const float left = BoardLeft();
	const float top = BoardTop();
	const float cellSize = CellSize();

	AddQuad(left, top, left + side, top + side, Color{ 0.08f, 0.10f, 0.14f });

	const Color gridColor{ 0.24f, 0.28f, 0.36f };
	const float lineWidth = std::max(1.0f, cellSize * 0.035f);
	for (int i = 0; i <= g_boardSize; ++i)
	{
		const float x = left + i * cellSize;
		AddQuad(x - lineWidth * 0.5f, top, x + lineWidth * 0.5f, top + side, gridColor);

		const float y = top + i * cellSize;
		AddQuad(left, y - lineWidth * 0.5f, left + side, y + lineWidth * 0.5f, gridColor);
	}
}

void DrawObstacles()
{
	for (const Obstacle& obstacle : g_obstacles)
	{
		const auto [centerX, centerY] = CellCenter(obstacle.row, obstacle.column);
		AddShape(centerX, centerY, CellSize() * obstacle.sizeRatio, obstacle.shape, obstacle.color);
	}
}

void DrawPlayer()
{
	const auto [centerX, centerY] = CellCenter(g_player.row, g_player.column);
	AddShape(centerX, centerY, CellSize() * 0.66f, g_player.shape, g_player.color);
}

void DrawCollisionEffect()
{
	if (g_collisionTimer <= 0.0f)
		return;

	const auto [centerX, centerY] = CellCenter(g_collisionRow, g_collisionColumn);
	const float cellSize = CellSize();
	const float progress = g_collisionTimer / COLLISION_EFFECT_TIME;
	const float border = std::max(2.0f, cellSize * 0.08f);
	const float halfSize = cellSize * (0.40f + (1.0f - progress) * 0.08f);
	const Color effectColor{ 1.0f, 0.30f + progress * 0.55f, 0.08f };

	AddQuad(centerX - halfSize, centerY - halfSize,
		centerX + halfSize, centerY - halfSize + border, effectColor);
	AddQuad(centerX - halfSize, centerY + halfSize - border,
		centerX + halfSize, centerY + halfSize, effectColor);
	AddQuad(centerX - halfSize, centerY - halfSize,
		centerX - halfSize + border, centerY + halfSize, effectColor);
	AddQuad(centerX + halfSize - border, centerY - halfSize,
		centerX + halfSize, centerY + halfSize, effectColor);
}

void DrawScene(float deltaTime)
{
	g_collisionTimer = std::max(0.0f, g_collisionTimer - deltaTime);
	g_vertices.clear();

	DrawBoard();
	DrawObstacles();
	DrawPlayer();
	DrawCollisionEffect();

	glClearColor(0.035f, 0.045f, 0.07f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(g_shaderProgram);
	glUniform2f(g_windowSizeLocation, static_cast<float>(g_windowWidth), static_cast<float>(g_windowHeight));
	glBindVertexArray(g_vao);
	glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
	glBufferData(GL_ARRAY_BUFFER,
		static_cast<GLsizeiptr>(g_vertices.size() * sizeof(Vertex)),
		g_vertices.data(),
		GL_DYNAMIC_DRAW);
	glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(g_vertices.size()));
}

int ReadBoardSize()
{
	int boardSize = 20;
	std::string input;
	std::cout << "보드 크기 입력 (" << MIN_BOARD_SIZE << "~" << MAX_BOARD_SIZE << ", 기본값 20): " << std::flush;

	while (std::getline(std::cin, input))
	{
		if (input.empty())
			return 20;

		std::stringstream parser(input);
		std::string extra;
		if (!(parser >> boardSize) || (parser >> extra))
		{
			std::cout << "정수를 입력하세요: " << std::flush;
			continue;
		}

		if (boardSize >= MIN_BOARD_SIZE && boardSize <= MAX_BOARD_SIZE)
			return boardSize;

		std::cout << MIN_BOARD_SIZE << "~" << MAX_BOARD_SIZE << " 사이로 입력하세요: " << std::flush;
	}

	std::cin.clear();
	return 20;
}

int main()
{
	g_boardSize = ReadBoardSize();

	if (!glfwInit())
	{
		std::cerr << "GLFW 초기화 실패!" << std::endl;
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	g_window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "1-11 GLSL Board Movement", nullptr, nullptr);
	if (!g_window)
	{
		std::cerr << "윈도우 생성 실패!" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(g_window);
	glfwSetKeyCallback(g_window, KeyCallback);
	glfwSetFramebufferSizeCallback(g_window, FramebufferSizeCallback);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cerr << "GLEW 초기화 실패!" << std::endl;
		glfwDestroyWindow(g_window);
		glfwTerminate();
		return -1;
	}

	g_shaderProgram = CreateShaderProgram();
	if (g_shaderProgram == 0)
	{
		glfwDestroyWindow(g_window);
		glfwTerminate();
		return -1;
	}

	glGenVertexArrays(1, &g_vao);
	glGenBuffers(1, &g_vbo);
	glBindVertexArray(g_vao);
	glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
	g_windowSizeLocation = glGetUniformLocation(g_shaderProgram, "windowSize");

	ResetGame();
	std::cout << "이동: 방향키 또는 WASD, Space=다음 지그재그 칸, R=재시작, ESC=종료" << std::endl;

	double previousTime = glfwGetTime();
	while (!glfwWindowShouldClose(g_window))
	{
		const double currentTime = glfwGetTime();
		const float deltaTime = std::min(static_cast<float>(currentTime - previousTime), 0.1f);
		previousTime = currentTime;

		DrawScene(deltaTime);
		glfwSwapBuffers(g_window);
		glfwPollEvents();
	}

	glDeleteBuffers(1, &g_vbo);
	glDeleteVertexArrays(1, &g_vao);
	glDeleteProgram(g_shaderProgram);
	glfwDestroyWindow(g_window);
	glfwTerminate();
	return 0;
}
