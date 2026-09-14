#include <windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <time.h>
#include <wchar.h>
#include <wctype.h>

#define MIN_BOARD_SIZE 3
#define MAX_BOARD_SIZE 6
#define MAX_CELL_COUNT 36
#define INPUT_MAX 64
#define HIDE_TIMER_ID 1
#define HINT_TIMER_ID 2
#define HIDE_DELAY 1000
#define HINT_DELAY 2000
#define JOKER L'#'

HINSTANCE g_hInst;
const WCHAR lpszClass[] = L"My Window Class";
const WCHAR lpszWindowName[] = L"1-4 Card Matching Game";

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

enum GameMode {
	MODE_SIZE_INPUT,
	MODE_PLAYING,
	MODE_WAIT_HIDE,
	MODE_GAME_OVER
};

typedef struct Card {
	WCHAR value;
	BOOL revealed;
	BOOL matched;
} Card;

Card board[MAX_BOARD_SIZE][MAX_BOARD_SIZE];
POINT selectedCells[2];
int selectedCount = 0;
int boardRows = 0;
int boardCols = 0;
int turnCount = 0;
int maxTurns = 0;
int score = 0;
int parsedRow = 0;
int parsedCol = 0;
BOOL hintMode = FALSE;
enum GameMode mode = MODE_SIZE_INPUT;
WCHAR inputText[INPUT_MAX] = L"";
int inputLength = 0;

int MinInt(int a, int b)
{
	return a < b ? a : b;
}

int MaxInt(int a, int b)
{
	return a > b ? a : b;
}

void ClearInput()
{
	inputLength = 0;
	inputText[0] = L'\0';
}

void AddInputChar(WCHAR ch)
{
	if (inputLength < INPUT_MAX - 1) {
		inputText[inputLength] = ch;
		inputLength++;
		inputText[inputLength] = L'\0';
	}
}

void BackspaceInput()
{
	if (inputLength > 0) {
		inputLength--;
		inputText[inputLength] = L'\0';
	}
}

void CopyTrimLower(const WCHAR src[], WCHAR dest[], int maxLength)
{
	int start = 0;
	int end;
	int i;
	int length;

	while (src[start] != L'\0' && iswspace(src[start])) {
		start++;
	}

	end = (int)wcslen(src);
	while (end > start && iswspace(src[end - 1])) {
		end--;
	}

	length = end - start;
	if (length >= maxLength) {
		length = maxLength - 1;
	}

	for (i = 0; i < length; ++i) {
		dest[i] = (WCHAR)towlower(src[start + i]);
	}
	dest[length] = L'\0';
}

int ExtractNumbers(const WCHAR text[], int numbers[], int maxCount)
{
	int count = 0;
	int current = -1;
	int i = 0;

	while (text[i] != L'\0') {
		if (iswdigit(text[i])) {
			if (current < 0) {
				current = 0;
			}
			current = current * 10 + (text[i] - L'0');
		}
		else if (current >= 0) {
			if (count < maxCount) {
				numbers[count] = current;
				count++;
			}
			current = -1;
		}
		i++;
	}

	if (current >= 0 && count < maxCount) {
		numbers[count] = current;
		count++;
	}

	return count;
}

void ClearSelection()
{
	int i;
	int row;
	int col;

	for (i = 0; i < selectedCount; ++i) {
		row = selectedCells[i].y;
		col = selectedCells[i].x;
		if (!board[row][col].matched) {
			board[row][col].revealed = FALSE;
		}
	}

	selectedCount = 0;
}

BOOL IsSelectedCell(int row, int col)
{
	int i;

	for (i = 0; i < selectedCount; ++i) {
		if (selectedCells[i].y == row && selectedCells[i].x == col) {
			return TRUE;
		}
	}

	return FALSE;
}

int CountMatchedCells()
{
	int count = 0;
	int row;
	int col;

	for (row = 0; row < boardRows; ++row) {
		for (col = 0; col < boardCols; ++col) {
			if (board[row][col].matched) {
				count++;
			}
		}
	}

	return count;
}

int OpenLastJokerIfNeeded(int matchedCount)
{
	int cellCount = boardRows * boardCols;
	int row;
	int col;

	if (matchedCount != cellCount - 1) {
		return matchedCount;
	}

	for (row = 0; row < boardRows; ++row) {
		for (col = 0; col < boardCols; ++col) {
			if (!board[row][col].matched && board[row][col].value == JOKER) {
				board[row][col].matched = TRUE;
				board[row][col].revealed = TRUE;
				return cellCount;
			}
		}
	}

	return matchedCount;
}

void CheckGameEnd(HWND hWnd)
{
	int matchedCount;

	if (boardRows == 0 || boardCols == 0) {
		return;
	}

	matchedCount = OpenLastJokerIfNeeded(CountMatchedCells());
	if (matchedCount == boardRows * boardCols || turnCount >= maxTurns) {
		mode = MODE_GAME_OVER;
		KillTimer(hWnd, HIDE_TIMER_ID);
		KillTimer(hWnd, HINT_TIMER_ID);
		hintMode = FALSE;
	}
}

void ShuffleCards(WCHAR cards[], int count)
{
	int i;
	int j;
	WCHAR temp;

	for (i = count - 1; i > 0; --i) {
		j = rand() % (i + 1);
		temp = cards[i];
		cards[i] = cards[j];
		cards[j] = temp;
	}
}

void StartGame(HWND hWnd, int width, int height)
{
	WCHAR cards[MAX_CELL_COUNT];
	int cellCount = width * height;
	int pairCount = cellCount / 2;
	int cardCount = 0;
	int row;
	int col;
	int i;
	int index = 0;

	KillTimer(hWnd, HIDE_TIMER_ID);
	KillTimer(hWnd, HINT_TIMER_ID);

	boardCols = width;
	boardRows = height;
	turnCount = 0;
	maxTurns = width * height;
	score = 0;
	selectedCount = 0;
	hintMode = FALSE;
	mode = MODE_PLAYING;
	ClearInput();

	for (i = 0; i < pairCount; ++i) {
		cards[cardCount] = (WCHAR)(L'a' + i);
		cardCount++;
		cards[cardCount] = (WCHAR)(L'a' + i);
		cardCount++;
	}

	if (cellCount % 2 == 1) {
		cards[cardCount] = JOKER;
		cardCount++;
	}

	ShuffleCards(cards, cardCount);

	for (row = 0; row < height; ++row) {
		for (col = 0; col < width; ++col) {
			board[row][col].value = cards[index];
			board[row][col].revealed = FALSE;
			board[row][col].matched = FALSE;
			index++;
		}
	}
}

void ReturnToSizeInput(HWND hWnd)
{
	KillTimer(hWnd, HIDE_TIMER_ID);
	KillTimer(hWnd, HINT_TIMER_ID);

	boardRows = 0;
	boardCols = 0;
	turnCount = 0;
	maxTurns = 0;
	score = 0;
	selectedCount = 0;
	hintMode = FALSE;
	mode = MODE_SIZE_INPUT;
	ClearInput();
}

BOOL ParseCell(const WCHAR text[])
{
	int numbers[4];
	int numberCount;
	int i = 0;
	WCHAR columnLetter = 0;

	numberCount = ExtractNumbers(text, numbers, 4);

	while (text[i] != L'\0') {
		if (text[i] >= L'a' && text[i] <= L'z') {
			columnLetter = text[i];
			break;
		}
		i++;
	}

	if (columnLetter != 0) {
		if (numberCount == 0) {
			return FALSE;
		}
		parsedRow = numbers[0] - 1;
		parsedCol = columnLetter - L'a';
	}
	else if (numberCount >= 2) {
		parsedRow = numbers[0] - 1;
		parsedCol = numbers[1] - 1;
	}
	else {
		return FALSE;
	}

	if (parsedRow >= 0 && parsedRow < boardRows && parsedCol >= 0 && parsedCol < boardCols) {
		return TRUE;
	}

	return FALSE;
}

void MatchNormalPair(WCHAR value)
{
	int row;
	int col;

	for (row = 0; row < boardRows; ++row) {
		for (col = 0; col < boardCols; ++col) {
			if (board[row][col].value == value) {
				board[row][col].matched = TRUE;
				board[row][col].revealed = TRUE;
			}
		}
	}
}

void ResolveSelectedPair(HWND hWnd)
{
	int firstRow = selectedCells[0].y;
	int firstCol = selectedCells[0].x;
	int secondRow = selectedCells[1].y;
	int secondCol = selectedCells[1].x;
	WCHAR firstValue = board[firstRow][firstCol].value;
	WCHAR secondValue = board[secondRow][secondCol].value;
	WCHAR normalValue;

	turnCount++;

	if (firstValue == JOKER || secondValue == JOKER) {
		normalValue = firstValue == JOKER ? secondValue : firstValue;

		board[firstRow][firstCol].matched = TRUE;
		board[secondRow][secondCol].matched = TRUE;
		board[firstRow][firstCol].revealed = TRUE;
		board[secondRow][secondCol].revealed = TRUE;

		if (normalValue != JOKER) {
			MatchNormalPair(normalValue);
		}

		score += 5;
		selectedCount = 0;
		CheckGameEnd(hWnd);
	}
	else if (firstValue == secondValue) {
		board[firstRow][firstCol].matched = TRUE;
		board[secondRow][secondCol].matched = TRUE;
		board[firstRow][firstCol].revealed = TRUE;
		board[secondRow][secondCol].revealed = TRUE;

		score += 10;
		selectedCount = 0;
		CheckGameEnd(hWnd);
	}
	else {
		score -= 2;
		mode = MODE_WAIT_HIDE;
		SetTimer(hWnd, HIDE_TIMER_ID, HIDE_DELAY, NULL);
	}
}

void SelectCard(HWND hWnd, int row, int col)
{
	if (mode != MODE_PLAYING) {
		return;
	}

	if (board[row][col].matched) {
		return;
	}

	if (IsSelectedCell(row, col)) {
		return;
	}

	board[row][col].revealed = TRUE;
	selectedCells[selectedCount].x = col;
	selectedCells[selectedCount].y = row;
	selectedCount++;

	if (selectedCount == 2) {
		ResolveSelectedPair(hWnd);
	}
}

void ProcessCommand(HWND hWnd, const WCHAR rawText[])
{
	WCHAR text[INPUT_MAX];
	int numbers[4];
	int numberCount;

	CopyTrimLower(rawText, text, INPUT_MAX);

	if (text[0] == L'\0') {
		return;
	}

	if (wcscmp(text, L"q") == 0) {
		DestroyWindow(hWnd);
		return;
	}

	if (wcscmp(text, L"r") == 0) {
		ReturnToSizeInput(hWnd);
		return;
	}

	if (wcscmp(text, L"h") == 0) {
		if (mode == MODE_PLAYING && boardRows > 0 && boardCols > 0) {
			hintMode = TRUE;
			score -= 3;
			SetTimer(hWnd, HINT_TIMER_ID, HINT_DELAY, NULL);
		}
		return;
	}

	if (mode == MODE_SIZE_INPUT) {
		numberCount = ExtractNumbers(text, numbers, 4);
		if (numberCount >= 2 &&
			numbers[0] >= MIN_BOARD_SIZE && numbers[0] <= MAX_BOARD_SIZE &&
			numbers[1] >= MIN_BOARD_SIZE && numbers[1] <= MAX_BOARD_SIZE) {
			StartGame(hWnd, numbers[0], numbers[1]);
		}
		return;
	}

	if (mode == MODE_GAME_OVER) {
		return;
	}

	if (ParseCell(text)) {
		SelectCard(hWnd, parsedRow, parsedCol);
	}
}

void DrawTextAt(HDC hDC, int x, int y, const WCHAR text[])
{
	TextOutW(hDC, x, y, text, lstrlenW(text));
}

void DrawCenteredText(HDC hDC, RECT rect, const WCHAR text[])
{
	DrawTextW(hDC, text, lstrlenW(text), &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

void DrawGame(HDC hDC, RECT clientRect)
{
	HBRUSH backBrush;
	HFONT inputFont;
	HFONT labelFont;
	HFONT cardFont;
	HFONT oldFont;
	HPEN gridPen;
	HPEN selectPen;
	HPEN oldPen;
	HBRUSH hiddenBrush;
	HBRUSH openBrush;
	HBRUSH matchedBrush;
	HBRUSH oldBrush;
	WCHAR inputLine[INPUT_MAX + 16];
	int boardLeft;
	int boardTop;
	int clientWidth;
	int clientHeight;
	int availableWidth;
	int availableHeight;
	int cellSize;
	int row;
	int col;

	SetBkMode(hDC, TRANSPARENT);

	backBrush = CreateSolidBrush(RGB(247, 249, 252));
	FillRect(hDC, &clientRect, backBrush);
	DeleteObject(backBrush);

	inputFont = CreateFontW(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_SWISS, L"Malgun Gothic");
	oldFont = (HFONT)SelectObject(hDC, inputFont);

	wsprintfW(inputLine, L"Input: %s", inputText);
	SetTextColor(hDC, RGB(25, 34, 46));
	DrawTextAt(hDC, 32, 32, inputLine);

	if (mode == MODE_SIZE_INPUT) {
		SelectObject(hDC, oldFont);
		DeleteObject(inputFont);
		return;
	}

	boardLeft = 78;
	boardTop = 92;
	clientWidth = (int)(clientRect.right - clientRect.left);
	clientHeight = (int)(clientRect.bottom - clientRect.top);
	availableWidth = MaxInt(1, clientWidth - boardLeft - 40);
	availableHeight = MaxInt(1, clientHeight - boardTop - 40);
	cellSize = MinInt(availableWidth / boardCols, availableHeight / boardRows);
	cellSize = MaxInt(44, MinInt(76, cellSize));

	labelFont = CreateFontW(18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_SWISS, L"Malgun Gothic");
	cardFont = CreateFontW(cellSize / 2, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_SWISS, L"Consolas");
	gridPen = CreatePen(PS_SOLID, 2, RGB(56, 68, 84));
	selectPen = CreatePen(PS_SOLID, 4, RGB(20, 120, 230));
	hiddenBrush = CreateSolidBrush(RGB(228, 234, 242));
	openBrush = CreateSolidBrush(RGB(255, 247, 205));
	matchedBrush = CreateSolidBrush(RGB(216, 242, 225));
	oldPen = (HPEN)SelectObject(hDC, gridPen);
	oldBrush = (HBRUSH)SelectObject(hDC, GetStockObject(NULL_BRUSH));

	SelectObject(hDC, labelFont);
	SetTextColor(hDC, RGB(63, 75, 91));

	for (col = 0; col < boardCols; ++col) {
		WCHAR label[4];
		RECT labelRect;

		wsprintfW(label, L"%c", L'a' + col);
		labelRect.left = boardLeft + col * cellSize;
		labelRect.top = boardTop - 30;
		labelRect.right = boardLeft + (col + 1) * cellSize;
		labelRect.bottom = boardTop - 4;
		DrawCenteredText(hDC, labelRect, label);
	}

	for (row = 0; row < boardRows; ++row) {
		WCHAR label[4];
		RECT labelRect;

		wsprintfW(label, L"%d", row + 1);
		labelRect.left = boardLeft - 38;
		labelRect.top = boardTop + row * cellSize;
		labelRect.right = boardLeft - 8;
		labelRect.bottom = boardTop + (row + 1) * cellSize;
		DrawCenteredText(hDC, labelRect, label);
	}

	for (row = 0; row < boardRows; ++row) {
		for (col = 0; col < boardCols; ++col) {
			RECT cellRect;
			WCHAR cardText[2];
			BOOL showCard;
			HBRUSH fillBrush;

			cellRect.left = boardLeft + col * cellSize;
			cellRect.top = boardTop + row * cellSize;
			cellRect.right = cellRect.left + cellSize;
			cellRect.bottom = cellRect.top + cellSize;

			showCard = hintMode || board[row][col].revealed || board[row][col].matched;
			fillBrush = hiddenBrush;

			if (board[row][col].matched) {
				fillBrush = matchedBrush;
			}
			else if (showCard) {
				fillBrush = openBrush;
			}

			FillRect(hDC, &cellRect, fillBrush);
			SelectObject(hDC, IsSelectedCell(row, col) ? selectPen : gridPen);
			Rectangle(hDC, cellRect.left, cellRect.top, cellRect.right, cellRect.bottom);

			cardText[0] = L'*';
			cardText[1] = L'\0';

			if (showCard) {
				if (board[row][col].value == JOKER) {
					cardText[0] = JOKER;
					SetTextColor(hDC, RGB(195, 88, 16));
				}
				else if (board[row][col].matched) {
					cardText[0] = (WCHAR)towupper(board[row][col].value);
					SetTextColor(hDC, RGB(18, 126, 78));
				}
				else {
					cardText[0] = board[row][col].value;
					SetTextColor(hDC, RGB(32, 45, 62));
				}
			}
			else {
				SetTextColor(hDC, RGB(80, 91, 107));
			}

			SelectObject(hDC, cardFont);
			DrawCenteredText(hDC, cellRect, cardText);
		}
	}

	SelectObject(hDC, oldFont);
	SelectObject(hDC, oldPen);
	SelectObject(hDC, oldBrush);
	DeleteObject(inputFont);
	DeleteObject(labelFont);
	DeleteObject(cardFont);
	DeleteObject(gridPen);
	DeleteObject(selectPen);
	DeleteObject(hiddenBrush);
	DeleteObject(openBrush);
	DeleteObject(matchedBrush);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	HWND hWnd;
	MSG Message;
	WNDCLASSEX WndClass;

	g_hInst = hInstance;
	srand((unsigned int)time(NULL));

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
		100, 100, 900, 700, NULL, (HMENU)NULL, hInstance, NULL);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

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
		ReturnToSizeInput(hWnd);
		SetFocus(hWnd);
		break;

	case WM_CHAR:
		if (wParam == VK_RETURN) {
			ProcessCommand(hWnd, inputText);
			ClearInput();
		}
		else if (wParam == VK_BACK) {
			BackspaceInput();
		}
		else if (wParam >= 32 && wParam <= 126) {
			AddInputChar((WCHAR)wParam);
		}
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case WM_PAINT:
		GetClientRect(hWnd, &rt);
		hDC = BeginPaint(hWnd, &ps);
		mDC = CreateCompatibleDC(hDC);
		hBitmap = CreateCompatibleBitmap(hDC, rt.right, rt.bottom);
		oldBitmap = (HBITMAP)SelectObject(mDC, hBitmap);

		DrawGame(mDC, rt);

		BitBlt(hDC, 0, 0, rt.right, rt.bottom, mDC, 0, 0, SRCCOPY);

		SelectObject(mDC, oldBitmap);
		DeleteDC(mDC);
		DeleteObject(hBitmap);
		EndPaint(hWnd, &ps);
		break;

	case WM_TIMER:
		if (wParam == HIDE_TIMER_ID) {
			KillTimer(hWnd, HIDE_TIMER_ID);
			ClearSelection();
			mode = MODE_PLAYING;
			CheckGameEnd(hWnd);
		}
		else if (wParam == HINT_TIMER_ID) {
			KillTimer(hWnd, HINT_TIMER_ID);
			hintMode = FALSE;
		}
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
