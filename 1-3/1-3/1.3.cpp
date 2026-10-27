#include <GL/glew.h>
#include <GL/glfw3.h>

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>

using namespace std;

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int MAX_CREATE_COUNT = 10;
const int MAX_RECTANGLE_COUNT = 20;

struct RectangleInfo
{
	double x;
	double y;
	double width;
	double height;
	float red;
	float green;
	float blue;
};

vector<RectangleInfo> rectangles;
int selectedIndex = -1;
double dragOffsetX = 0.0;
double dragOffsetY = 0.0;
bool isDragging = false;

float RandomColor()
{
	return static_cast<float>(rand() % 256) / 255.0f;
}

double RandomRange(double minimum, double maximum)
{
	return minimum + (maximum - minimum) * rand() / static_cast<double>(RAND_MAX);
}

void SetRandomColor(RectangleInfo& rectangle)
{
	rectangle.red = RandomColor();
	rectangle.green = RandomColor();
	rectangle.blue = RandomColor();
}

// a 키로 만드는 사각형은 현재 사각형 수가 10개 미만일 때만 추가한다.
void AddRandomRectangle(GLFWwindow* window)
{
	if (rectangles.size() >= MAX_CREATE_COUNT)
	{
		cout << "a 키로 만들 수 있는 사각형은 최대 10개입니다.\n";
		return;
	}

	int windowWidth;
	int windowHeight;
	glfwGetWindowSize(window, &windowWidth, &windowHeight);

	RectangleInfo rectangle;
	rectangle.width = RandomRange(80.0, 180.0);
	rectangle.height = RandomRange(60.0, 150.0);
	rectangle.width = min(rectangle.width, static_cast<double>(windowWidth));
	rectangle.height = min(rectangle.height, static_cast<double>(windowHeight));
	rectangle.x = RandomRange(0.0, max(0.0, windowWidth - rectangle.width));
	rectangle.y = RandomRange(0.0, max(0.0, windowHeight - rectangle.height));
	SetRandomColor(rectangle);

	// vector의 뒤에 넣고 앞에서부터 그리므로 나중에 만든 것이 항상 맨 위에 보인다.
	rectangles.push_back(rectangle);
}

bool Contains(const RectangleInfo& rectangle, double mouseX, double mouseY)
{
	return mouseX >= rectangle.x && mouseX <= rectangle.x + rectangle.width &&
		mouseY >= rectangle.y && mouseY <= rectangle.y + rectangle.height;
}

bool IsOverlapping(const RectangleInfo& a, const RectangleInfo& b)
{
	return a.x < b.x + b.width && a.x + a.width > b.x &&
		a.y < b.y + b.height && a.y + a.height > b.y;
}

// 뒤에서부터 검사하면 화면상 가장 위에 있는 사각형이 먼저 선택된다.
int FindTopRectangle(double mouseX, double mouseY)
{
	for (int i = static_cast<int>(rectangles.size()) - 1; i >= 0; --i)
	{
		if (Contains(rectangles[i], mouseX, mouseY))
			return i;
	}
	return -1;
}

void MergeSelectedRectangle()
{
	if (selectedIndex < 0 || selectedIndex >= static_cast<int>(rectangles.size()))
		return;

	// 겹친 대상도 위에 있는 사각형부터 찾는다.
	int overlapIndex = -1;
	for (int i = static_cast<int>(rectangles.size()) - 1; i >= 0; --i)
	{
		if (i != selectedIndex && IsOverlapping(rectangles[selectedIndex], rectangles[i]))
		{
			overlapIndex = i;
			break;
		}
	}

	if (overlapIndex == -1)
		return;

	const RectangleInfo selected = rectangles[selectedIndex];
	const RectangleInfo overlap = rectangles[overlapIndex];
	RectangleInfo merged;
	merged.x = min(selected.x, overlap.x);
	merged.y = min(selected.y, overlap.y);
	double maximumX = max(selected.x + selected.width, overlap.x + overlap.width);
	double maximumY = max(selected.y + selected.height, overlap.y + overlap.height);
	merged.width = maximumX - merged.x;
	merged.height = maximumY - merged.y;
	SetRandomColor(merged);

	int first = min(selectedIndex, overlapIndex);
	int second = max(selectedIndex, overlapIndex);
	rectangles.erase(rectangles.begin() + second);
	rectangles.erase(rectangles.begin() + first);
	rectangles.push_back(merged);
}

void SplitRectangle(int index)
{
	if (index < 0 || rectangles.size() >= MAX_RECTANGLE_COUNT)
	{
		if (rectangles.size() >= MAX_RECTANGLE_COUNT)
			cout << "전체 사각형은 최대 20개입니다.\n";
		return;
	}

	const RectangleInfo original = rectangles[index];
	RectangleInfo first = original;
	RectangleInfo second = original;
	double ratio = RandomRange(0.3, 0.7);

	// 긴 방향을 임의의 30~70% 지점에서 나누어 두 사각형을 만든다.
	if (original.width >= original.height)
	{
		first.width = original.width * ratio;
		second.x = original.x + first.width;
		second.width = original.width - first.width;
	}
	else
	{
		first.height = original.height * ratio;
		second.y = original.y + first.height;
		second.height = original.height - first.height;
	}
	SetRandomColor(first);
	SetRandomColor(second);

	rectangles.erase(rectangles.begin() + index);
	rectangles.push_back(first);
	rectangles.push_back(second);
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action != GLFW_PRESS)
		return;
	if (key == GLFW_KEY_A)
		AddRandomRectangle(window);
	else if (key == GLFW_KEY_ESCAPE)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	double mouseX;
	double mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		selectedIndex = FindTopRectangle(mouseX, mouseY);
		if (selectedIndex != -1)
		{
			isDragging = true;
			dragOffsetX = mouseX - rectangles[selectedIndex].x;
			dragOffsetY = mouseY - rectangles[selectedIndex].y;
		}
	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
	{
		if (isDragging)
			MergeSelectedRectangle();
		isDragging = false;
		selectedIndex = -1;
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		SplitRectangle(FindTopRectangle(mouseX, mouseY));
	}
}

void CursorPositionCallback(GLFWwindow* window, double mouseX, double mouseY)
{
	if (!isDragging || selectedIndex < 0)
		return;

	int windowWidth;
	int windowHeight;
	glfwGetWindowSize(window, &windowWidth, &windowHeight);
	RectangleInfo& rectangle = rectangles[selectedIndex];
	rectangle.x = clamp(mouseX - dragOffsetX, 0.0,
		max(0.0, windowWidth - rectangle.width));
	rectangle.y = clamp(mouseY - dragOffsetY, 0.0,
		max(0.0, windowHeight - rectangle.height));
}

void DrawRectangles(GLFWwindow* window)
{
	int windowWidth;
	int windowHeight;
	int frameWidth;
	int frameHeight;
	glfwGetWindowSize(window, &windowWidth, &windowHeight);
	glfwGetFramebufferSize(window, &frameWidth, &frameHeight);
	double scaleX = frameWidth / static_cast<double>(max(1, windowWidth));
	double scaleY = frameHeight / static_cast<double>(max(1, windowHeight));

	glViewport(0, 0, frameWidth, frameHeight);
	glDisable(GL_SCISSOR_TEST);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_SCISSOR_TEST);

	for (const RectangleInfo& rectangle : rectangles)
	{
		int x = static_cast<int>(rectangle.x * scaleX);
		int y = static_cast<int>((windowHeight - rectangle.y - rectangle.height) * scaleY);
		int width = max(1, static_cast<int>(rectangle.width * scaleX));
		int height = max(1, static_cast<int>(rectangle.height * scaleY));
		glScissor(x, y, width, height);
		glClearColor(rectangle.red, rectangle.green, rectangle.blue, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
	}
	glDisable(GL_SCISSOR_TEST);
}

int main()
{
	srand(static_cast<unsigned int>(time(nullptr)));
	if (!glfwInit())
	{
		cerr << "GLFW 초기화 실패\n";
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT,
		"Rectangle Move / Merge / Split", nullptr, nullptr);
	if (window == nullptr)
	{
		cerr << "윈도우 생성 실패\n";
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		cerr << "GLEW 초기화 실패\n";
		glfwDestroyWindow(window);
		glfwTerminate();
		return -1;
	}

	glfwSetKeyCallback(window, KeyCallback);
	glfwSetMouseButtonCallback(window, MouseButtonCallback);
	glfwSetCursorPosCallback(window, CursorPositionCallback);
	glfwSwapInterval(1);

	cout << "a: 랜덤 사각형 생성(최대 10개)\n";
	cout << "왼쪽 드래그: 이동, 놓을 때 겹치면 병합\n";
	cout << "오른쪽 클릭: 두 사각형으로 분할(전체 최대 20개)\n";

	while (!glfwWindowShouldClose(window))
	{
		DrawRectangles(window);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
