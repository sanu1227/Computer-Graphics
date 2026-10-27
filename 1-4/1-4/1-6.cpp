#include <GL/glew.h>
#include <GL/glfw3.h>
#include <cstdlib>
#include <ctime>
#include <iostream>

// 프로젝트 속성 > 링커 > 명령줄
// opengl32.lib glew32.lib glfw3.lib

const int MAX_RECTANGLES = 128;
const int MIN_START_RECTANGLES = 5;
const int MAX_START_RECTANGLES = 10;

const float MOVE_SPEED = 110.0f;
const float DIAGONAL_SPEED = MOVE_SPEED * 0.7071f;
const float SHRINK_SPEED = 14.0f;
const float COLOR_CHANGE_SPEED = 0.35f;
const float DISAPPEAR_SIZE = 3.0f;

struct Color
{
	float red;
	float green;
	float blue;
};

struct RectangleParticle
{
	float centerX;
	float centerY;
	float size;
	float width;
	float height;
	float velocityX;
	float velocityY;
	Color color;
	bool active;
	bool animating;
	bool becomingBrighter;
};

RectangleParticle rectangles[MAX_RECTANGLES];
int rectangleCount = 0;
int windowWidth = 800;
int windowHeight = 600;

// 위치 이동 애니메이션 선택
// 1: 좌우상하, 2: 대각선, 3: 같은 방향, 4: 8방향
int selectedMovementType = 1;
bool useSelectedMovementType = false;

float RandomColorValue()
{
	return static_cast<float>(rand()) / RAND_MAX;
}

Color MakeRandomColor()
{
	Color color;
	color.red = RandomColorValue();
	color.green = RandomColorValue();
	color.blue = RandomColorValue();
	return color;
}

void AddRectangle(float centerX, float centerY, float size, Color color,
	float velocityX, float velocityY, bool animating, bool becomingBrighter)
{
	if (rectangleCount >= MAX_RECTANGLES)
		return;

	RectangleParticle& rectangle = rectangles[rectangleCount];
	rectangle.centerX = centerX;
	rectangle.centerY = centerY;
	rectangle.size = size;
	rectangle.width = size;
	rectangle.height = size;
	rectangle.velocityX = velocityX;
	rectangle.velocityY = velocityY;
	rectangle.color = color;
	rectangle.active = true;
	rectangle.animating = animating;
	rectangle.becomingBrighter = becomingBrighter;

	rectangleCount++;
}

void CreateStartRectangles()
{
	rectangleCount = 0;

	int startCount = MIN_START_RECTANGLES +
		rand() % (MAX_START_RECTANGLES - MIN_START_RECTANGLES + 1);

	for (int i = 0; i < startCount; ++i)
	{
		float size = 45.0f + static_cast<float>(rand() % 56);
		float halfSize = size / 2.0f;
		float centerX = halfSize +
			static_cast<float>(rand() % (windowWidth - static_cast<int>(size) + 1));
		float centerY = halfSize +
			static_cast<float>(rand() % (windowHeight - static_cast<int>(size) + 1));

		AddRectangle(centerX, centerY, size, MakeRandomColor(),
			0.0f, 0.0f, false, false);
	}
}

bool PointInsideRectangle(const RectangleParticle& rectangle, float x, float y)
{
	float halfWidth = rectangle.width / 2.0f;
	float halfHeight = rectangle.height / 2.0f;

	return x >= rectangle.centerX - halfWidth &&
		x <= rectangle.centerX + halfWidth &&
		y >= rectangle.centerY - halfHeight &&
		y <= rectangle.centerY + halfHeight;
}

void AddFourQuarterRectangles(const RectangleParticle& source,
	const float velocityX[4], const float velocityY[4], bool becomingBrighter)
{
	float childSize = source.size / 2.0f;
	float offset = source.size / 4.0f;

	const float offsetX[4] = { -offset, offset, -offset, offset };
	const float offsetY[4] = { -offset, -offset, offset, offset };

	for (int i = 0; i < 4; ++i)
	{
		AddRectangle(
			source.centerX + offsetX[i],
			source.centerY + offsetY[i],
			childSize,
			source.color,
			velocityX[i],
			velocityY[i],
			true,
			becomingBrighter);
		RectangleParticle& child = rectangles[rectangleCount - 1];
		child.width = childSize;
		child.height = childSize;
	}
}

void SplitWithCardinalMovement(const RectangleParticle& source, bool becomingBrighter)
{
	const float velocityX[4] = { 0.0f, MOVE_SPEED, 0.0f, -MOVE_SPEED };
	const float velocityY[4] = { -MOVE_SPEED, 0.0f, MOVE_SPEED, 0.0f };

	AddFourQuarterRectangles(source, velocityX, velocityY, becomingBrighter);
}

void SplitWithDiagonalMovement(const RectangleParticle& source, bool becomingBrighter)
{
	const float velocityX[4] = {
		-DIAGONAL_SPEED, DIAGONAL_SPEED, -DIAGONAL_SPEED, DIAGONAL_SPEED
	};
	const float velocityY[4] = {
		-DIAGONAL_SPEED, -DIAGONAL_SPEED, DIAGONAL_SPEED, DIAGONAL_SPEED
	};

	AddFourQuarterRectangles(source, velocityX, velocityY, becomingBrighter);
}

void SplitWithSameDirection(const RectangleParticle& source, bool becomingBrighter)
{
	const float directionX[8] = {
		0.0f, DIAGONAL_SPEED, MOVE_SPEED, DIAGONAL_SPEED,
		0.0f, -DIAGONAL_SPEED, -MOVE_SPEED, -DIAGONAL_SPEED
	};
	const float directionY[8] = {
		-MOVE_SPEED, -DIAGONAL_SPEED, 0.0f, DIAGONAL_SPEED,
		MOVE_SPEED, DIAGONAL_SPEED, 0.0f, -DIAGONAL_SPEED
	};

	int direction = rand() % 8;
	float velocityX[4];
	float velocityY[4];

	for (int i = 0; i < 4; ++i)
	{
		velocityX[i] = directionX[direction];
		velocityY[i] = directionY[direction];
	}

	AddFourQuarterRectangles(source, velocityX, velocityY, becomingBrighter);
}

void SplitIntoEightDirections(const RectangleParticle& source, bool becomingBrighter)
{
	const float velocityX[8] = {
		0.0f, DIAGONAL_SPEED, MOVE_SPEED, DIAGONAL_SPEED,
		0.0f, -DIAGONAL_SPEED, -MOVE_SPEED, -DIAGONAL_SPEED
	};
	const float velocityY[8] = {
		-MOVE_SPEED, -DIAGONAL_SPEED, 0.0f, DIAGONAL_SPEED,
		MOVE_SPEED, DIAGONAL_SPEED, 0.0f, -DIAGONAL_SPEED
	};

	float childWidth = source.size / 2.0f;
	float childHeight = source.size / 4.0f;
	float startX = source.centerX - source.size / 4.0f;
	float startY = source.centerY - source.size * 3.0f / 8.0f;

	for (int i = 0; i < 8; ++i)
	{
		AddRectangle(source.centerX, source.centerY, childWidth,
			source.color, velocityX[i], velocityY[i], true,
			becomingBrighter);

		RectangleParticle& child = rectangles[rectangleCount - 1];
		child.width = childWidth;
		child.height = childHeight;
		child.centerX = startX + (i % 2) * childWidth;
		child.centerY = startY + (i / 2) * childHeight;
	}
}

void SplitRectangle(int rectangleIndex)
{
	RectangleParticle source = rectangles[rectangleIndex];
	rectangles[rectangleIndex].active = false;

	int movementType = useSelectedMovementType
		? selectedMovementType
		: 1 + rand() % 4;
	bool becomingBrighter = rand() % 2 == 0;

	if (movementType == 1)
		SplitWithCardinalMovement(source, becomingBrighter);
	else if (movementType == 2)
		SplitWithDiagonalMovement(source, becomingBrighter);
	else if (movementType == 3)
		SplitWithSameDirection(source, becomingBrighter);
	else
		SplitIntoEightDirections(source, becomingBrighter);
}

float MoveColorToward(float value, float target, float amount)
{
	if (value < target)
	{
		value += amount;
		if (value > target)
			value = target;
	}
	else if (value > target)
	{
		value -= amount;
		if (value < target)
			value = target;
	}

	return value;
}

void UpdateRectangleAnimations(float deltaTime)
{
	for (int i = 0; i < rectangleCount; ++i)
	{
		RectangleParticle& rectangle = rectangles[i];

		if (!rectangle.active || !rectangle.animating)
			continue;

		rectangle.centerX += rectangle.velocityX * deltaTime;
		rectangle.centerY += rectangle.velocityY * deltaTime;
		rectangle.size -= SHRINK_SPEED * deltaTime;
		rectangle.width -= SHRINK_SPEED * deltaTime;
		rectangle.height -= SHRINK_SPEED * deltaTime;

		float targetColor = rectangle.becomingBrighter ? 1.0f : 0.0f;
		float colorAmount = COLOR_CHANGE_SPEED * deltaTime;

		rectangle.color.red =
			MoveColorToward(rectangle.color.red, targetColor, colorAmount);
		rectangle.color.green =
			MoveColorToward(rectangle.color.green, targetColor, colorAmount);
		rectangle.color.blue =
			MoveColorToward(rectangle.color.blue, targetColor, colorAmount);

		if (rectangle.width <= DISAPPEAR_SIZE ||
			rectangle.height <= DISAPPEAR_SIZE)
			rectangle.active = false;
	}
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (button != GLFW_MOUSE_BUTTON_LEFT || action != GLFW_PRESS)
		return;

	double mouseX;
	double mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);

	// 나중에 그린 사각형부터 검사한다. 겹쳤을 때 화면 위 사각형이 선택된다.
	for (int i = rectangleCount - 1; i >= 0; --i)
	{
		const RectangleParticle& rectangle = rectangles[i];

		if (rectangle.active && !rectangle.animating &&
			PointInsideRectangle(rectangle,
				static_cast<float>(mouseX), static_cast<float>(mouseY)))
		{
			SplitRectangle(i);
			break;
		}
	}
}

// r 또는 R을 누르면 모든 사각형을 지우고 처음부터 다시 만든다.
void CharacterCallback(GLFWwindow* window, unsigned int codepoint)
{
	if (codepoint >= '1' && codepoint <= '4')
	{
		selectedMovementType = static_cast<int>(codepoint - '0');
		useSelectedMovementType = true;
		std::cout << "위치 이동 애니메이션 "
			<< selectedMovementType << "번 선택" << std::endl;
	}

	if (codepoint == '0')
	{
		useSelectedMovementType = false;
		std::cout << "위치 이동 애니메이션 랜덤 선택" << std::endl;
	}

	if (codepoint == 'r' || codepoint == 'R')
		CreateStartRectangles();
}

void WindowSizeCallback(GLFWwindow* window, int width, int height)
{
	windowWidth = width;
	windowHeight = height;
}

void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void DrawRectangle(const RectangleParticle& rectangle)
{
	float halfWidth = rectangle.width / 2.0f;
	float halfHeight = rectangle.height / 2.0f;
	float left = rectangle.centerX - halfWidth;
	float right = rectangle.centerX + halfWidth;
	float top = rectangle.centerY - halfHeight;
	float bottom = rectangle.centerY + halfHeight;

	glColor3f(rectangle.color.red, rectangle.color.green, rectangle.color.blue);

	glBegin(GL_QUADS);
	glVertex2f(left, top);
	glVertex2f(right, top);
	glVertex2f(right, bottom);
	glVertex2f(left, bottom);
	glEnd();
}

void DrawScene()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, windowWidth, windowHeight, 0.0, -1.0, 1.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	for (int i = 0; i < rectangleCount; ++i)
	{
		if (rectangles[i].active)
			DrawRectangle(rectangles[i]);
	}
}

int main()
{
	srand(static_cast<unsigned int>(time(nullptr)));

	if (!glfwInit())
	{
		std::cerr << "GLFW 초기화 실패!" << std::endl;
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

	GLFWwindow* window =
		glfwCreateWindow(windowWidth, windowHeight, "1-6 Spreading Rectangles", nullptr, nullptr);

	if (!window)
	{
		std::cerr << "윈도우 생성 실패!" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetMouseButtonCallback(window, MouseButtonCallback);
	glfwSetCharCallback(window, CharacterCallback);
	glfwSetWindowSizeCallback(window, WindowSizeCallback);
	glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cerr << "GLEW 초기화 실패!" << std::endl;
		glfwDestroyWindow(window);
		glfwTerminate();
		return -1;
	}

	int framebufferWidth;
	int framebufferHeight;
	glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
	glViewport(0, 0, framebufferWidth, framebufferHeight);

	CreateStartRectangles();
	double previousTime = glfwGetTime();

	while (!glfwWindowShouldClose(window))
	{
		double currentTime = glfwGetTime();
		float deltaTime = static_cast<float>(currentTime - previousTime);
		previousTime = currentTime;

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);

		UpdateRectangleAnimations(deltaTime);

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		DrawScene();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
