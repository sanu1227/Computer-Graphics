#include <GL/glew.h>
#include <GL/glfw3.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>

//프로젝트 속성 > 링커 > 명령줄
// opengl32.lib glew32.lib glfw3.lib

struct RandRect
{
	float x;
	float y;
	float startX;
	float startY;
	float width;
	float height;
	float originalWidth;
	float originalHeight;
	float sizeDirection;

	float red;
	float green;
	float blue;
	float velocityX;
	float velocityY;

	float PatternDirX;
	float PatternDirY;
	float patternTargetY;
	bool patternVertical;

	float cornerDirX;
	float cornerDirY;
};

RandRect rects[5];
int rect_num = 0;
int movementMode = 0; // 0: 정지, 1: 대각선, 2: 가로 지그재그, 3: 가장자리, 4: 최초 위치 복귀
bool sizeAnimationOn = false;
bool colorAnimationOn = false;
float colorChangeTimer = 0.0f;

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT &&
		action == GLFW_PRESS &&
		rect_num < 5)
	{
		double mouseX, mouseY;
		int windowWidth, windowHeight;

		glfwGetCursorPos(window, &mouseX, &mouseY);
		glfwGetWindowSize(window, &windowWidth, &windowHeight);

		RandRect& rect = rects[rect_num];

		// 마우스 좌표 → OpenGL 좌표(-1~1)
		rect.x = static_cast<float>(mouseX / windowWidth * 2.0 - 1.0);
		rect.y = static_cast<float>(1.0 - mouseY / windowHeight * 2.0);
		rect.startX = rect.x;
		rect.startY = rect.y;

		// 랜덤 크기
		rect.width = 0.1f + (rand() % 21) / 100.0f;
		rect.height = 0.1f + (rand() % 21) / 100.0f;
		rect.originalWidth = rect.width;
		rect.originalHeight = rect.height;
		rect.sizeDirection = 1.0f;

		// 랜덤 색상(0.0~1.0)
		rect.red = static_cast<float>(rand()) / RAND_MAX;
		rect.green = static_cast<float>(rand()) / RAND_MAX;
		rect.blue = static_cast<float>(rand()) / RAND_MAX;

		// x축과 y축 방향을 각각 무작위로 정하여 대각선으로 이동한다.
		rect.velocityX = (rand() % 2 == 0) ? 0.5f : -0.5f;
		rect.velocityY = (rand() % 2 == 0) ? 0.5f : -0.5f;
		rect.PatternDirX = (rand() % 2 == 0) ? 0.65f : -0.65f;
		rect.PatternDirY = (rand() % 2 == 0) ? 0.5f : -0.5f;
		rect.patternTargetY = rect.y;
		rect.patternVertical = false;
		rect.cornerDirX = 0.5f;
		rect.cornerDirY = 0.0f;

		rect_num++;
	}
}
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos) {

}

void CharCallback(GLFWwindow* window, unsigned int codepoint) {
	if (codepoint == '1') {
		movementMode = (movementMode == 1) ? 0 : 1;
	}
	if (codepoint == '2') {
		movementMode = (movementMode == 2) ? 0 : 2;
	}
	if (codepoint == '3') {
		movementMode = (movementMode == 3) ? 0 : 3;
	}
	if (codepoint == '4') {
		sizeAnimationOn = !sizeAnimationOn;
	}
	if (codepoint == '5') {
		colorAnimationOn = !colorAnimationOn;
		colorChangeTimer = 0.0f;
	}
	if (codepoint == 's' || codepoint == 'S') {
		movementMode = 0;
		sizeAnimationOn = false;
		colorAnimationOn = false;
	}
	if (codepoint == 'm' || codepoint == 'M') {
		movementMode = 4;
	}
	if (codepoint == 'r' || codepoint == 'R') {
		rect_num = 0;
		movementMode = 0;
		sizeAnimationOn = false;
		colorAnimationOn = false;
		colorChangeTimer = 0.0f;
	}
	if (codepoint == 'q' || codepoint == 'Q') {
		glfwSetWindowShouldClose(window, true);
	}
}

// 4번 명령: 원래 크기의 50%와 150% 사이에서 커졌다 작아진다.
void UpdateSizeAnimation(float deltaTime)
{
	if (!sizeAnimationOn)
		return;

	for (int i = 0; i < rect_num; ++i)
	{
		RandRect& rect = rects[i];
		float sizeChange = rect.sizeDirection * deltaTime;

		rect.width += rect.originalWidth * sizeChange;
		rect.height += rect.originalHeight * sizeChange;

		if (rect.width >= rect.originalWidth * 1.5f)
		{
			rect.width = rect.originalWidth * 1.5f;
			rect.height = rect.originalHeight * 1.5f;
			rect.sizeDirection = -1.0f;
		}
		else if (rect.width <= rect.originalWidth * 0.5f)
		{
			rect.width = rect.originalWidth * 0.5f;
			rect.height = rect.originalHeight * 0.5f;
			rect.sizeDirection = 1.0f;
		}
	}
}

// 5번 명령: 0.2초마다 각 사각형에 새로운 랜덤 색상을 지정한다.
void UpdateColorAnimation(float deltaTime)
{
	if (!colorAnimationOn)
		return;

	colorChangeTimer += deltaTime;
	if (colorChangeTimer < 0.2f)
		return;

	colorChangeTimer = 0.0f;

	for (int i = 0; i < rect_num; ++i)
	{
		rects[i].red = static_cast<float>(rand()) / RAND_MAX;
		rects[i].green = static_cast<float>(rand()) / RAND_MAX;
		rects[i].blue = static_cast<float>(rand()) / RAND_MAX;
	}
}

// m 명령: 각 사각형을 마우스로 처음 그린 위치까지 직선 이동시킨다.
void MoveRectanglesToStart(float deltaTime)
{
	if (movementMode != 4)
		return;

	const float returnSpeed = 0.8f;
	bool allRectanglesArrived = true;

	for (int i = 0; i < rect_num; ++i)
	{
		RandRect& rect = rects[i];
		float distanceX = rect.startX - rect.x;
		float distanceY = rect.startY - rect.y;
		float distance = std::sqrt(distanceX * distanceX + distanceY * distanceY);
		float moveDistance = returnSpeed * deltaTime;

		if (distance <= moveDistance)
		{
			rect.x = rect.startX;
			rect.y = rect.startY;
		}
		else
		{
			rect.x += distanceX / distance * moveDistance;
			rect.y += distanceY / distance * moveDistance;
			allRectanglesArrived = false;
		}
	}

	if (allRectanglesArrived)
		movementMode = 0;
}

// 3번 명령: 오른쪽 벽에 도착한 뒤 화면 가장자리를 시계 방향으로 돈다.
void MoveAlongWindowEdge(float deltaTime)
{
	if (movementMode != 3)
		return;

	for (int i = 0; i < rect_num; ++i)
	{
		RandRect& rect = rects[i];
		float halfWidth = rect.width / 2.0f;
		float halfHeight = rect.height / 2.0f;

		rect.x += rect.cornerDirX * deltaTime;
		rect.y += rect.cornerDirY * deltaTime;

		// 오른쪽 벽에 닿으면 아래로 이동한다.
		if (rect.cornerDirX > 0.0f && rect.x + halfWidth >= 1.0f)
		{
			rect.x = 1.0f - halfWidth;
			rect.cornerDirX = 0.0f;
			rect.cornerDirY = -0.5f;
		}
		// 아래쪽 벽에 닿으면 왼쪽으로 이동한다.
		else if (rect.cornerDirY < 0.0f && rect.y - halfHeight <= -1.0f)
		{
			rect.y = -1.0f + halfHeight;
			rect.cornerDirX = -0.5f;
			rect.cornerDirY = 0.0f;
		}
		// 왼쪽 벽에 닿으면 위로 이동한다.
		else if (rect.cornerDirX < 0.0f && rect.x - halfWidth <= -1.0f)
		{
			rect.x = -1.0f + halfWidth;
			rect.cornerDirX = 0.0f;
			rect.cornerDirY = 0.5f;
		}
		// 위쪽 벽에 닿으면 오른쪽으로 이동한다.
		else if (rect.cornerDirY > 0.0f && rect.y + halfHeight >= 1.0f)
		{
			rect.y = 1.0f - halfHeight;
			rect.cornerDirX = 0.5f;
			rect.cornerDirY = 0.0f;
		}
	}
}

void Pattern(float deltaTime) {
	if (movementMode != 2)
		return;

	for (int i = 0; i < rect_num; ++i) {
		RandRect& rect = rects[i];
		float halfWidth = rect.width / 2.0f;
		float halfHeight = rect.height / 2.0f;
		float topLimit = 1.0f - halfHeight;
		float bottomLimit = -1.0f + halfHeight;


		if (rect.patternVertical) {
			rect.y += rect.PatternDirY * deltaTime;

			if ((rect.PatternDirY > 0.0f && rect.y >= rect.patternTargetY) ||
				(rect.PatternDirY < 0.0f && rect.y <= rect.patternTargetY)) {
				rect.y = rect.patternTargetY;
				rect.patternVertical = false;
			}

			continue;
		}

		// 한쪽 벽에 닿을 때까지 가로 직선으로 이동한다.
		rect.x += rect.PatternDirX * deltaTime;

		if (rect.x + halfWidth >= 1.0f || rect.x - halfWidth <= -1.0f) {
			if (rect.x + halfWidth >= 1.0f)
				rect.x = 1.0f - halfWidth;
			else
				rect.x = -1.0f + halfWidth;

			rect.PatternDirX = -rect.PatternDirX;

			// 다음 가로줄로 이동한다. 상하단에 닿으면 세로 방향을 바꾼다.
			float verticalStep = (rect.PatternDirY > 0.0f) ? 0.25f : -0.25f;
			float nextY = rect.y + verticalStep;

			if (nextY > topLimit) {
				rect.PatternDirY = -0.5f;
				nextY = rect.y - 0.25f;
			}
			else if (nextY < bottomLimit) {
				rect.PatternDirY = 0.5f;
				nextY = rect.y + 0.25f;
			}

			if (nextY > topLimit)
				nextY = topLimit;
			else if (nextY < bottomLimit)
				nextY = bottomLimit;

			rect.patternTargetY = nextY;
			rect.patternVertical = true;
		}
	}
}

void UpdateRectangles(float deltaTime)
{
	if (movementMode != 1)
		return;

	for (int i = 0; i < rect_num; ++i)
	{
		RandRect& rect = rects[i];
		float halfWidth = rect.width / 2.0f;
		float halfHeight = rect.height / 2.0f;

		rect.x += rect.velocityX * deltaTime;
		rect.y += rect.velocityY * deltaTime;

		if (rect.x + halfWidth >= 1.0f)
		{
			rect.x = 1.0f - halfWidth;
			rect.velocityX = -rect.velocityX;
		}
		else if (rect.x - halfWidth <= -1.0f)
		{
			rect.x = -1.0f + halfWidth;
			rect.velocityX = -rect.velocityX;
		}

		if (rect.y + halfHeight >= 1.0f)
		{
			rect.y = 1.0f - halfHeight;
			rect.velocityY = -rect.velocityY;
		}
		else if (rect.y - halfHeight <= -1.0f)
		{
			rect.y = -1.0f + halfHeight;
			rect.velocityY = -rect.velocityY;
		}
	}
}

void DrawRectangle(const RandRect& rect)
{
	float halfWidth = rect.width / 2.0f;
	float halfHeight = rect.height / 2.0f;

	glColor3f(rect.red, rect.green, rect.blue);

	glBegin(GL_QUADS);
	glVertex2f(rect.x - halfWidth, rect.y - halfHeight);
	glVertex2f(rect.x + halfWidth, rect.y - halfHeight);
	glVertex2f(rect.x + halfWidth, rect.y + halfHeight);
	glVertex2f(rect.x - halfWidth, rect.y + halfHeight);
	glEnd();
}

int main() {
	srand(static_cast<unsigned int>(time(nullptr)));

	//--- GLFW 초기화
	if (!glfwInit()) {
		std::cerr << "GLFW 초기화 실패!" << std::endl;
		return -1;
	}
	//--- OpenGL 버전 설정 (예: 3.3 Core Profile)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
	//--- 윈도우 생성
	GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Window", nullptr, nullptr);
	if (!window) {
		std::cerr << "윈도우 생성 실패!" << std::endl;
		glfwTerminate();
		return -1;
	}
	//--- 컨텍스트 설정
	glfwMakeContextCurrent(window);
	glfwSetMouseButtonCallback(window, MouseButtonCallback);
	glfwSetCursorPosCallback(window, CursorPosCallback);
	glfwSetCharCallback(window, CharCallback);
	//--- GLEW 초기화
	glewExperimental = GL_TRUE; // 최신 기능 사용
	if (glewInit() != GLEW_OK) {
		std::cerr << "GLEW 초기화 실패!" << std::endl;
		return -1;
	}
	//--- 뷰포트 설정
	glViewport(0, 0, 800, 600);
	double previousTime = glfwGetTime();
	//--- 메인 루프
	while (!glfwWindowShouldClose(window)) {
		double currentTime = glfwGetTime();
		float deltaTime = static_cast<float>(currentTime - previousTime);
		previousTime = currentTime;

		// 입력 처리
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);
		// 화면 지우기 (회색)
		glClearColor(0.4f, 0.4f, 0.4f, 1.0f); // RGBA (파랑)
		glClear(GL_COLOR_BUFFER_BIT);
		UpdateRectangles(deltaTime);
		Pattern(deltaTime);
		MoveAlongWindowEdge(deltaTime);
		MoveRectanglesToStart(deltaTime);
		UpdateSizeAnimation(deltaTime);
		UpdateColorAnimation(deltaTime);
		for (int i = 0; i < rect_num; ++i)
		{
			DrawRectangle(rects[i]);
		}

		// 버퍼 교체
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	//--- 종료 처리
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
