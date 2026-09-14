#include <windows.h> // WinAPI 창, 그리기, 메시지 처리를 위한 헤더 파일
#include <tchar.h>

#define BASIC_BOARD_SIZE 30
#define MAX_BOARD_SIZE 40
#define MIN_BOARD_SIZE 10
#define INPUT_COUNT 8
#define INPUT_TEXT_MAX 64

#define MODE_INPUT 0
#define MODE_PLAY 1

#define MESSAGE_INPUT 0
#define MESSAGE_INPUT_ERROR 1
#define MESSAGE_READY 2
#define MESSAGE_MOVED 3
#define MESSAGE_SCALED 4
#define MESSAGE_SCALE_BLOCKED 5
#define MESSAGE_AREA 6
#define MESSAGE_BOARD_MAX 7
#define MESSAGE_BOARD_MIN 8
#define MESSAGE_BOARD_BLOCKED 9
#define MESSAGE_UNKNOWN 10
#define MESSAGE_RESET 11

#define COLOR_TEXT RGB(30, 30, 30)
#define COLOR_GRAY RGB(200, 200, 200)
#define COLOR_RECT1 RGB(0, 70, 240)
#define COLOR_RECT2 RGB(0, 145, 55)
#define COLOR_COLLISION_TEXT RGB(230, 0, 0)

HINSTANCE g_hInst;
const WCHAR lpszClass[] = L"My Window Class";
const WCHAR lpszWindowName[] = L"1-5 Rectangle Board";

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

typedef struct RectangleShape {
    int x;
    int y;
    int width;
    int height;
} RectangleShape;

RectangleShape rect1;
RectangleShape rect2;
int boardSize = BASIC_BOARD_SIZE;
int gameMode = MODE_INPUT;
int inputStep = 0;
int inputValues[INPUT_COUNT];
WCHAR inputText[INPUT_TEXT_MAX];
int inputLength = 0;
int lastMessage = MESSAGE_INPUT;
int lastTarget = 0;
int lastValue = 0;

int MinInt(int a, int b)
{
    if (a < b) {
        return a;
    }

    return b;
}

int AbsInt(int value)
{
    if (value < 0) {
        return -value;
    }

    return value;
}

int PositiveModulo(int value, int divisor)
{
    // 이동 결과가 음수가 되거나 보드 크기를 넘어도 항상 보드 안 좌표로 바꾼다.
    // 가장자리 밖으로 나간 도형 일부가 반대편에 나타나는 래핑 처리를 담당한다.
    while (value < 0) {
        value = value + divisor;
    }

    while (value >= divisor) {
        value = value - divisor;
    }

    return value;
}

RectangleShape MakeRectangle(int x1, int y1, int x2, int y2)
{
    RectangleShape rectangle;

    // 사용자가 두 꼭짓점을 어떤 순서로 입력해도 같은 사각형이 되도록
    // 작은 x, 작은 y를 기준 좌표로 삼는다.
    rectangle.x = MinInt(x1, x2);
    rectangle.y = MinInt(y1, y2);

    // 좌표값 양쪽 끝이 모두 사각형에 포함되어야 하므로 차이에 1을 더한다.
    // 예를 들어 x1=2, x2=4라면 2, 3, 4 총 3칸이다.
    rectangle.width = AbsInt(x2 - x1) + 1;
    rectangle.height = AbsInt(y2 - y1) + 1;

    return rectangle;
}

RectangleShape GetRectangle(int target)
{
    // 같은 이동/확대 함수를 두 도형에 공통으로 적용하기 위해
    // target 번호에 따라 현재 도형 값을 복사해서 반환한다.
    if (target == 1) {
        return rect1;
    }

    return rect2;
}

void SaveRectangle(int target, RectangleShape rectangle)
{
    // GetRectangle으로 복사한 도형을 수정한 뒤 다시 실제 전역 도형에 저장한다.
    // 이렇게 하면 도형별로 함수를 따로 만들지 않아도 된다.
    if (target == 1) {
        rect1 = rectangle;
    }
    else {
        rect2 = rectangle;
    }
}

void ClearInputText(void)
{
    int i;

    inputLength = 0;

    for (i = 0; i < INPUT_TEXT_MAX; ++i) {
        inputText[i] = L'\0';
    }
}

void ResetAll(void)
{
    int i;

    // r 명령 또는 프로그램 시작 시 모든 상태를 초기화한다.
    // 보드는 다시 30x30이 되고, 두 도형의 좌표를 처음부터 다시 입력받는다.
    boardSize = BASIC_BOARD_SIZE;
    gameMode = MODE_INPUT;
    inputStep = 0;
    lastMessage = MESSAGE_RESET;
    lastTarget = 0;
    lastValue = 0;

    rect1 = MakeRectangle(0, 0, 0, 0);
    rect2 = MakeRectangle(0, 0, 0, 0);

    for (i = 0; i < INPUT_COUNT; ++i) {
        inputValues[i] = 0;
    }

    ClearInputText();
}

int AxisContains(int start, int length, int position)
{
    int step;
    int wrappedPosition;

    // 한 축에서 도형이 차지하는 모든 칸을 검사한다.
    // start + step이 보드를 넘어가면 PositiveModulo로 반대편 좌표가 된다.
    for (step = 0; step < length; ++step) {
        wrappedPosition = PositiveModulo(start + step, boardSize);

        if (wrappedPosition == position) {
            return 1;
        }
    }

    return 0;
}

int CellInRectangle(RectangleShape rectangle, int x, int y)
{
    int inX;
    int inY;

    // 한 칸이 사각형 내부인지 알려면 x축 범위와 y축 범위에 모두 들어가야 한다.
    // 이 함수는 보드 출력과 충돌 검사에서 같은 기준으로 사용된다.
    inX = AxisContains(rectangle.x, rectangle.width, x);
    inY = AxisContains(rectangle.y, rectangle.height, y);

    if (inX == 1) {
        if (inY == 1) {
            return 1;
        }
    }

    return 0;
}

int IsRectangleVisible(int target)
{
    // 좌표 입력 도중에는 아직 완성되지 않은 도형을 그리지 않는다.
    // 1번 도형은 입력값 4개가 들어간 뒤, 2번 도형은 입력값 8개가 들어간 뒤 보인다.
    if (gameMode == MODE_PLAY) {
        return 1;
    }

    if (target == 1) {
        if (inputStep >= 4) {
            return 1;
        }
    }
    else {
        if (inputStep >= 8) {
            return 1;
        }
    }

    return 0;
}

int CollisionArea(void)
{
    int x;
    int y;
    int inFirst;
    int inSecond;
    int count = 0;

    if (IsRectangleVisible(1) == 0) {
        return 0;
    }

    if (IsRectangleVisible(2) == 0) {
        return 0;
    }

    // 보드의 모든 칸을 검사해서 두 도형에 동시에 포함되는 칸 수를 센다.
    // 이 값이 0보다 크면 충돌 중이며, DrawBoard에서 해당 칸을 다른 색으로 칠한다.
    for (y = 0; y < boardSize; ++y) {
        for (x = 0; x < boardSize; ++x) {
            inFirst = CellInRectangle(rect1, x, y);
            inSecond = CellInRectangle(rect2, x, y);

            if (inFirst == 1) {
                if (inSecond == 1) {
                    count = count + 1;
                }
            }
        }
    }

    return count;
}

int RectangleArea(int target)
{
    RectangleShape rectangle;

    rectangle = GetRectangle(target);

    return rectangle.width * rectangle.height;
}

int DimensionAfterScale(int currentLength, int change)
{
    int nextLength;

    // 축소하면 1보다 작아질 수 없고, 확대하면 현재 보드 크기를 넘을 수 없다.
    // 범위를 넘어가는 축은 기존 값을 그대로 반환해서 변경하지 않는다.
    nextLength = currentLength + change;

    if (nextLength < 1) {
        return currentLength;
    }

    if (nextLength > boardSize) {
        return currentLength;
    }

    return nextLength;
}

void MoveRectangle(int target, int dx, int dy)
{
    RectangleShape rectangle;

    rectangle = GetRectangle(target);

    // 기준 좌표만 한 칸 이동시키고, 도형의 크기는 그대로 둔다.
    // PositiveModulo 덕분에 보드 끝을 넘어도 반대편 좌표로 이어진다.
    rectangle.x = PositiveModulo(rectangle.x + dx, boardSize);
    rectangle.y = PositiveModulo(rectangle.y + dy, boardSize);

    SaveRectangle(target, rectangle);

    lastMessage = MESSAGE_MOVED;
    lastTarget = target;
    lastValue = 0;
}

void ScaleRectangle(int target, int widthChange, int heightChange)
{
    RectangleShape rectangle;
    int oldWidth;
    int oldHeight;

    rectangle = GetRectangle(target);
    oldWidth = rectangle.width;
    oldHeight = rectangle.height;

    // widthChange와 heightChange 조합으로 전체 확대/축소,
    // x축만 변경, y축만 변경, x확대+y축소 같은 명령을 모두 처리한다.
    rectangle.width = DimensionAfterScale(rectangle.width, widthChange);
    rectangle.height = DimensionAfterScale(rectangle.height, heightChange);

    SaveRectangle(target, rectangle);

    if (oldWidth == rectangle.width) {
        if (oldHeight == rectangle.height) {
            lastMessage = MESSAGE_SCALE_BLOCKED;
            lastTarget = target;
            lastValue = 0;
            return;
        }
    }

    lastMessage = MESSAGE_SCALED;
    lastTarget = target;
    lastValue = 0;
}

int RectangleFitsInBoard(RectangleShape rectangle, int nextBoardSize)
{
    // 보드 자체를 줄일 때는 도형이 잘려서는 안 된다.
    // 그래서 다음 보드 크기 안에 기준 좌표와 끝 좌표가 모두 들어가는지 확인한다.
    if (rectangle.width > nextBoardSize) {
        return 0;
    }

    if (rectangle.height > nextBoardSize) {
        return 0;
    }

    if (rectangle.x + rectangle.width > nextBoardSize) {
        return 0;
    }

    if (rectangle.y + rectangle.height > nextBoardSize) {
        return 0;
    }

    return 1;
}

void GrowBoard(void)
{
    // c 명령: 보드의 x축, y축 칸 수를 동시에 1씩 늘린다.
    // 기본 30에서 최대 10칸만 늘어날 수 있으므로 40에서 멈춘다.
    if (boardSize >= MAX_BOARD_SIZE) {
        lastMessage = MESSAGE_BOARD_MAX;
        lastTarget = 0;
        lastValue = 0;
        return;
    }

    boardSize = boardSize + 1;
    lastMessage = MESSAGE_READY;
    lastTarget = 0;
    lastValue = 0;
}

void ShrinkBoard(void)
{
    int nextBoardSize;
    int firstFits;
    int secondFits;

    // d 명령: 보드의 x축, y축 칸 수를 동시에 1씩 줄인다.
    // 기본 30에서 최대 20칸만 줄어들 수 있으므로 10에서 멈춘다.
    if (boardSize <= MIN_BOARD_SIZE) {
        lastMessage = MESSAGE_BOARD_MIN;
        lastTarget = 0;
        lastValue = 0;
        return;
    }

    nextBoardSize = boardSize - 1;
    firstFits = RectangleFitsInBoard(rect1, nextBoardSize);
    secondFits = RectangleFitsInBoard(rect2, nextBoardSize);

    if (firstFits == 0) {
        lastMessage = MESSAGE_BOARD_BLOCKED;
        lastTarget = 0;
        lastValue = nextBoardSize;
        return;
    }

    if (secondFits == 0) {
        lastMessage = MESSAGE_BOARD_BLOCKED;
        lastTarget = 0;
        lastValue = nextBoardSize;
        return;
    }

    boardSize = nextBoardSize;
    lastMessage = MESSAGE_READY;
    lastTarget = 0;
    lastValue = 0;
}

void ShowArea(int target)
{
    // GUI 창에서는 콘솔 출력 대신 상태 줄에 면적을 표시한다.
    // 1번 도형은 b, 2번 도형은 n을 사용한다.
    lastMessage = MESSAGE_AREA;
    lastTarget = target;
    lastValue = RectangleArea(target);
}

int IsInputSeparator(WCHAR ch)
{
    // 네 좌표를 한 번에 입력할 때 숫자 사이를 구분하는 문자이다.
    // 공백, 쉼표, 괄호를 허용해서 "2 2 6 5", "2,2,6,5", "(2,2) (6,5)" 모두 받을 수 있다.
    if (ch == L' ') {
        return 1;
    }

    if (ch == L',') {
        return 1;
    }

    if (ch == L'(') {
        return 1;
    }

    if (ch == L')') {
        return 1;
    }

    if (ch == L'\0') {
        return 1;
    }

    return 0;
}

int SaveFourInputNumbers(int startIndex)
{
    int i;
    int count = 0;
    int value = 0;
    int hasDigit = 0;
    WCHAR ch;

    // inputText 한 줄에서 숫자 4개를 찾아 inputValues에 저장한다.
    // startIndex가 0이면 1번 도형, 4이면 2번 도형의 x1, y1, x2, y2 위치가 된다.
    for (i = 0; i <= inputLength; ++i) {
        ch = inputText[i];

        if (ch >= L'0') {
            if (ch <= L'9') {
                hasDigit = 1;
                value = value * 10 + ch - L'0';
            }
            else {
                return 0;
            }
        }
        else if (IsInputSeparator(ch) == 1) {
            if (hasDigit == 1) {
                if (count >= 4) {
                    return 0;
                }

                if (value >= boardSize) {
                    return 0;
                }

                inputValues[startIndex + count] = value;
                count = count + 1;
                value = 0;
                hasDigit = 0;
            }
        }
        else {
            return 0;
        }
    }

    if (count == 4) {
        return 1;
    }

    return 0;
}

void FinishRectangleInput(void)
{
    if (inputLength == 0) {
        lastMessage = MESSAGE_INPUT_ERROR;
        return;
    }

    if (inputStep == 0) {
        if (SaveFourInputNumbers(0) == 0) {
            lastMessage = MESSAGE_INPUT_ERROR;
            return;
        }

        // 1번 도형의 x1, y1, x2, y2 네 값이 한 번에 입력된 시점이다.
        rect1 = MakeRectangle(inputValues[0], inputValues[1], inputValues[2], inputValues[3]);
        inputStep = 4;
        ClearInputText();
        lastMessage = MESSAGE_INPUT;
    }
    else {
        if (SaveFourInputNumbers(4) == 0) {
            lastMessage = MESSAGE_INPUT_ERROR;
            return;
        }

        // 2번 도형의 네 값까지 입력되면 명령어를 받을 수 있는 플레이 상태로 전환한다.
        rect2 = MakeRectangle(inputValues[4], inputValues[5], inputValues[6], inputValues[7]);
        inputStep = 8;
        gameMode = MODE_PLAY;
        ClearInputText();
        lastMessage = MESSAGE_READY;
    }
}

void AddInputCharacter(WPARAM wParam)
{
    // 좌표 입력 상태에서는 숫자와 구분 문자만 받는다.
    // 네 좌표를 한 줄에 받으므로 입력 칸 길이는 넉넉하게 잡았다.
    if (inputLength >= INPUT_TEXT_MAX - 1) {
        return;
    }

    inputText[inputLength] = (WCHAR)wParam;
    inputLength = inputLength + 1;
    inputText[inputLength] = L'\0';
}

void BackspaceInput(void)
{
    if (inputLength <= 0) {
        return;
    }

    inputLength = inputLength - 1;
    inputText[inputLength] = L'\0';
}

void ProcessInputChar(WPARAM wParam)
{
    if (wParam >= L'0') {
        if (wParam <= L'9') {
            AddInputCharacter(wParam);
            lastMessage = MESSAGE_INPUT;
            return;
        }
    }

    if (IsInputSeparator((WCHAR)wParam) == 1) {
        if (wParam != VK_RETURN) {
            if (wParam != L'\0') {
                AddInputCharacter(wParam);
                lastMessage = MESSAGE_INPUT;
                return;
            }
        }
    }

    if (wParam == VK_BACK) {
        BackspaceInput();
        lastMessage = MESSAGE_INPUT;
    }
    else if (wParam == VK_RETURN) {
        FinishRectangleInput();
    }
}

void ProcessPlayCommand(HWND hWnd, WPARAM wParam)
{
    // 공통 명령어: 보드 크기 변경, 리셋, 종료를 처리한다.
    if (wParam == L'q') {
        DestroyWindow(hWnd);
    }
    else if (wParam == L'r') {
        ResetAll();
    }
    else if (wParam == L'c') {
        GrowBoard();
    }
    else if (wParam == L'd') {
        ShrinkBoard();
    }
    else if (wParam == L'x') {
        MoveRectangle(1, 1, 0);
    }
    else if (wParam == L'X') {
        MoveRectangle(1, -1, 0);
    }
    else if (wParam == L'y') {
        MoveRectangle(1, 0, 1);
    }
    else if (wParam == L'Y') {
        MoveRectangle(1, 0, -1);
    }
    else if (wParam == L's') {
        ScaleRectangle(1, -1, -1);
    }
    else if (wParam == L'S') {
        ScaleRectangle(1, 1, 1);
    }
    else if (wParam == L'i') {
        ScaleRectangle(1, 1, 0);
    }
    else if (wParam == L'I') {
        ScaleRectangle(1, -1, 0);
    }
    else if (wParam == L'j') {
        ScaleRectangle(1, 0, 1);
    }
    else if (wParam == L'J') {
        ScaleRectangle(1, 0, -1);
    }
    else if (wParam == L'a') {
        ScaleRectangle(1, 1, -1);
    }
    else if (wParam == L'A') {
        ScaleRectangle(1, -1, 1);
    }
    else if (wParam == L'b') {
        ShowArea(1);
    }

    else if (wParam == L'u') {
        MoveRectangle(2, 1, 0);
    }
    else if (wParam == L'U') {
        MoveRectangle(2, -1, 0);
    }
    else if (wParam == L'v') {
        MoveRectangle(2, 0, 1);
    }
    else if (wParam == L'V') {
        MoveRectangle(2, 0, -1);
    }
    else if (wParam == L'g') {
        ScaleRectangle(2, -1, -1);
    }
    else if (wParam == L'G') {
        ScaleRectangle(2, 1, 1);
    }
    else if (wParam == L'h') {
        ScaleRectangle(2, 1, 0);
    }
    else if (wParam == L'H') {
        ScaleRectangle(2, -1, 0);
    }
    else if (wParam == L'k') {
        ScaleRectangle(2, 0, 1);
    }
    else if (wParam == L'K') {
        ScaleRectangle(2, 0, -1);
    }
    else if (wParam == L'm') {
        ScaleRectangle(2, 1, -1);
    }
    else if (wParam == L'M') {
        ScaleRectangle(2, -1, 1);
    }
    else if (wParam == L'n') {
        ShowArea(2);
    }
    else {
        lastMessage = MESSAGE_UNKNOWN;
        lastTarget = 0;
        lastValue = (int)wParam;
    }
}

void DrawTextLine(HDC hDC, int x, int y, const WCHAR text[])
{
    TextOutW(hDC, x, y, text, lstrlenW(text));
}

void DrawInputGuide(HDC hDC, int x, int y)
{
    WCHAR line[160];
    int rectangleNumber;

    if (inputStep < 4) {
        rectangleNumber = 1;
    }
    else {
        rectangleNumber = 2;
    }

    // 현재 입력해야 할 도형 번호를 표시한다.
    // 사용자는 x1 y1 x2 y2 네 값을 한 줄에 입력한 뒤 Enter를 누르면 된다.
    wsprintfW(line, L"%d번 도형 좌표 입력: %s", rectangleNumber, inputText);
    DrawTextLine(hDC, x, y, line);

    wsprintfW(line, L"x1 y1 x2 y2 순서로 4개 입력 (각 좌표 0 ~ %d)", boardSize - 1);
    DrawTextLine(hDC, x, y + 28, line);
    DrawTextLine(hDC, x, y + 56, L"예: 2 2 6 5  입력 후 Enter");
}

void DrawMessage(HDC hDC, int x, int y)
{
    WCHAR line[180];

    if (lastMessage == MESSAGE_INPUT) {
        DrawTextLine(hDC, x, y, L"상태: 좌표를 입력하는 중입니다.");
    }
    else if (lastMessage == MESSAGE_INPUT_ERROR) {
        SetTextColor(hDC, RGB(210, 40, 40));
        DrawTextLine(hDC, x, y, L"상태: 현재 보드 범위 안의 좌표 4개를 입력해야 합니다.");
        SetTextColor(hDC, COLOR_TEXT);
    }
    else if (lastMessage == MESSAGE_READY) {
        DrawTextLine(hDC, x, y, L"상태: 명령어를 입력하세요.");
    }
    else if (lastMessage == MESSAGE_MOVED) {
        wsprintfW(line, L"상태: %d번 도형을 이동했습니다.", lastTarget);
        DrawTextLine(hDC, x, y, line);
    }
    else if (lastMessage == MESSAGE_SCALED) {
        wsprintfW(line, L"상태: %d번 도형의 크기를 변경했습니다.", lastTarget);
        DrawTextLine(hDC, x, y, line);
    }
    else if (lastMessage == MESSAGE_SCALE_BLOCKED) {
        wsprintfW(line, L"상태: %d번 도형은 범위 제한 때문에 크기가 바뀌지 않았습니다.", lastTarget);
        DrawTextLine(hDC, x, y, line);
    }
    else if (lastMessage == MESSAGE_AREA) {
        wsprintfW(line, L"상태: %d번 도형의 면적은 %d입니다.", lastTarget, lastValue);
        DrawTextLine(hDC, x, y, line);
    }
    else if (lastMessage == MESSAGE_BOARD_MAX) {
        wsprintfW(line, L"상태: 보드는 최대 %d x %d까지만 늘릴 수 있습니다.", MAX_BOARD_SIZE, MAX_BOARD_SIZE);
        DrawTextLine(hDC, x, y, line);
    }
    else if (lastMessage == MESSAGE_BOARD_MIN) {
        wsprintfW(line, L"상태: 보드는 최소 %d x %d까지만 줄일 수 있습니다.", MIN_BOARD_SIZE, MIN_BOARD_SIZE);
        DrawTextLine(hDC, x, y, line);
    }
    else if (lastMessage == MESSAGE_BOARD_BLOCKED) {
        wsprintfW(line, L"상태: %d x %d로 줄이면 도형이 보드 밖으로 나가므로 줄이지 않았습니다.",
            lastValue, lastValue);
        DrawTextLine(hDC, x, y, line);
    }
    else if (lastMessage == MESSAGE_UNKNOWN) {
        wsprintfW(line, L"상태: 알 수 없는 명령어 '%c'입니다.", lastValue);
        DrawTextLine(hDC, x, y, line);
    }
    else if (lastMessage == MESSAGE_RESET) {
        DrawTextLine(hDC, x, y, L"상태: 보드와 입력값을 초기화했습니다.");
    }
}

void DrawRectangleInfo(HDC hDC, int x, int y, int target, RectangleShape rectangle)
{
    WCHAR line[160];

    wsprintfW(line, L"%d번 도형  시작(%d, %d)  가로 %d  세로 %d  면적 %d",
        target, rectangle.x, rectangle.y, rectangle.width, rectangle.height,
        rectangle.width * rectangle.height);
    DrawTextLine(hDC, x, y, line);
}

void DrawHelp(HDC hDC, int x, int y)
{
    DrawTextLine(hDC, x, y, L"1번: x/X 우/좌, y/Y 아래/위, s/S 축소/확대, i/I x확대/축소, j/J y확대/축소, a/A 교차확대, b 면적");
    DrawTextLine(hDC, x, y + 24, L"2번: u/U 우/좌, v/V 아래/위, g/G 축소/확대, h/H x확대/축소, k/K y확대/축소, m/M 교차확대, n 면적");
    DrawTextLine(hDC, x, y + 48, L"공통: c 보드 확대, d 보드 축소, r 리셋, q 종료");
}

void DrawBoard(HDC hDC, int left, int top, int cellSize)
{
    int x;
    int y;
    int inFirst;
    int inSecond;
    RECT cellRect;
    int dotX;
    int dotY;
    int strongDot;

    // 30x30에서 시작하는 현재 보드의 모든 칸을 '.' 문자로 찍는다.
    // 보드 자체는 배경색이나 격자선을 쓰지 않고 오직 점 문자만으로 표현한다.
    // 도형 칸은 같은 '.'을 더 진한 색으로 여러 번 출력해서 사각형 모양만 눈에 띄게 한다.
    for (y = 0; y < boardSize; ++y) {
        for (x = 0; x < boardSize; ++x) {
            inFirst = 0;
            inSecond = 0;
            strongDot = 0;

            if (IsRectangleVisible(1) == 1) {
                inFirst = CellInRectangle(rect1, x, y);
            }

            if (IsRectangleVisible(2) == 1) {
                inSecond = CellInRectangle(rect2, x, y);
            }

            cellRect.left = left + x * cellSize;
            cellRect.top = top + y * cellSize;
            cellRect.right = cellRect.left + cellSize;
            cellRect.bottom = cellRect.top + cellSize;

            SetBkMode(hDC, TRANSPARENT);
            dotX = cellRect.left + cellSize / 3;
            dotY = cellRect.top + cellSize / 7;

            if (inFirst == 1) {
                if (inSecond == 1) {
                    SetTextColor(hDC, COLOR_COLLISION_TEXT);
                    SetBkMode(hDC, TRANSPARENT);
                    strongDot = 1;
                }
                else {
                    SetTextColor(hDC, COLOR_RECT1);
                    strongDot = 1;
                }
            }
            else if (inSecond == 1) {
                SetTextColor(hDC, COLOR_RECT2);
                strongDot = 1;
            }
            else {
                SetTextColor(hDC, COLOR_GRAY);
            }

            TextOutW(hDC, dotX, dotY, L".", 1);

            if (strongDot == 1) {
                TextOutW(hDC, dotX + 1, dotY, L".", 1);
                TextOutW(hDC, dotX, dotY + 1, L".", 1);
            }
        }
    }
}

void DrawGame(HDC hDC, RECT rt)
{
    HFONT boardFont;
    HFONT textFont;
    HFONT oldFont;
    int boardLeft;
    int boardTop;
    int boardAreaWidth;
    int boardAreaHeight;
    int cellSize;

    if (gameMode == MODE_PLAY) {
        boardAreaWidth = rt.right - 60;
        boardAreaHeight = rt.bottom - 60;
    }
    else {
        // 입력 화면에서는 보드 오른쪽에 좌표 입력 안내를 함께 보여 준다.
        // 그래서 보드가 차지할 가로 공간을 조금 줄여 오른쪽 여백을 확보한다.
        boardAreaWidth = rt.right - 380;
        boardAreaHeight = rt.bottom - 80;
    }

    if (boardAreaWidth < 300) {
        boardAreaWidth = rt.right - 60;
    }

    if (boardAreaHeight < 300) {
        boardAreaHeight = rt.bottom - 60;
    }

    cellSize = boardAreaWidth / boardSize;
    if (boardAreaHeight / boardSize < cellSize) {
        cellSize = boardAreaHeight / boardSize;
    }

    if (cellSize < 10) {
        cellSize = 10;
    }

    if (cellSize > 24) {
        cellSize = 24;
    }

    if (gameMode == MODE_PLAY) {
        boardLeft = (rt.right - boardSize * cellSize) / 2;
        boardTop = (rt.bottom - boardSize * cellSize) / 2;
    }
    else {
        boardLeft = 30;
        boardTop = 40;
    }

    if (boardLeft < 20) {
        boardLeft = 20;
    }

    if (boardTop < 20) {
        boardTop = 20;
    }

    boardFont = CreateFontW(cellSize + 6, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_MODERN, L"Consolas");

    SetBkMode(hDC, TRANSPARENT);

    // 좌표 입력이 끝난 결과 화면에서는 보드 외의 안내문, 상태문, 도움말을 출력하지 않는다.
    // 명령어를 눌러 이동/확대/축소해도 화면에는 항상 보드만 다시 그려진다.
    oldFont = (HFONT)SelectObject(hDC, boardFont);
    DrawBoard(hDC, boardLeft, boardTop, cellSize);

    if (gameMode == MODE_PLAY) {
        SelectObject(hDC, oldFont);
        DeleteObject(boardFont);
        return;
    }

    SelectObject(hDC, oldFont);
    DeleteObject(boardFont);

    textFont = CreateFontW(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_SWISS, L"Malgun Gothic");

    oldFont = (HFONT)SelectObject(hDC, textFont);
    SetTextColor(hDC, COLOR_TEXT);
    DrawInputGuide(hDC, boardLeft + boardSize * cellSize + 35, 70);
    DrawMessage(hDC, boardLeft + boardSize * cellSize + 35, 158);
    DrawTextLine(hDC, boardLeft + boardSize * cellSize + 35, 208, L"입력 형식");
    DrawTextLine(hDC, boardLeft + boardSize * cellSize + 35, 234, L"1번 도형: x1 y1 x2 y2");
    DrawTextLine(hDC, boardLeft + boardSize * cellSize + 35, 260, L"2번 도형: x1 y1 x2 y2");

    SelectObject(hDC, oldFont);
    DeleteObject(textFont);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
    HWND hWnd;
    MSG Message;
    WNDCLASSEX WndClass;

    g_hInst = hInstance;

    // 첨부된 기본 WinAPI 예제처럼 윈도우 클래스를 등록한다.
    // 이 클래스 이름으로 CreateWindow를 호출하면 WndProc이 메시지를 받는다.
    WndClass.cbSize = sizeof(WndClass);
    WndClass.style = CS_HREDRAW | CS_VREDRAW;
    WndClass.lpfnWndProc = (WNDPROC)WndProc;
    WndClass.cbClsExtra = 0;
    WndClass.cbWndExtra = 0;
    WndClass.hInstance = hInstance;
    WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    WndClass.lpszMenuName = NULL;
    WndClass.lpszClassName = lpszClass;
    WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    RegisterClassEx(&WndClass);

    hWnd = CreateWindow(lpszClass, lpszWindowName, WS_OVERLAPPEDWINDOW,
        100, 80, 1040, 820, NULL, (HMENU)NULL, hInstance, NULL);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // Windows 프로그램은 메시지 루프가 계속 돌면서 키보드, 그리기, 종료 메시지를 처리한다.
    while (GetMessage(&Message, 0, 0, 0)) {
        TranslateMessage(&Message);
        DispatchMessage(&Message);
    }

    return (int)Message.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hDC;
    HDC mDC;
    RECT rt;
    HBITMAP hBitmap;
    HBITMAP oldBitmap;

    switch (uMsg) {
    case WM_CREATE:
        ResetAll();
        SetFocus(hWnd);
        break;

    case WM_CHAR:
        // 입력 상태에서는 도형 하나의 좌표 4개와 Enter를 처리하고,
        // 플레이 상태에서는 과제 명령어 한 글자를 즉시 처리한다.
        if (gameMode == MODE_INPUT) {
            ProcessInputChar(wParam);
        }
        else {
            ProcessPlayCommand(hWnd, wParam);
        }

        InvalidateRect(hWnd, NULL, FALSE);
        break;

    case WM_PAINT:
        GetClientRect(hWnd, &rt);
        hDC = BeginPaint(hWnd, &ps);

        // 참고 코드의 방식처럼 메모리 DC에 먼저 그린 뒤 BitBlt로 한 번에 복사한다.
        // 이렇게 하면 화면을 다시 그릴 때 깜빡임이 줄어든다.
        mDC = CreateCompatibleDC(hDC);
        hBitmap = CreateCompatibleBitmap(hDC, rt.right, rt.bottom);
        oldBitmap = (HBITMAP)SelectObject(mDC, hBitmap);

        FillRect(mDC, &rt, (HBRUSH)GetStockObject(WHITE_BRUSH));
        DrawGame(mDC, rt);

        BitBlt(hDC, 0, 0, rt.right, rt.bottom, mDC, 0, 0, SRCCOPY);

        SelectObject(mDC, oldBitmap);
        DeleteObject(hBitmap);
        DeleteDC(mDC);
        EndPaint(hWnd, &ps);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
