#include <GL/glew.h>
#include <GL/glfw3.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
using namespace std;

float backgroundRed = 0.0f;
float backgroundGreen = 0.0f;
float backgroundBlue = 1.0f;

bool timerEnabled = false;
double lastColorChangeTime = 0.0;
const double colorChangeInterval = 1.0;

void SetRandomBackgroundColor()
{
	backgroundRed = (rand() % 256) / 255.0f;
	backgroundGreen = (rand() % 256) / 255.0f;
	backgroundBlue = (rand() % 256) / 255.0f;
}

void CharCallback(GLFWwindow* window, unsigned int codepoint) //--- 등록한 사용자 정의 콜백 함수
{
	if (codepoint == 'c' || codepoint == 'C') {
		backgroundRed = 0.0f;
		backgroundGreen = 1.0f;
		backgroundBlue = 1.0f;
	}
	if (codepoint == 'm' || codepoint == 'M') {
		backgroundRed = 1.0f;
		backgroundGreen = 0.0f;
		backgroundBlue = 1.0f;
	}
	if (codepoint == 'y' || codepoint == 'Y') {
		backgroundRed = 1.0f;
		backgroundGreen = 1.0f;
		backgroundBlue = 0.0f;
	}
	if (codepoint == 'a' || codepoint == 'A') {
		SetRandomBackgroundColor();
	}
	if (codepoint == 'g' || codepoint == 'G') {
		backgroundRed = 0.5f;
		backgroundGreen = 0.5f;
		backgroundBlue = 0.5f;
	}
	if (codepoint == 'k' || codepoint == 'K') {
		backgroundRed = 0.0f;
		backgroundGreen = 0.0f;
		backgroundBlue = 0.0f;
	}
	if (codepoint == 't' || codepoint == 'T') {
		timerEnabled = true;
		lastColorChangeTime = glfwGetTime();
	}
	if (codepoint == 's' || codepoint == 'S') {
		timerEnabled = false;
	}
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
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//--- 윈도우 생성
	GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Window", nullptr, nullptr);
	if (!window) {
		std::cerr << "윈도우 생성 실패!" << std::endl;
		glfwTerminate();
		return -1;
	}
	//--- 컨텍스트 설정
	glfwMakeContextCurrent(window);
	glfwSetCharCallback(window, CharCallback);
	//--- GLEW 초기화
	glewExperimental = GL_TRUE; // 최신 기능 사용
	if (glewInit() != GLEW_OK) {
		std::cerr << "GLEW 초기화 실패!" << std::endl;
		return -1;
	}

	//--- 뷰포트 설정
	glViewport(0, 0, 800, 600);
	//--- 메인 루프
	while (!glfwWindowShouldClose(window)) {
		// 입력 처리
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);

		double currentTime = glfwGetTime();
		if (timerEnabled && currentTime - lastColorChangeTime >= colorChangeInterval) {
			SetRandomBackgroundColor();
			lastColorChangeTime = currentTime;
		}
		
		// 화면 지우기
		glClearColor(backgroundRed, backgroundGreen, backgroundBlue, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		// 버퍼 교체
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	//--- 종료 처리
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
