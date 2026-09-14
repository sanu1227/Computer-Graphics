#include <GL/glew.h>
#include <GL/glfw3.h>
#include <cstdlib>
#include <ctime>
#include <iostream>

// 프로젝트 속성 > 링커 > 명령줄
// opengl32.lib glew32.lib glfw3.lib

const int QUADRANT_COUNT = 4;
const int MAX_RECT_PER_QUADRANT = 5;
const int MAX_RECT_COUNT = QUADRANT_COUNT * MAX_RECT_PER_QUADRANT;

struct RectangleInfo {
	int quadrant;
	float centerX;
	float centerY;
	float halfWidth;
	float halfHeight;
	float red;
	float green;
	float blue;
};

RectangleInfo rectangles[MAX_RECT_COUNT];
int rectangleCount = 0;
int quadrantRectangleCount[QUADRANT_COUNT] = { 0, 0, 0, 0 };
int selectedRectangle = -1;

float backgroundColor[QUADRANT_COUNT][3] = {
	{ 0.45f, 0.30f, 0.30f },
	{ 0.30f, 0.45f, 0.30f },
	{ 0.30f, 0.30f, 0.45f },
	{ 0.45f, 0.45f, 0.30f }
};

float RandomFloat(float minimum, float maximum) {
	return minimum + (maximum - minimum) * (static_cast<float>(rand()) / RAND_MAX);
}

void ChangeRectangleColor(RectangleInfo& rectangle) {
	rectangle.red = RandomFloat(0.25f, 1.0f);
	rectangle.green = RandomFloat(0.25f, 1.0f);
	rectangle.blue = RandomFloat(0.25f, 1.0f);
}

void AddRectangle(int quadrant) {
	int quadrantIndex = quadrant - 1;
	if (quadrantRectangleCount[quadrantIndex] >= MAX_RECT_PER_QUADRANT)
		return;

	// 같은 영역에 여러 개를 그려도 모두 보이도록 중앙 근처에서 조금씩 위치를 바꾼다.
	const float offsetX[MAX_RECT_PER_QUADRANT] = { 0.00f, -0.18f, 0.18f, -0.15f, 0.15f };
	const float offsetY[MAX_RECT_PER_QUADRANT] = { 0.00f,  0.16f, 0.16f, -0.16f, -0.16f };
	int positionIndex = quadrantRectangleCount[quadrantIndex];

	float baseX = (quadrant == 1 || quadrant == 4) ? 0.5f : -0.5f;
	float baseY = (quadrant == 1 || quadrant == 2) ? 0.5f : -0.5f;

	RectangleInfo& rectangle = rectangles[rectangleCount];
	rectangle.quadrant = quadrant;
	rectangle.centerX = baseX + offsetX[positionIndex];
	rectangle.centerY = baseY + offsetY[positionIndex];
	rectangle.halfWidth = RandomFloat(0.09f, 0.17f);
	rectangle.halfHeight = RandomFloat(0.07f, 0.15f);
	ChangeRectangleColor(rectangle);

	selectedRectangle = rectangleCount;
	++rectangleCount;
	++quadrantRectangleCount[quadrantIndex];
}

void ResetScene() {
	rectangleCount = 0;
	selectedRectangle = -1;

	for (int i = 0; i < QUADRANT_COUNT; ++i) {
		quadrantRectangleCount[i] = 0;
		backgroundColor[i][0] = RandomFloat(0.25f, 0.55f);
		backgroundColor[i][1] = RandomFloat(0.25f, 0.55f);
		backgroundColor[i][2] = RandomFloat(0.25f, 0.55f);
	}
}

void ResizeSelectedRectangle(float amount) {
	if (selectedRectangle < 0)
		return;

	RectangleInfo& rectangle = rectangles[selectedRectangle];
	float newHalfWidth = rectangle.halfWidth + amount;
	float newHalfHeight = rectangle.halfHeight + amount;

	// 너무 작아지거나 사분면 경계를 크게 벗어나지 않도록 제한한다.
	if (newHalfWidth >= 0.03f && newHalfWidth <= 0.30f &&
		newHalfHeight >= 0.03f && newHalfHeight <= 0.30f) {
		rectangle.halfWidth = newHalfWidth;
		rectangle.halfHeight = newHalfHeight;
	}
}

void CharCallback(GLFWwindow* window, unsigned int codepoint) {
	if (codepoint >= '1' && codepoint <= '4') {
		AddRectangle(static_cast<int>(codepoint - '0'));
	}
	else if (codepoint == '+') {
		ResizeSelectedRectangle(0.03f);
	}
	else if (codepoint == '-') {
		ResizeSelectedRectangle(-0.03f);
	}
	else if (codepoint == 'c' || codepoint == 'C') {
		if (selectedRectangle >= 0)
			ChangeRectangleColor(rectangles[selectedRectangle]);
	}
	else if (codepoint == 'r' || codepoint == 'R') {
		ResetScene();
	}
	else if (codepoint == 'q' || codepoint == 'Q') {
		glfwSetWindowShouldClose(window, true);
	}
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	if (button != GLFW_MOUSE_BUTTON_LEFT || action != GLFW_PRESS)
		return;

	double mouseX;
	double mouseY;
	int windowWidth;
	int windowHeight;
	glfwGetCursorPos(window, &mouseX, &mouseY);
	glfwGetWindowSize(window, &windowWidth, &windowHeight);

	float openGLX = static_cast<float>(mouseX / windowWidth * 2.0 - 1.0);
	float openGLY = static_cast<float>(1.0 - mouseY / windowHeight * 2.0);
	selectedRectangle = -1;

	// 나중에 그린 사각형부터 검사하여 화면의 가장 위에 보이는 것을 선택한다.
	for (int i = rectangleCount - 1; i >= 0; --i) {
		RectangleInfo& rectangle = rectangles[i];
		if (openGLX >= rectangle.centerX - rectangle.halfWidth &&
			openGLX <= rectangle.centerX + rectangle.halfWidth &&
			openGLY >= rectangle.centerY - rectangle.halfHeight &&
			openGLY <= rectangle.centerY + rectangle.halfHeight) {
			selectedRectangle = i;
			break;
		}
	}
}

void DrawBackground() {
	const float area[QUADRANT_COUNT][4] = {
		{  0.0f,  0.0f,  1.0f,  1.0f }, // 1사분면
		{ -1.0f,  0.0f,  0.0f,  1.0f }, // 2사분면
		{ -1.0f, -1.0f,  0.0f,  0.0f }, // 3사분면
		{  0.0f, -1.0f,  1.0f,  0.0f }  // 4사분면
	};

	for (int i = 0; i < QUADRANT_COUNT; ++i) {
		glColor3fv(backgroundColor[i]);
		glRectf(area[i][0], area[i][1], area[i][2], area[i][3]);
	}
}

void DrawRectangles() {
	for (int i = 0; i < rectangleCount; ++i) {
		RectangleInfo& rectangle = rectangles[i];
		float left = rectangle.centerX - rectangle.halfWidth;
		float right = rectangle.centerX + rectangle.halfWidth;
		float bottom = rectangle.centerY - rectangle.halfHeight;
		float top = rectangle.centerY + rectangle.halfHeight;

		glColor3f(rectangle.red, rectangle.green, rectangle.blue);
		glRectf(left, bottom, right, top);

		if (i == selectedRectangle) {
			glLineWidth(4.0f);
			glColor3f(1.0f, 1.0f, 1.0f);
			glBegin(GL_LINE_LOOP);
			glVertex2f(left, bottom);
			glVertex2f(right, bottom);
			glVertex2f(right, top);
			glVertex2f(left, top);
			glEnd();
		}
	}
}

int main() {
	srand(static_cast<unsigned int>(time(nullptr)));

	if (!glfwInit()) {
		std::cerr << "GLFW 초기화 실패!" << std::endl;
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

	GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Rectangle Commands", nullptr, nullptr);
	if (!window) {
		std::cerr << "윈도우 생성 실패!" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetCharCallback(window, CharCallback);
	glfwSetMouseButtonCallback(window, MouseButtonCallback);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		std::cerr << "GLEW 초기화 실패!" << std::endl;
		glfwDestroyWindow(window);
		glfwTerminate();
		return -1;
	}

	while (!glfwWindowShouldClose(window)) {
		int framebufferWidth;
		int framebufferHeight;
		glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
		glViewport(0, 0, framebufferWidth, framebufferHeight);

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		DrawBackground();
		DrawRectangles();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
