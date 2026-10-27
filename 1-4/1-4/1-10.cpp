#include <GL/glew.h>
#include <GL/glfw3.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

// 프로젝트 속성 > 링커 > 명령줄: opengl32.lib glew32.lib glfw3.lib
constexpr int WINDOW_WIDTH = 1080;
constexpr int WINDOW_HEIGHT = 720;
constexpr int LEFT_AREA_WIDTH = 440;
constexpr int BOARD_COUNT = 5;

constexpr float PIECE_SIZE = 40.0f;
constexpr float RIGHT_TRIANGLE_HEIGHT = 80.0f;
constexpr float SNAP_DISTANCE = 28.0f;

enum class ShapeNumber
{
    Square = 1,
    EquilateralUp,
    EquilateralRight,
    EquilateralDown,
    EquilateralLeft,
    RightTopRight,
    RightBottomLeft
};

enum class BoardType
{
    FourSquares,
    FourTriangles,
    DiagonalRectangle,
    House,
    Arrow
};

struct Point
{
    float x = 0.0f;
    float y = 0.0f;
};

struct Color
{
    float red = 0.0f;
    float green = 0.0f;
    float blue = 0.0f;
};

struct Piece
{
    ShapeNumber shapeNumber = ShapeNumber::Square;
    Point position;
    Color color;
    int boardIndex = -1;
    int slotIndex = -1;
};

struct TargetSlot
{
    ShapeNumber shapeNumber = ShapeNumber::Square;
    int pieceIndex = -1;
};

struct Board
{
    BoardType type = BoardType::FourSquares;
    std::vector<TargetSlot> slots;
    bool completed = false;
};

struct DragState
{
    int selectedPieceIndex = -1;
    Point mouseOffset;
};

struct Vertex
{
    float x;
    float y;
    float red;
    float green;
    float blue;
};

std::vector<Piece> pieces;
std::vector<Board> boards;
DragState dragState;

std::mt19937 randomGenerator(std::random_device{}());
GLuint shaderProgram = 0;
GLuint vao = 0;
GLuint vbo = 0;
GLint windowSizeLocation = -1;

float EquilateralHeight()
{
    return PIECE_SIZE * std::sqrt(3.0f) / 2.0f;
}

Color GetShapeColor(ShapeNumber shapeNumber)
{
    switch (shapeNumber)
    {
    case ShapeNumber::Square:
        return { 0.20f, 0.55f, 0.92f };
    case ShapeNumber::EquilateralUp:
    case ShapeNumber::EquilateralRight:
    case ShapeNumber::EquilateralDown:
    case ShapeNumber::EquilateralLeft:
        return { 0.18f, 0.73f, 0.47f };
    case ShapeNumber::RightTopRight:
    case ShapeNumber::RightBottomLeft:
        return { 0.98f, 0.59f, 0.20f };
    }

    return { 0.50f, 0.50f, 0.50f };
}

std::vector<Point> GetShapeVertices(ShapeNumber shapeNumber, Point position)
{
    const float halfSize = PIECE_SIZE / 2.0f;
    const float halfTriangleHeight = RIGHT_TRIANGLE_HEIGHT / 2.0f;
    std::vector<Point> vertices;

    switch (shapeNumber)
    {
    case ShapeNumber::Square:
        vertices = {
            { -halfSize, -halfSize },
            { halfSize, -halfSize },
            { halfSize, halfSize },
            { -halfSize, halfSize }
        };
        break;

    case ShapeNumber::EquilateralUp:
    case ShapeNumber::EquilateralRight:
    case ShapeNumber::EquilateralDown:
    case ShapeNumber::EquilateralLeft:
    {
        const float height = EquilateralHeight();
        vertices = {
            { 0.0f, -2.0f * height / 3.0f },
            { -halfSize, height / 3.0f },
            { halfSize, height / 3.0f }
        };

        int quarterTurns = 0;
        if (shapeNumber == ShapeNumber::EquilateralRight)
            quarterTurns = 1;
        else if (shapeNumber == ShapeNumber::EquilateralDown)
            quarterTurns = 2;
        else if (shapeNumber == ShapeNumber::EquilateralLeft)
            quarterTurns = 3;

        for (Point& vertex : vertices)
        {
            for (int turn = 0; turn < quarterTurns; ++turn)
            {
                const float previousX = vertex.x;
                vertex.x = -vertex.y;
                vertex.y = previousX;
            }
        }
        break;
    }

    case ShapeNumber::RightTopRight:
        vertices = {
            { -halfSize, -halfTriangleHeight },
            { halfSize, -halfTriangleHeight },
            { halfSize, halfTriangleHeight }
        };
        break;

    case ShapeNumber::RightBottomLeft:
        vertices = {
            { -halfSize, -halfTriangleHeight },
            { halfSize, halfTriangleHeight },
            { -halfSize, halfTriangleHeight }
        };
        break;
    }

    for (Point& vertex : vertices)
    {
        vertex.x += position.x;
        vertex.y += position.y;
    }

    return vertices;
}

Point GetBoardPosition(int boardIndex)
{
    const int column = boardIndex % 2;
    const int row = boardIndex / 2;
    return { 610.0f + column * 285.0f, 140.0f + row * 220.0f };
}

Point GetSlotPosition(int boardIndex, int slotIndex)
{
    const Board& board = boards[boardIndex];
    const float height = EquilateralHeight();
    Point offset;

    switch (board.type)
    {
    case BoardType::FourSquares:
    {
        const float halfSpacing = (PIECE_SIZE + 4.0f) / 2.0f;
        offset.x = slotIndex % 2 == 0 ? -halfSpacing : halfSpacing;
        offset.y = slotIndex < 2 ? -halfSpacing : halfSpacing;
        break;
    }

    case BoardType::FourTriangles:
        if (slotIndex == 0)
            offset.y = -2.0f * height / 3.0f;
        else if (slotIndex == 1)
            offset.x = -2.0f * height / 3.0f;
        else if (slotIndex == 2)
            offset.y = 2.0f * height / 3.0f;
        else
            offset.x = 2.0f * height / 3.0f;
        break;

    case BoardType::DiagonalRectangle:
        // 두 직각삼각형이 같은 외접 사각형을 반씩 차지한다.
        break;

    case BoardType::House:
        if (slotIndex == 0)
            offset.y = PIECE_SIZE / 2.0f;
        else
            offset.y = -height / 3.0f;
        break;

    case BoardType::Arrow:
        if (slotIndex == 0)
            offset.x = -PIECE_SIZE / 2.0f;
        else
            offset.x = height / 3.0f;
        break;
    }

    const Point boardPosition = GetBoardPosition(boardIndex);
    return { boardPosition.x + offset.x, boardPosition.y + offset.y };
}

Point GetPiecePosition(int pieceIndex)
{
    const Piece& piece = pieces[pieceIndex];
    if (piece.boardIndex >= 0)
        return GetSlotPosition(piece.boardIndex, piece.slotIndex);

    return piece.position;
}

void CreateBoards()
{
    boards.clear();

    // 과제 그림: 작은 사각형 네 개
    boards.push_back({ BoardType::FourSquares, {
        { ShapeNumber::Square }, { ShapeNumber::Square },
        { ShapeNumber::Square }, { ShapeNumber::Square }
    } });

    // 과제 그림: 중심을 향한 정삼각형 네 개
    boards.push_back({ BoardType::FourTriangles, {
        { ShapeNumber::EquilateralDown },
        { ShapeNumber::EquilateralRight },
        { ShapeNumber::EquilateralUp },
        { ShapeNumber::EquilateralLeft }
    } });

    // 과제 그림: 대각선으로 나뉜 사각형
    boards.push_back({ BoardType::DiagonalRectangle, {
        { ShapeNumber::RightTopRight },
        { ShapeNumber::RightBottomLeft }
    } });

    // 직접 추가한 모양판 두 개
    boards.push_back({ BoardType::House, {
        { ShapeNumber::Square },
        { ShapeNumber::EquilateralUp }
    } });

    boards.push_back({ BoardType::Arrow, {
        { ShapeNumber::Square },
        { ShapeNumber::EquilateralRight }
    } });
}

void CreatePieces()
{
    pieces.clear();

    // 필요한 조각을 먼저 만들면 어떤 랜덤 결과에서도 완성할 수 있다.
    for (const Board& board : boards)
    {
        for (const TargetSlot& slot : board.slots)
        {
            Piece piece;
            piece.shapeNumber = slot.shapeNumber;
            piece.color = GetShapeColor(piece.shapeNumber);
            pieces.push_back(piece);
        }
    }

    const std::array<ShapeNumber, 7> shapeNumbers = {
        ShapeNumber::Square,
        ShapeNumber::EquilateralUp,
        ShapeNumber::EquilateralRight,
        ShapeNumber::EquilateralDown,
        ShapeNumber::EquilateralLeft,
        ShapeNumber::RightTopRight,
        ShapeNumber::RightBottomLeft
    };
    std::uniform_int_distribution<int> extraCount(0, 2);

    for (ShapeNumber shapeNumber : shapeNumbers)
    {
        const int count = extraCount(randomGenerator);
        for (int i = 0; i < count; ++i)
        {
            Piece piece;
            piece.shapeNumber = shapeNumber;
            piece.color = GetShapeColor(shapeNumber);
            pieces.push_back(piece);
        }
    }
}

void PlacePiecesRandomly()
{
    std::vector<Point> positions;
    for (int row = 0; row < 8; ++row)
    {
        for (int column = 0; column < 6; ++column)
            positions.push_back({ 45.0f + column * 70.0f,
                                  70.0f + row * 80.0f });
    }

    std::shuffle(positions.begin(), positions.end(), randomGenerator);
    std::uniform_real_distribution<float> jitter(-5.0f, 5.0f);

    assert(pieces.size() <= positions.size());
    for (size_t i = 0; i < pieces.size(); ++i)
    {
        pieces[i].position = {
            positions[i].x + jitter(randomGenerator),
            positions[i].y + jitter(randomGenerator)
        };
    }
}

void ValidateGameState()
{
#ifndef NDEBUG
    assert(boards.size() == BOARD_COUNT);

    for (size_t boardIndex = 0; boardIndex < boards.size(); ++boardIndex)
    {
        const Board& board = boards[boardIndex];
        bool allFilled = true;

        for (size_t slotIndex = 0; slotIndex < board.slots.size(); ++slotIndex)
        {
            const TargetSlot& slot = board.slots[slotIndex];
            if (slot.pieceIndex < 0)
            {
                allFilled = false;
                continue;
            }

            assert(slot.pieceIndex < static_cast<int>(pieces.size()));
            const Piece& piece = pieces[slot.pieceIndex];
            assert(piece.shapeNumber == slot.shapeNumber);
            assert(piece.boardIndex == static_cast<int>(boardIndex));
            assert(piece.slotIndex == static_cast<int>(slotIndex));
        }

        assert(board.completed == allFilled);
    }

    for (const Piece& piece : pieces)
    {
        if (piece.boardIndex < 0)
        {
            assert(piece.slotIndex == -1);
            continue;
        }

        assert(piece.boardIndex < static_cast<int>(boards.size()));
        const Board& board = boards[piece.boardIndex];
        assert(piece.slotIndex >= 0);
        assert(piece.slotIndex < static_cast<int>(board.slots.size()));
    }
#endif
}

void ResetGame()
{
    dragState = {};
    CreateBoards();
    CreatePieces();
    PlacePiecesRandomly();
    ValidateGameState();
    glfwSetWindowTitle(glfwGetCurrentContext(), "1-10 Shape Puzzle | R: Reset  Q: Quit");
}

float CrossProduct(Point a, Point b, Point point)
{
    return (b.x - a.x) * (point.y - a.y)
         - (b.y - a.y) * (point.x - a.x);
}

bool IsPointInsidePiece(int pieceIndex, Point mousePosition)
{
    const std::vector<Point> vertices = GetShapeVertices(
        pieces[pieceIndex].shapeNumber,
        GetPiecePosition(pieceIndex));

    bool hasPositive = false;
    bool hasNegative = false;
    for (size_t i = 0; i < vertices.size(); ++i)
    {
        const float cross = CrossProduct(
            vertices[i], vertices[(i + 1) % vertices.size()], mousePosition);
        hasPositive |= cross > 0.0f;
        hasNegative |= cross < 0.0f;
        if (hasPositive && hasNegative)
            return false;
    }

    return true;
}

bool IsPieceLocked(int pieceIndex)
{
    const int boardIndex = pieces[pieceIndex].boardIndex;
    return boardIndex >= 0 && boards[boardIndex].completed;
}

int FindTopPiece(Point mousePosition)
{
    // 나중에 그린 조각이 앞에 보이므로 역순으로 검사한다.
    for (int i = static_cast<int>(pieces.size()) - 1; i >= 0; --i)
    {
        if (!IsPieceLocked(i) && IsPointInsidePiece(i, mousePosition))
            return i;
    }

    return -1;
}

Point ClampPiecePosition(ShapeNumber shapeNumber, Point position)
{
    const std::vector<Point> vertices = GetShapeVertices(shapeNumber, position);
    float minimumX = vertices[0].x;
    float maximumX = vertices[0].x;
    float minimumY = vertices[0].y;
    float maximumY = vertices[0].y;

    for (const Point& vertex : vertices)
    {
        minimumX = std::min(minimumX, vertex.x);
        maximumX = std::max(maximumX, vertex.x);
        minimumY = std::min(minimumY, vertex.y);
        maximumY = std::max(maximumY, vertex.y);
    }

    if (minimumX < 0.0f)
        position.x -= minimumX;
    if (maximumX > WINDOW_WIDTH)
        position.x -= maximumX - WINDOW_WIDTH;
    if (minimumY < 0.0f)
        position.y -= minimumY;
    if (maximumY > WINDOW_HEIGHT)
        position.y -= maximumY - WINDOW_HEIGHT;

    return position;
}

void DetachPiece(int pieceIndex)
{
    Piece& piece = pieces[pieceIndex];
    if (piece.boardIndex < 0)
        return;

    piece.position = GetPiecePosition(pieceIndex);
    Board& board = boards[piece.boardIndex];
    board.slots[piece.slotIndex].pieceIndex = -1;
    board.completed = false;
    piece.boardIndex = -1;
    piece.slotIndex = -1;
    ValidateGameState();
}

bool FindMatchingSlot(int pieceIndex, int& matchingBoard, int& matchingSlot)
{
    const Piece& piece = pieces[pieceIndex];
    const float maximumDistanceSquared = SNAP_DISTANCE * SNAP_DISTANCE;
    float closestDistanceSquared = maximumDistanceSquared;
    matchingBoard = -1;
    matchingSlot = -1;

    for (int boardIndex = 0; boardIndex < static_cast<int>(boards.size()); ++boardIndex)
    {
        const Board& board = boards[boardIndex];
        if (board.completed)
            continue;

        for (int slotIndex = 0; slotIndex < static_cast<int>(board.slots.size()); ++slotIndex)
        {
            const TargetSlot& slot = board.slots[slotIndex];
            if (slot.shapeNumber != piece.shapeNumber || slot.pieceIndex >= 0)
                continue;

            const Point target = GetSlotPosition(boardIndex, slotIndex);
            const float differenceX = piece.position.x - target.x;
            const float differenceY = piece.position.y - target.y;
            const float distanceSquared = differenceX * differenceX
                                        + differenceY * differenceY;

            if (distanceSquared <= closestDistanceSquared)
            {
                closestDistanceSquared = distanceSquared;
                matchingBoard = boardIndex;
                matchingSlot = slotIndex;
            }
        }
    }

    return matchingBoard >= 0;
}

void CheckBoardCompleted(int boardIndex)
{
    Board& board = boards[boardIndex];
    board.completed = std::all_of(board.slots.begin(), board.slots.end(),
        [](const TargetSlot& slot) { return slot.pieceIndex >= 0; });

    const bool allBoardsCompleted = std::all_of(boards.begin(), boards.end(),
        [](const Board& item) { return item.completed; });
    if (allBoardsCompleted)
        glfwSetWindowTitle(glfwGetCurrentContext(), "1-10 Complete! | R: Reset  Q: Quit");
}

void AttachPiece(int pieceIndex, int boardIndex, int slotIndex)
{
    Piece& piece = pieces[pieceIndex];
    piece.boardIndex = boardIndex;
    piece.slotIndex = slotIndex;
    boards[boardIndex].slots[slotIndex].pieceIndex = pieceIndex;
    CheckBoardCompleted(boardIndex);
    ValidateGameState();
}

bool TryAttachPiece(int pieceIndex)
{
    int boardIndex = -1;
    int slotIndex = -1;
    if (!FindMatchingSlot(pieceIndex, boardIndex, slotIndex))
        return false;

    AttachPiece(pieceIndex, boardIndex, slotIndex);
    return true;
}

void RunPlacementSelfCheck()
{
#ifndef NDEBUG
    CreateBoards();
    CreatePieces();

    // 필요한 조각만으로 모든 모양판을 완성할 수 있는지 확인한다.
    for (int boardIndex = 0; boardIndex < static_cast<int>(boards.size()); ++boardIndex)
    {
        for (int slotIndex = 0;
             slotIndex < static_cast<int>(boards[boardIndex].slots.size());
             ++slotIndex)
        {
            const ShapeNumber requiredShape = boards[boardIndex].slots[slotIndex].shapeNumber;
            auto matchingPiece = std::find_if(pieces.begin(), pieces.end(),
                [requiredShape](const Piece& piece)
                {
                    return piece.boardIndex < 0 && piece.shapeNumber == requiredShape;
                });

            assert(matchingPiece != pieces.end());
            const int pieceIndex = static_cast<int>(matchingPiece - pieces.begin());
            pieces[pieceIndex].position = GetSlotPosition(boardIndex, slotIndex);
            assert(TryAttachPiece(pieceIndex));
        }
    }

    for (const Board& board : boards)
    {
        assert(board.completed);
        for (const TargetSlot& slot : board.slots)
            assert(IsPieceLocked(slot.pieceIndex));
    }
#endif
}

void MoveSelectedPiece(Point mousePosition)
{
    const int pieceIndex = dragState.selectedPieceIndex;
    if (pieceIndex < 0)
        return;

    Piece& piece = pieces[pieceIndex];
    const Point proposedPosition = {
        mousePosition.x - dragState.mouseOffset.x,
        mousePosition.y - dragState.mouseOffset.y
    };
    piece.position = ClampPiecePosition(piece.shapeNumber, proposedPosition);
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int)
{
    if (button != GLFW_MOUSE_BUTTON_LEFT)
        return;

    double cursorX = 0.0;
    double cursorY = 0.0;
    glfwGetCursorPos(window, &cursorX, &cursorY);
    const Point mousePosition = {
        static_cast<float>(cursorX), static_cast<float>(cursorY)
    };

    if (action == GLFW_PRESS)
    {
        const int pieceIndex = FindTopPiece(mousePosition);
        if (pieceIndex < 0)
            return;

        const Point piecePosition = GetPiecePosition(pieceIndex);
        dragState.mouseOffset = {
            mousePosition.x - piecePosition.x,
            mousePosition.y - piecePosition.y
        };
        DetachPiece(pieceIndex);
        dragState.selectedPieceIndex = pieceIndex;
    }
    else if (action == GLFW_RELEASE && dragState.selectedPieceIndex >= 0)
    {
        MoveSelectedPiece(mousePosition);
        TryAttachPiece(dragState.selectedPieceIndex);
        dragState.selectedPieceIndex = -1;
    }
}

void CursorPositionCallback(GLFWwindow*, double cursorX, double cursorY)
{
    MoveSelectedPiece({ static_cast<float>(cursorX), static_cast<float>(cursorY) });
}

void KeyCallback(GLFWwindow* window, int key, int, int action, int)
{
    if (action != GLFW_PRESS)
        return;

    if (key == GLFW_KEY_R)
        ResetGame();
    else if (key == GLFW_KEY_Q)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void FramebufferSizeCallback(GLFWwindow*, int width, int height)
{
    glViewport(0, 0, width, height);
}

void AddVertex(std::vector<Vertex>& vertices, Point position, Color color)
{
    vertices.push_back({ position.x, position.y,
                         color.red, color.green, color.blue });
}

void UploadVertices(const std::vector<Vertex>& vertices)
{
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
                 vertices.data(), GL_DYNAMIC_DRAW);
}

void DrawFilledPolygon(const std::vector<Point>& points, Color color)
{
    std::vector<Vertex> vertices;
    for (size_t i = 1; i + 1 < points.size(); ++i)
    {
        AddVertex(vertices, points[0], color);
        AddVertex(vertices, points[i], color);
        AddVertex(vertices, points[i + 1], color);
    }

    UploadVertices(vertices);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size()));
}

void DrawOutline(const std::vector<Point>& points, Color color)
{
    std::vector<Vertex> vertices;
    for (const Point& point : points)
        AddVertex(vertices, point, color);

    UploadVertices(vertices);
    glDrawArrays(GL_LINE_LOOP, 0, static_cast<GLsizei>(vertices.size()));
}

void DrawLine(Point start, Point end, Color color)
{
    std::vector<Vertex> vertices;
    AddVertex(vertices, start, color);
    AddVertex(vertices, end, color);
    UploadVertices(vertices);
    glDrawArrays(GL_LINES, 0, 2);
}

void DrawBoard(int boardIndex)
{
    const Board& board = boards[boardIndex];
    const Point center = GetBoardPosition(boardIndex);
    const std::vector<Point> panel = {
        { center.x - 125.0f, center.y - 95.0f },
        { center.x + 125.0f, center.y - 95.0f },
        { center.x + 125.0f, center.y + 95.0f },
        { center.x - 125.0f, center.y + 95.0f }
    };

    DrawFilledPolygon(panel, board.completed
        ? Color{ 0.85f, 0.97f, 0.88f }
        : Color{ 0.94f, 0.95f, 0.97f });
    DrawOutline(panel, board.completed
        ? Color{ 0.20f, 0.65f, 0.35f }
        : Color{ 0.73f, 0.76f, 0.81f });

    for (int slotIndex = 0; slotIndex < static_cast<int>(board.slots.size()); ++slotIndex)
    {
        const TargetSlot& slot = board.slots[slotIndex];
        if (slot.pieceIndex >= 0)
            continue;

        const Color pieceColor = GetShapeColor(slot.shapeNumber);
        const Color guideColor = {
            (pieceColor.red + 1.0f) / 2.0f,
            (pieceColor.green + 1.0f) / 2.0f,
            (pieceColor.blue + 1.0f) / 2.0f
        };
        DrawOutline(GetShapeVertices(slot.shapeNumber,
                    GetSlotPosition(boardIndex, slotIndex)), guideColor);
    }
}

void DrawPiece(int pieceIndex)
{
    const Piece& piece = pieces[pieceIndex];
    const std::vector<Point> vertices = GetShapeVertices(
        piece.shapeNumber, GetPiecePosition(pieceIndex));

    DrawFilledPolygon(vertices, piece.color);
    DrawOutline(vertices, pieceIndex == dragState.selectedPieceIndex
        ? Color{ 0.98f, 0.88f, 0.20f }
        : Color{ 0.20f, 0.24f, 0.30f });
}

void DrawScene()
{
    glUseProgram(shaderProgram);
    glUniform2f(windowSizeLocation,
                static_cast<float>(WINDOW_WIDTH),
                static_cast<float>(WINDOW_HEIGHT));
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    DrawLine({ static_cast<float>(LEFT_AREA_WIDTH), 0.0f },
             { static_cast<float>(LEFT_AREA_WIDTH), static_cast<float>(WINDOW_HEIGHT) },
             { 0.72f, 0.75f, 0.80f });

    for (int boardIndex = 0; boardIndex < static_cast<int>(boards.size()); ++boardIndex)
        DrawBoard(boardIndex);

    for (int pieceIndex = 0; pieceIndex < static_cast<int>(pieces.size()); ++pieceIndex)
    {
        if (pieceIndex != dragState.selectedPieceIndex)
            DrawPiece(pieceIndex);
    }

    if (dragState.selectedPieceIndex >= 0)
        DrawPiece(dragState.selectedPieceIndex);
}

std::string ReadShaderFile(const char* fileName)
{
    std::ifstream file(fileName);
    if (!file)
    {
        file.clear();
        file.open(std::filesystem::path(__FILE__).parent_path() / fileName);
    }

    if (!file)
    {
        std::cerr << "Shader file not found: " << fileName << std::endl;
        return {};
    }

    std::stringstream source;
    source << file.rdbuf();
    return source.str();
}

GLuint CompileShader(GLenum shaderType, const std::string& source)
{
    if (source.empty())
        return 0;

    const char* sourcePointer = source.c_str();
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &sourcePointer, nullptr);
    glCompileShader(shader);

    GLint compiled = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled)
    {
        char log[1024] = {};
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        std::cerr << "Shader compilation failed: " << log << std::endl;
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

GLuint CreateShaderProgram()
{
    GLuint vertexShader = CompileShader(GL_VERTEX_SHADER,
                                        ReadShaderFile("vertex.glsl"));
    GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER,
                                          ReadShaderFile("fragment.glsl"));
    if (!vertexShader || !fragmentShader)
    {
        if (vertexShader)
            glDeleteShader(vertexShader);
        if (fragmentShader)
            glDeleteShader(fragmentShader);
        return 0;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    GLint linked = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked)
    {
        char log[1024] = {};
        glGetProgramInfoLog(program, sizeof(log), nullptr, log);
        std::cerr << "Shader linking failed: " << log << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    if (!linked)
    {
        glDeleteProgram(program);
        return 0;
    }

    return program;
}

int main()
{
    // --- GLFW 초기화
    if (!glfwInit())
    {
        std::cerr << "GLFW 초기화 실패!" << std::endl;
        return -1;
    }

    // --- OpenGL 3.3 Core Profile과 고정 크기 창 생성
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(
        WINDOW_WIDTH, WINDOW_HEIGHT, "1-10 Shape Puzzle", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "윈도우 생성 실패!" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // --- GLEW 초기화
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "GLEW 초기화 실패!" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // --- 입력 콜백과 실제 프레임버퍼 크기 설정
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetCursorPosCallback(window, CursorPositionCallback);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);

    int framebufferWidth = 0;
    int framebufferHeight = 0;
    glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
    glViewport(0, 0, framebufferWidth, framebufferHeight);

    // --- 셰이더 프로그램과 정점 버퍼 설정
    shaderProgram = CreateShaderProgram();
    if (!shaderProgram)
    {
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    windowSizeLocation = glGetUniformLocation(shaderProgram, "windowSize");

    RunPlacementSelfCheck();
    ResetGame();

    // --- 메인 루프
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.98f, 0.98f, 0.99f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        DrawScene();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // --- 종료 처리
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
