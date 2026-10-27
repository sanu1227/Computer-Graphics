#include <GL/glew.h>
#include <GL/glfw3.h>
#include <cstdlib>
#include <ctime>
#include <iostream>

// 프로젝트 속성 > 링커 > 명령줄
// opengl32.lib glew32.lib glfw3.lib

const int MAX_RECTANGLES = 50;
const int MIN_START_RECTANGLES = 20;
const int MAX_START_RECTANGLES = 40;
const int MAX_ADDED_RECTANGLES = 10;

const float RECTANGLE_SIZE = 20.0f;
const float ERASER_START_SIZE = RECTANGLE_SIZE * 2.0f;
const float ERASER_SIZE_CHANGE = 4.0f;

struct Color
{
	float red;
	float green;
	float blue;
};

struct SmallRectangle
{
	float x;
	float y;
	Color color;
	bool visible;
};

struct EraserRectangle
{
	float centerX;
	float centerY;
	float size;
	Color color;
	bool visible;
};

SmallRectangle rectangles[MAX_RECTANGLES];
EraserRectangle eraser;

int rectangleCount = 0;
int addedRectangleCount = 0;
int windowWidth = 800;
int windowHeight = 600;

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

float Clamp(float value, float minimum, float maximum)
{
	if (value < minimum)
		return minimum;

	if (value > maximum)
		return maximum;

	return value;
}

void AddRectangle(float centerX, float centerY)
{
	if (rectangleCount >= MAX_RECTANGLES)
		return;

	SmallRectangle& rectangle = rectangles[rectangleCount];
	rectangle.x = Clamp(centerX - RECTANGLE_SIZE / 2.0f,
		0.0f, static_cast<float>(windowWidth) - RECTANGLE_SIZE);
	rectangle.y = Clamp(centerY - RECTANGLE_SIZE / 2.0f,
		0.0f, static_cast<float>(windowHeight) - RECTANGLE_SIZE);
	rectangle.color = MakeRandomColor();
	rectangle.visible = true;

	rectangleCount++;
}

void ResetScene(GLFWwindow* window)
{
	glfwGetWindowSize(window, &windowWidth, &windowHeight);

	rectangleCount = 0;
	addedRectangleCount = 0;

	int startRectangleCount =
		MIN_START_RECTANGLES + rand() %
		(MAX_START_RECTANGLES - MIN_START_RECTANGLES + 1);

	for (int i = 0; i < startRectangleCount; ++i)
	{
		float centerX = RECTANGLE_SIZE / 2.0f +
			static_cast<float>(rand() % (windowWidth - static_cast<int>(RECTANGLE_SIZE) + 1));
		float centerY = RECTANGLE_SIZE / 2.0f +
			static_cast<float>(rand() % (windowHeight - static_cast<int>(RECTANGLE_SIZE) + 1));

		AddRectangle(centerX, centerY);
	}

	eraser.centerX = 0.0f;
	eraser.centerY = 0.0f;
	eraser.size = ERASER_START_SIZE;
	eraser.color = { 0.0f, 0.0f, 0.0f };
	eraser.visible = false;
}

bool RectanglesOverlap(float leftA, float topA, float rightA, float bottomA,
	float leftB, float topB, float rightB, float bottomB)
{
	return leftA < rightB && rightA > leftB &&
		topA < bottomB && bottomA > topB;
}

bool EraserTouches(const SmallRectangle& rectangle)
{
	float halfEraserSize = eraser.size / 2.0f;
	float eraserLeft = eraser.centerX - halfEraserSize;
	float eraserTop = eraser.centerY - halfEraserSize;
	float eraserRight = eraser.centerX + halfEraserSize;
	float eraserBottom = eraser.centerY + halfEraserSize;

	return RectanglesOverlap(
		eraserLeft, eraserTop, eraserRight, eraserBottom,
		rectangle.x, rectangle.y,
		rectangle.x + RECTANGLE_SIZE,
		rectangle.y + RECTANGLE_SIZE);
}

void CheckEraserCollisions()
{
	if (!eraser.visible)
		return;

	// 지우개가 커진 뒤 새로 닿는 사각형도 같은 프레임에서 검사한다.
	bool erasedNewRectangle;
	do
	{
		erasedNewRectangle = false;

		for (int i = 0; i < rectangleCount; ++i)
		{
			SmallRectangle& rectangle = rectangles[i];

			if (rectangle.visible && EraserTouches(rectangle))
			{
				rectangle.visible = false;
				eraser.size += ERASER_SIZE_CHANGE;
				eraser.color = rectangle.color;
				erasedNewRectangle = true;
			}
		}
	} while (erasedNewRectangle);
}

void RestoreAllRectangles()
{
	for (int i = 0; i < rectangleCount; ++i)
		rectangles[i].visible = true;
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	double mouseX;
	double mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		eraser.centerX = static_cast<float>(mouseX);
		eraser.centerY = static_cast<float>(mouseY);
		eraser.size = ERASER_START_SIZE;
		eraser.color = { 0.0f, 0.0f, 0.0f };
		eraser.visible = true;

		CheckEraserCollisions();
	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
	{
		eraser.visible = false;
		RestoreAllRectangles();
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		if (addedRectangleCount < MAX_ADDED_RECTANGLES)
		{
			AddRectangle(static_cast<float>(mouseX), static_cast<float>(mouseY));
			addedRectangleCount++;

			if (eraser.visible)
			{
				eraser.size -= ERASER_SIZE_CHANGE;
				if (eraser.size < RECTANGLE_SIZE)
					eraser.size = RECTANGLE_SIZE;
			}
		}
	}
}

void CursorPositionCallback(GLFWwindow* window, double mouseX, double mouseY)
{
	if (!eraser.visible)
		return;

	eraser.centerX = static_cast<float>(mouseX);
	eraser.centerY = static_cast<float>(mouseY);
	CheckEraserCollisions();
}

void CharacterCallback(GLFWwindow* window, unsigned int codepoint)
{
	if (codepoint == 'r' || codepoint == 'R')
		ResetScene(window);
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

void DrawFilledRectangle(float left, float top, float size, Color color)
{
	glColor3f(color.red, color.green, color.blue);

	glBegin(GL_QUADS);
	glVertex2f(left, top);
	glVertex2f(left + size, top);
	glVertex2f(left + size, top + size);
	glVertex2f(left, top + size);
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
		const SmallRectangle& rectangle = rectangles[i];

		if (rectangle.visible)
			DrawFilledRectangle(
				rectangle.x, rectangle.y,
				RECTANGLE_SIZE, rectangle.color);
	}

	if (eraser.visible)
	{
		float halfSize = eraser.size / 2.0f;
		DrawFilledRectangle(
			eraser.centerX - halfSize,
			eraser.centerY - halfSize,
			eraser.size,
			eraser.color);
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
		glfwCreateWindow(windowWidth, windowHeight, "1-5 Rectangle Eraser", nullptr, nullptr);

	if (!window)
	{
		std::cerr << "윈도우 생성 실패!" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetMouseButtonCallback(window, MouseButtonCallback);
	glfwSetCursorPosCallback(window, CursorPositionCallback);
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

	ResetScene(window);

	while (!glfwWindowShouldClose(window))
	{
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);

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
