#define _CRT_SECURE_NO_WARNINGS

#include "raylib.h"
#include <vector>
#include <ctime>
#include <cstdlib>
#include <string>
#include <algorithm>
#include <fstream>
#include <memory>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <random>
#include <chrono>
using namespace std;

// ---------- Constants & Enums ---------- //
const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 700;
const int FPS = 60;
const int UI_PANEL_WIDTH = 300;

enum MazeSize
{
    SIZE_SMALL = 20,
    SIZE_MEDIUM = 30,
    SIZE_LARGE = 40
};
enum GameState
{
    STATE_HOME,
    STATE_PLAYER_NAME,
    STATE_SIZE_SELECT,
    STATE_DIFFICULTY_SELECT,
    STATE_COUNTDOWN,
    STATE_PLAYING,
    STATE_END,
    STATE_SCOREBOARD,
    STATE_REPLAY
};
enum DifficultyLevel
{
    DIFF_EASY,
    DIFF_MEDIUM,
    DIFF_HARD
};

// ---------- Colors (custom) ---------- //
const Color DARKRED = Color{139, 0, 0, 255};
const Color DARKORANGE = Color{200, 100, 0, 255};
const Color LIGHTBLUE = Color{173, 216, 230, 255};
const Color PANEL_GRAY = Color{240, 240, 245, 255};
const Color MY_ORANGE = Color{255, 165, 0, 255};
const Color GREENDARK = Color{0, 100, 0, 255};
const Color BLUEDARK = Color{0, 0, 139, 255};
const Color UI_BLUE = Color{52, 110, 255, 255};

// ---------- RNG ---------- //
static std::mt19937 rng((unsigned)chrono::system_clock::now().time_since_epoch().count());
int irand(int a, int b) { return std::uniform_int_distribution<int>(a, b)(rng); }

// --------- Music ------------- //
Music backgroundMusic;

// ---------- Data structures ---------- //

// PlayerMove for replay
struct PlayerMove
{
    int x, y;
    float t;
    PlayerMove(int X = 0, int Y = 0, float T = 0) : x(X), y(Y), t(T) {}
};

// LinkedList to store player path
class LinkedListNode
{
public:
    PlayerMove data;
    LinkedListNode *next;
    LinkedListNode(const PlayerMove &d) : data(d), next(nullptr) {}
};
class LinkedList
{
    LinkedListNode *head = nullptr;
    LinkedListNode *tail = nullptr;
    int sz = 0;

public:
    ~LinkedList() { clear(); }
    void add(const PlayerMove &m)
    {
        LinkedListNode *n = new LinkedListNode(m);
        if (!head)
            head = tail = n;
        else
        {
            tail->next = n;
            tail = n;
        }
        ++sz;
    }
    LinkedListNode *getHead() { return head; }
    int getSize() const { return sz; }
    void clear()
    {
        LinkedListNode *p = head;
        while (p)
        {
            LinkedListNode *t = p;
            p = p->next;
            delete t;
        }
        head = tail = nullptr;
        sz = 0;
    }
};

template <typename T>
class Stack
{
    struct Node
    {
        T data;
        Node *next;
        Node(const T &d, Node *n = nullptr) : data(d), next(n) {}
    };
    Node *head;

public:
    Stack() : head(nullptr) {}
    ~Stack() { clear(); }

    bool empty() const { return head == nullptr; }

    void push(const T &value)
    {
        head = new Node(value, head);
    }

    T &top()
    {
        return head->data;
    }

    void pop()
    {
        if (!head)
            return;
        Node *tmp = head;
        head = head->next;
        delete tmp;
    }

    void clear()
    {
        while (head)
        {
            Node *tmp = head;
            head = head->next;
            delete tmp;
        }
    }
};

template <typename T>
class Queue
{
    struct Node
    {
        T data;
        Node *next;
        Node(const T &d) : data(d), next(nullptr) {}
    };
    Node *frontNode;
    Node *backNode;

public:
    Queue() : frontNode(nullptr), backNode(nullptr) {}
    ~Queue() { clear(); }

    bool empty() const { return frontNode == nullptr; }

    void push(const T &value)
    {
        Node *n = new Node(value);
        if (!backNode)
        {
            frontNode = backNode = n;
        }
        else
        {
            backNode->next = n;
            backNode = n;
        }
    }

    T &front()
    {
        return frontNode->data;
    }

    void pop()
    {
        if (!frontNode)
            return;
        Node *tmp = frontNode;
        frontNode = frontNode->next;
        if (!frontNode)
            backNode = nullptr;
        delete tmp;
    }

    void clear()
    {
        while (frontNode)
        {
            Node *tmp = frontNode;
            frontNode = frontNode->next;
            delete tmp;
        }
        backNode = nullptr;
    }
};

// Maze difficulty data container
struct MazeDifficulty
{
    int id;
    float score;
    int rows, cols;
    vector<unsigned char> cellBytes;
    MazeDifficulty() : id(0), score(0), rows(0), cols(0) {}
    MazeDifficulty(int id_, float sc, int r, int c) : id(id_), score(sc), rows(r), cols(c), cellBytes(r * c, 0) {}
    bool operator<(MazeDifficulty const &o) const { return score < o.score; }
};

// Scoreboard
struct PlayerScore
{
    string name;
    float time;
    DifficultyLevel diff;
    string date;
    PlayerScore() : name(""), time(0), diff(DIFF_EASY), date("") {}
    PlayerScore(string n, float t, DifficultyLevel d, string dt) : name(n), time(t), diff(d), date(dt) {}
    bool operator<(PlayerScore const &o) const { return time < o.time; }
};
class BSTNode
{
public:
    PlayerScore data;
    BSTNode *left = nullptr;
    BSTNode *right = nullptr;
    BSTNode(const PlayerScore &p) : data(p) {}
};
class ScoreBST
{
    BSTNode *root = nullptr;
    BSTNode *insert(BSTNode *node, const PlayerScore &p)
    {
        if (!node)
            return new BSTNode(p);
        if (p < node->data)
            node->left = insert(node->left, p);
        else
            node->right = insert(node->right, p);
        return node;
    }
    void inorder(BSTNode *n, vector<PlayerScore> &out)
    {
        if (!n)
            return;
        inorder(n->left, out);
        out.push_back(n->data);
        inorder(n->right, out);
    }
    void destroy(BSTNode *n)
    {
        if (!n)
            return;
        destroy(n->left);
        destroy(n->right);
        delete n;
    }

public:
    ~ScoreBST() { clear(); }
    void add(const PlayerScore &p) { root = insert(root, p); }
    vector<PlayerScore> getSorted()
    {
        vector<PlayerScore> out;
        inorder(root, out);
        return out;
    }
    void clear()
    {
        destroy(root);
        root = nullptr;
    }
};

// ---------- Visit table ---------- //
class VisitHashTable
{
    static const int SIZE = 1031;
    struct Node
    {
        int x, y;
        Node *next;
        Node(int X, int Y) : x(X), y(Y), next(nullptr) {}
    };
    vector<Node *> table;
    int h(int x, int y) const
    {
        unsigned long long v = ((unsigned long long)(unsigned)x << 32) ^ (unsigned)y;
        return static_cast<int>(v % SIZE);
    }

public:
    VisitHashTable() : table(SIZE, nullptr) {}
    ~VisitHashTable() { clear(); }
    void mark(int x, int y)
    {
        int idx = h(x, y);
        Node *n = new Node(x, y);
        n->next = table[idx];
        table[idx] = n;
    }
    bool isVisited(int x, int y) const
    {
        int idx = h(x, y);
        Node *p = table[idx];
        while (p)
        {
            if (p->x == x && p->y == y)
                return true;
            p = p->next;
        }
        return false;
    }
    void clear()
    {
        for (int i = 0; i < SIZE; ++i)
        {
            Node *p = table[i];
            while (p)
            {
                Node *t = p;
                p = p->next;
                delete t;
            }
            table[i] = nullptr;
        }
    }
};

// ---------- Cell ---------- //
struct Cell
{
    int x = 0, y = 0;
    bool walls[4];
    bool isPath;
    bool isReplay;
    Cell() : x(0), y(0), isPath(false), isReplay(false)
    {
        walls[0] = walls[1] = walls[2] = walls[3] = true;
    }
    Cell(int X, int Y) : x(X), y(Y), isPath(false), isReplay(false)
    {
        walls[0] = walls[1] = walls[2] = walls[3] = true;
    }
};
struct Coord
{
    int x, y;
};

// ---------- Button ---------- //
struct Button
{
    Rectangle rect;
    string text;
    Color color, hoverColor;
    bool hovered;

    Button() : rect({0, 0, 0, 0}), text(""), color(LIGHTGRAY), hoverColor(GRAY), hovered(false) {}
    Button(float x, float y, float w, float h, const string &t, Color c = LIGHTGRAY, Color hc = GRAY)
        : rect({x, y, w, h}), text(t), color(c), hoverColor(hc), hovered(false)
    {
    }

    void draw()
    {
        DrawRectangleRec(rect, hovered ? hoverColor : color);
        DrawRectangleLinesEx(rect, 2, BLACK);
        int fs = 22;
        int tw = MeasureText(text.c_str(), fs);
        DrawText(text.c_str(), static_cast<int>(rect.x + (rect.width - tw) / 2.0f), static_cast<int>(rect.y + (rect.height - fs) / 2.0f), fs, WHITE);
    }
    bool checkHover(Vector2 m)
    {
        hovered = CheckCollisionPointRec(m, rect);
        return hovered;
    }
    bool clicked(Vector2 m) { return hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON); }
};

// ---------- Globals ---------- //
GameState currentState = STATE_HOME;
DifficultyLevel currentDifficulty = DIFF_EASY;
int mazeRows = SIZE_SMALL, mazeCols = SIZE_SMALL;
float cellSize = 0.0f;
vector<vector<Cell>> maze;
Stack<Cell *> dfsStack;
VisitHashTable visitedTable;
LinkedList playerPath;
string playerInput = "";
bool nameEntered = false;
vector<Coord> solutionPath;
int playerX = 0, playerY = 0, movesCount = 0;
float gameTimer = 0.0f, countdownTimer = 3.0f;
bool gamePaused = false, musicEnabled = true, musicPlaying = false;
;
vector<MazeDifficulty> generatedMazes;
float currentMazeScore = 0.0f;
ScoreBST easyScores, mediumScores, hardScores;
string playerName = "Player";
vector<Button> homeButtons, sizeButtons, diffButtons, endButtons, mainButtons;
Button musicBtn, pauseBtn;
bool endButtonsCreated = false;
bool mainButtonsCreated = false;
bool gameCompleted = false;
vector<Coord> replayPath;
int replayIndex = 0;
float replayTimer = 0.0f;
float replayDisplayTime = 0.0f;
int replayDisplayMoves = 0;
bool replayDisplayHasTime = false;
Color pathColor = Color{200, 200, 210, 220};
Color playerColor = Color{255, 100, 0, 200};
bool replaySessionIsSolution = false;

// ---------- Utilities ---------- //
string CurrentDateTime()
{
    time_t now = time(nullptr);
#if defined(_MSC_VER)
    tm t;
    localtime_s(&t, &now);
    char buf[80];
    strftime(buf, 80, "%Y-%m-%d %H:%M:%S", &t);
    return string(buf);
#else
    tm *t = localtime(&now);
    char buf[80];
    strftime(buf, 80, "%Y-%m-%d %H:%M:%S", t);
    return string(buf);
#endif
}

void DrawTextShadow(const char *text, int x, int y, int fontSize, Color color)
{
    DrawText(text, x + 2, y + 2, fontSize, Fade(BLACK, 0.35f)); // subtle shadow
    DrawText(text, x, y, fontSize, color);
}
void DrawCenteredTextShadow(const char *text, int centerX, int y, int fontSize, Color color)
{
    int tw = MeasureText(text, fontSize);
    DrawTextShadow(text, centerX - tw / 2, y, fontSize, color);
}

// ---------- Maze generation (iterative DFS) ---------- //
void initMazeGrid(int rows, int cols)
{
    maze.clear();
    maze.resize(rows, vector<Cell>(cols));
    for (int y = 0; y < rows; ++y)
        for (int x = 0; x < cols; ++x)
            maze[y][x] = Cell(x, y);
    while (!dfsStack.empty())
        dfsStack.pop();
    visitedTable.clear();
}

void generateFullMaze(int rows, int cols)
{
    mazeRows = rows;
    mazeCols = cols;
    initMazeGrid(rows, cols);

    Cell *start = &maze[0][0];
    start->isPath = true;
    visitedTable.mark(0, 0);
    dfsStack.push(start);

    const int dx[4] = {0, 0, -1, 1};
    const int dy[4] = {-1, 1, 0, 0};
    while (!dfsStack.empty())
    {
        Cell *cur = dfsStack.top();
        vector<int> dirs;
        for (int d = 0; d < 4; ++d)
        {
            int nx = cur->x + dx[d], ny = cur->y + dy[d];
            if (nx >= 0 && nx < mazeCols && ny >= 0 && ny < mazeRows && !visitedTable.isVisited(nx, ny))
                dirs.push_back(d);
        }
        if (!dirs.empty())
        {
            std::shuffle(dirs.begin(), dirs.end(), rng);
            int d = dirs[0];
            int nx = cur->x + dx[d], ny = cur->y + dy[d];
            if (d == 0)
            {
                cur->walls[0] = false;
                maze[ny][nx].walls[1] = false;
            }
            else if (d == 1)
            {
                cur->walls[1] = false;
                maze[ny][nx].walls[0] = false;
            }
            else if (d == 2)
            {
                cur->walls[2] = false;
                maze[ny][nx].walls[3] = false;
            }
            else
            {
                cur->walls[3] = false;
                maze[ny][nx].walls[2] = false;
            }
            maze[ny][nx].isPath = true;
            visitedTable.mark(nx, ny);
            dfsStack.push(&maze[ny][nx]);
        }
        else
        {
            dfsStack.pop();
        }
    }
    maze[0][0].isPath = true;
    maze[rows - 1][cols - 1].isPath = true;
}

// ---------- BFS solver ---------- //
vector<Cell *> BFSSolve()
{
    vector<Cell *> path;
    if (mazeRows <= 0 || mazeCols <= 0)
        return path;
    vector<vector<bool>> vis(mazeRows, vector<bool>(mazeCols, false));
    vector<vector<Cell *>> parent(mazeRows, vector<Cell *>(mazeCols, nullptr));
    Queue<Cell *> q;
    q.push(&maze[0][0]);
    vis[0][0] = true;
    while (!q.empty())
    {
        Cell *cur = q.front();
        q.pop();
        if (cur->x == mazeCols - 1 && cur->y == mazeRows - 1)
            break;
        if (!cur->walls[0] && cur->y > 0 && !vis[cur->y - 1][cur->x])
        {
            vis[cur->y - 1][cur->x] = true;
            parent[cur->y - 1][cur->x] = cur;
            q.push(&maze[cur->y - 1][cur->x]);
        }
        if (!cur->walls[1] && cur->y < mazeRows - 1 && !vis[cur->y + 1][cur->x])
        {
            vis[cur->y + 1][cur->x] = true;
            parent[cur->y + 1][cur->x] = cur;
            q.push(&maze[cur->y + 1][cur->x]);
        }
        if (!cur->walls[2] && cur->x > 0 && !vis[cur->y][cur->x - 1])
        {
            vis[cur->y][cur->x - 1] = true;
            parent[cur->y][cur->x - 1] = cur;
            q.push(&maze[cur->y][cur->x - 1]);
        }
        if (!cur->walls[3] && cur->x < mazeCols - 1 && !vis[cur->y][cur->x + 1])
        {
            vis[cur->y][cur->x + 1] = true;
            parent[cur->y][cur->x + 1] = cur;
            q.push(&maze[cur->y][cur->x + 1]);
        }
    }
    Cell *cur = &maze[mazeRows - 1][mazeCols - 1];
    if (!parent[cur->y][cur->x] && !(cur->x == 0 && cur->y == 0))
        return path;
    while (cur)
    {
        path.push_back(cur);
        Cell *p = parent[cur->y][cur->x];
        if (p == cur)
            break;
        cur = p;
    }
    reverse(path.begin(), path.end());
    return path;
}

// ---------- Score file I/O ---------- //
void SaveScoreToFile(const PlayerScore &p)
{
    ofstream f("scores.txt", ios::app);
    if (!f.is_open())
        return;
    f << p.name << "," << p.time << "," << (int)p.diff << "," << p.date << "\n";
    f.close();
}
void LoadScoresFromFile()
{
    ifstream f("scores.txt");
    if (!f.is_open())
        return;
    string line;
    while (getline(f, line))
    {
        stringstream ss(line);
        string name, timeS, diffS, date;
        if (!getline(ss, name, ','))
            continue;
        if (!getline(ss, timeS, ','))
            continue;
        if (!getline(ss, diffS, ','))
            continue;
        if (!getline(ss, date))
            continue;
        try
        {
            float t = stof(timeS);
            int di = stoi(diffS);
            PlayerScore ps(name, t, (DifficultyLevel)di, date);
            if (ps.diff == DIFF_EASY)
                easyScores.add(ps);
            else if (ps.diff == DIFF_MEDIUM)
                mediumScores.add(ps);
            else
                hardScores.add(ps);
        }
        catch (...)
        {
            continue;
        }
    }
    f.close();
}

// ---------- UI Initialization ---------- //
void InitUI()
{
    homeButtons.clear();
    homeButtons.push_back(Button(350, 200, 300, 60, "START GAME", UI_BLUE, BLUEDARK));
    homeButtons.push_back(Button(350, 280, 300, 60, "SCOREBOARD", Color{40, 200, 120, 255}, GREENDARK));
    homeButtons.push_back(Button(350, 360, 300, 60, "EXIT", DARKRED, Color{160, 20, 20, 255}));

    sizeButtons.clear();
    sizeButtons.push_back(Button(350, 200, 300, 60, "SMALL (20x20)", Color{90, 90, 90, 255}, Color{120, 120, 120, 255}));
    sizeButtons.push_back(Button(350, 280, 300, 60, "MEDIUM (30x30)", Color{90, 90, 90, 255}, Color{120, 120, 120, 255}));
    sizeButtons.push_back(Button(350, 360, 300, 60, "LARGE (40x40)", Color{90, 90, 90, 255}, Color{120, 120, 120, 255}));

    diffButtons.clear();
    diffButtons.push_back(Button(350, 200, 300, 60, "EASY", Color{50, 180, 80, 255}, GREENDARK));
    diffButtons.push_back(Button(350, 280, 300, 60, "MEDIUM", MY_ORANGE, DARKORANGE));
    diffButtons.push_back(Button(350, 360, 300, 60, "HARD", DARKRED, Color{160, 20, 20, 255}));

    endButtons.clear();
    int cx = (SCREEN_WIDTH - UI_PANEL_WIDTH) / 2;
    int cy = SCREEN_HEIGHT / 3;
    endButtons.push_back(Button(static_cast<float>(cx) - 150.0f, static_cast<float>(cy) + 220.0f, 300, 60, "RETRY MAZE", UI_BLUE, BLUEDARK));
    endButtons.push_back(Button(static_cast<float>(cx) - 150.0f, static_cast<float>(cy) + 280.0f, 300, 60, "SELECT ANOTHER", Color{40, 200, 120, 255}, GREENDARK));
    endButtons.push_back(Button(static_cast<float>(cx) - 150.0f, static_cast<float>(cy) + 340.0f, 300, 60, "RETURN HOME", DARKRED, Color{160, 20, 20, 255}));

    musicBtn = Button(static_cast<float>(SCREEN_WIDTH - UI_PANEL_WIDTH) + 12.0f, 12.0f, 36, 36, "M", Color{220, 220, 220, 255}, Color{200, 200, 200, 255});
    pauseBtn = Button(static_cast<float>(SCREEN_WIDTH - 150), 12.0f, 36, 36, "||");

    LoadScoresFromFile();
    endButtonsCreated = true;
}

// ---------- Drawing helpers for Maze style tiles ---------- //

void DrawGreenWall(float x, float y, float w, float h)
{
    DrawRectangle(static_cast<int>(x), static_cast<int>(y), static_cast<int>(w), static_cast<int>(h), Color{14, 160, 0, 255});
    DrawRectangle(static_cast<int>(x), static_cast<int>(y), static_cast<int>(w), static_cast<int>(h * 0.45f), Color{100, 220, 100, 255});
    DrawRectangleLines(static_cast<int>(x), static_cast<int>(y), static_cast<int>(w), static_cast<int>(h), Color{20, 90, 10, 255});
}

void DrawFloorTile(float x, float y, float w, float h)
{
    DrawRectangle(static_cast<int>(x), static_cast<int>(y), static_cast<int>(w), static_cast<int>(h), Fade(LIGHTGRAY, 0.08f));
    DrawRectangleLines(static_cast<int>(x), static_cast<int>(y), static_cast<int>(w), static_cast<int>(h), Fade(BLACK, 0.06f));
}

void DrawCheeseIcon(float cx, float cy, float r)
{
    Vector2 p1 = {cx, cy - r};
    Vector2 p2 = {cx - r, cy + r};
    Vector2 p3 = {cx + r, cy + r};

    Vector2 verts[3] = {p1, p2, p3};
    DrawTriangle(p1, p2, p3, YELLOW);
    DrawTriangleLines(p1, p2, p3, BLACK);
}

void DrawBombIcon(float cx, float cy, float r)
{
    DrawCircle(static_cast<int>(cx), static_cast<int>(cy), static_cast<int>(r), DARKGRAY);
    DrawCircleLines(static_cast<int>(cx), static_cast<int>(cy), static_cast<int>(r), BLACK);
    DrawLine(static_cast<int>(cx - r * 0.6f), static_cast<int>(cy - r), static_cast<int>(cx - r * 0.2f), static_cast<int>(cy - r * 0.8f), BLACK);
}

void DrawEnemyIcon(float cx, float cy, float r, Color c)
{
    DrawCircle(static_cast<int>(cx), static_cast<int>(cy), static_cast<int>(r), c);
    DrawCircle(static_cast<int>(cx - r / 3.5f), static_cast<int>(cy - r / 6.0f), static_cast<int>(r * 0.18f), WHITE);
    DrawCircle(static_cast<int>(cx + r / 3.5f), static_cast<int>(cy - r / 6.0f), static_cast<int>(r * 0.18f), WHITE);
    DrawCircle(static_cast<int>(cx), static_cast<int>(cy + r / 4.0f), static_cast<int>(r * 0.12f), BLACK);
}

void DrawPlayerIcon(float cx, float cy, float r)
{
    DrawCircle(static_cast<int>(cx), static_cast<int>(cy), static_cast<int>(r), playerColor);
    DrawCircle(static_cast<int>(cx - r / 2.5f), static_cast<int>(cy - r / 2.5f), static_cast<int>(r * 0.4f), WHITE); // ear highlight
}

// ---------- Draw Maze in Mice-Maze look (walls as green strips) ---------- //
void DrawMaze()
{
    if (mazeRows <= 0 || mazeCols <= 0)
        return;
    int mazeWidth = SCREEN_WIDTH - UI_PANEL_WIDTH;
    cellSize = min(static_cast<float>(mazeWidth) / mazeCols, static_cast<float>(SCREEN_HEIGHT) / mazeRows);
    float startX = (mazeWidth - mazeCols * cellSize) / 2.0f;
    float startY = (SCREEN_HEIGHT - mazeRows * cellSize) / 2.0f;

    // background frame
    DrawRectangle(static_cast<int>(startX) - 2, static_cast<int>(startY) - 2, static_cast<int>(mazeCols * cellSize + 4.0f), static_cast<int>(mazeRows * cellSize + 4.0f), BLACK);

    // floors for carved path cells and dark tiles for walls
    for (int y = 0; y < mazeRows; ++y)
    {
        for (int x = 0; x < mazeCols; ++x)
        {
            float cx = startX + x * cellSize;
            float cy = startY + y * cellSize;
            float pad = cellSize * 0.06f;
            if (maze[y][x].isPath)
            {
                float cx = startX + x * cellSize;
                float cy = startY + y * cellSize;
                float pad = cellSize * 0.06f;

                if (maze[y][x].isReplay)
                {
                    DrawRectangle((int)(cx + pad), (int)(cy + pad),
                                  (int)(cellSize - 2 * pad), (int)(cellSize - 2 * pad),
                                  Color{255, 215, 0, 220}); // gold
                    DrawRectangleLines((int)(cx + pad), (int)(cy + pad),
                                       (int)(cellSize - 2 * pad), (int)(cellSize - 2 * pad),
                                       Fade(BLACK, 0.25f));
                }
                else
                {
                    DrawFloorTile(cx + pad, cy + pad,
                                  cellSize - 2 * pad, cellSize - 2 * pad);
                }
            }
            else
            {
                DrawRectangle(static_cast<int>(cx + pad), static_cast<int>(cy + pad), static_cast<int>(cellSize - 2 * pad), static_cast<int>(cellSize - 2 * pad), Color{10, 10, 10, 255});
            }
        }
    }

    // walls as green strips on edges
    float wallThickness = max(2.0f, cellSize * 0.16f);
    for (int y = 0; y < mazeRows; ++y)
    {
        for (int x = 0; x < mazeCols; ++x)
        {
            float cx = startX + x * cellSize;
            float cy = startY + y * cellSize;
            if (maze[y][x].walls[0])
                DrawGreenWall(cx, cy - wallThickness / 2.0f, cellSize, wallThickness);
            if (maze[y][x].walls[1])
                DrawGreenWall(cx, cy + cellSize - wallThickness / 2.0f, cellSize, wallThickness);
            if (maze[y][x].walls[2])
                DrawGreenWall(cx - wallThickness / 2.0f, cy, wallThickness, cellSize);
            if (maze[y][x].walls[3])
                DrawGreenWall(cx + cellSize - wallThickness / 2.0f, cy, wallThickness, cellSize);
        }
    }

    //// Start / End markers
    // float sx = startX + cellSize / 2.0f, sy = startY + cellSize / 2.0f;
    // float ex = startX + (mazeCols - 1) * cellSize + cellSize / 2.0f, ey = startY + (mazeRows - 1) * cellSize + cellSize / 2.0f;
    // DrawCircleLines(static_cast<int>(sx), static_cast<int>(sy), static_cast<int>(cellSize * 0.33f), WHITE);
    // DrawCircle(static_cast<int>(ex), static_cast<int>(ey), static_cast<int>(cellSize * 0.33f), RED);
}

void DrawDecorations()
{
    if (mazeRows <= 0 || mazeCols <= 0)
        return;
    int mazeWidth = SCREEN_WIDTH - UI_PANEL_WIDTH;
    float startX = (mazeWidth - mazeCols * cellSize) / 2.0f;
    float startY = (SCREEN_HEIGHT - mazeRows * cellSize) / 2.0f;

    for (int y = 0; y < mazeRows; ++y)
    {
        for (int x = 0; x < mazeCols; ++x)
        {
            if (!maze[y][x].isPath)
                continue;

            float cx = startX + x * cellSize + cellSize / 2.0f;
            float cy = startY + y * cellSize + cellSize / 2.0f;

            if (x == mazeCols - 1 && y == mazeRows - 1)
            {
                DrawCheeseIcon(cx, cy, cellSize * 0.25f);
                continue;
            }
        }
    }
}

// ---------- Draw player ---------- //
void DrawPlayer()
{
    if (cellSize <= 0)
        return;
    int mazeWidth = SCREEN_WIDTH - UI_PANEL_WIDTH;
    float startX = (mazeWidth - mazeCols * cellSize) / 2.0f;
    float startY = (SCREEN_HEIGHT - mazeRows * cellSize) / 2.0f;

    float px = startX + playerX * cellSize + cellSize / 2.0f;
    float py = startY + playerY * cellSize + cellSize / 2.0f;
    DrawPlayerIcon(px, py, cellSize * 0.28f);

    float sx = startX + cellSize / 2.0f;
    float sy = startY + cellSize / 2.0f;
    DrawCircle(static_cast<int>(sx), static_cast<int>(sy),
               static_cast<int>(cellSize * 0.18f), RED);
}

// the right panel
void DrawHUD()
{
    int panelX = SCREEN_WIDTH - UI_PANEL_WIDTH;
    DrawRectangle(panelX, 0, UI_PANEL_WIDTH, SCREEN_HEIGHT, UI_BLUE);
    if (currentState != STATE_REPLAY)
    {
        Vector2 m = GetMousePosition();
        musicBtn.checkHover(m);
        musicBtn.draw();
        DrawText(musicEnabled ? "ON" : "OFF", panelX + 60, 20, 14, WHITE);
    }

    if (currentState == STATE_REPLAY)
    {
        if (replayDisplayHasTime)
            DrawText(TextFormat("TIME: %.2fs", replayDisplayTime), panelX + 20, 90, 18, WHITE);
        else
            DrawText("TIME: -", panelX + 20, 90, 18, WHITE);

        DrawText(TextFormat("MOVES: %d", replayDisplayMoves), panelX + 20, 120, 16, WHITE);
    }
    else
    {
        DrawText(TextFormat("TIME: %.2fs", gameTimer), panelX + 20, 90, 18, WHITE);
        DrawText(TextFormat("MOVES: %d", movesCount), panelX + 20, 120, 16, WHITE);
    }

    // Difficulty bar
    DrawText("DIFFICULTY", panelX + 20, 190, 14, WHITE);
    DrawRectangle(panelX + 20, 215, 200, 18, Fade(WHITE, 0.12f));

    float fill = currentMazeScore / 100.0f * 200.0f;
    Color col;
    if (currentDifficulty == DIFF_EASY)
        col = GREEN;
    else if (currentDifficulty == DIFF_MEDIUM)
        col = MY_ORANGE;
    else
        col = RED;

    DrawRectangle(panelX + 20, 215, static_cast<int>(fill), 18, col);
    DrawRectangleLines(panelX + 20, 215, 200, 18, BLACK);
    DrawText("Controls:", panelX + 20, 260, 14, WHITE);
    DrawText("Arrow keys - move", panelX + 20, 285, 12, WHITE);
    DrawText("Q - back to home", panelX + 20, 315, 12, WHITE);
}

// ---------- Update hover ---------- //
void UpdateHoversMain()
{
    Vector2 m = GetMousePosition();
    if (currentState == STATE_HOME)
    {
        for (auto &b : homeButtons)
            b.checkHover(m);
        musicBtn.checkHover(m);
    }
    else if (currentState == STATE_SIZE_SELECT)
        for (auto &b : sizeButtons)
            b.checkHover(m);
    else if (currentState == STATE_DIFFICULTY_SELECT)
        for (auto &b : diffButtons)
            b.checkHover(m);
    else if (currentState == STATE_END && !endButtons.empty())
        for (auto &b : endButtons)
            b.checkHover(m);

    else if (currentState == STATE_PLAYING)
        pauseBtn.checkHover(m);
}

// ---------- End comparison / replay ---------- //
void ShowEndComparison()
{
    bool perfect = false;
    if (playerPath.getSize() == static_cast<int>(solutionPath.size()))
    {
        perfect = true;
        auto *n = playerPath.getHead();
        for (size_t i = 0; i < solutionPath.size() && n; ++i)
        {
            const Coord &pos = solutionPath[i];
            if (n->data.x != pos.x || n->data.y != pos.y)
            {
                perfect = false;
                break;
            }

            n = n->next;
        }
    }

    int cx = (SCREEN_WIDTH - UI_PANEL_WIDTH) / 2;
    int cy = SCREEN_HEIGHT / 3;
    if (perfect)
    {
        DrawCenteredTextShadow("PERFECT RUN!", cx, cy, 40, Color{200, 255, 200, 255});
        DrawCenteredTextShadow("You found the shortest path!", cx, cy + 56, 22, GREENDARK);
    }
    else
    {
        DrawCenteredTextShadow("GOOD EFFORT!", cx, cy, 40, MY_ORANGE);
        DrawCenteredTextShadow("View optimal path or replay your path", cx, cy + 56, 20, DARKORANGE);

        // OPTIMAL / REPLAY buttons
        Button opt(static_cast<float>(cx) - 150.0f, static_cast<float>(cy) + 100.0f, 300, 60, "OPTIMAL PATH", Color{70, 130, 255, 255}, BLUEDARK);
        Button rep(static_cast<float>(cx) - 150.0f, static_cast<float>(cy) + 160.0f, 300, 60, "REPLAY MY PATH", Color{40, 200, 120, 255}, GREENDARK);
        Vector2 m = GetMousePosition();
        opt.checkHover(m);
        rep.checkHover(m);
        opt.draw();
        rep.draw();

        if (opt.clicked(m) && !solutionPath.empty())
        {
            for (auto &row : maze)
                for (auto &c : row)
                    c.isReplay = false;

            replayPath = solutionPath;
            replayIndex = 0;
            replayTimer = 0.0f;
            replaySessionIsSolution = true;
            currentState = STATE_REPLAY;
        }

        if (rep.clicked(m))
        {
            replayPath.clear();
            auto *n = playerPath.getHead();
            while (n)
            {
                replayPath.push_back({n->data.x, n->data.y});
                n = n->next;
            }

            for (auto &row : maze)
                for (auto &c : row)
                    c.isReplay = false;

            replayIndex = 0;
            replayTimer = 0.0f;
            replaySessionIsSolution = false;
            currentState = STATE_REPLAY;
        }
    }
}

float evaluateDifficulty(int rows, int cols);

// ---------- Reset and start ---------- //
void ResetGameWithCurrentMaze()
{
    if (generatedMazes.empty())
    {
        generateFullMaze(mazeRows, mazeCols);
        currentMazeScore = evaluateDifficulty(mazeRows, mazeCols);
    }
    playerX = 0;
    playerY = 0;
    movesCount = 0;
    gameTimer = 0.0f;
    countdownTimer = 3.0f;
    gamePaused = false;
    playerPath.clear();
    playerPath.add(PlayerMove(playerX, playerY, 0.0f));
    solutionPath.clear();
    for (auto *c : BFSSolve())
    {
        solutionPath.push_back({c->x, c->y});
    }

    replayPath.clear();
    replayIndex = 0;
    replayTimer = 0.0f;
    gameCompleted = false;
}

// ---------- Evaluate difficulty quick helper ---------- //
float evaluateDifficulty(int rows, int cols)
{
    if (maze.empty())
        return 0.0f;
    int R = rows, C = cols;
    int total = R * C, deadEnds = 0, branches = 0, maxCoord = 0;
    for (int y = 0; y < R; ++y)
        for (int x = 0; x < C; ++x)
        {
            int open = 0;
            for (int k = 0; k < 4; ++k)
                if (!maze[y][x].walls[k])
                    open++;
            if (open == 1)
                deadEnds++;
            if (open >= 3)
                branches++;
            if (x + y > maxCoord)
                maxCoord = x + y;
        }
    float deadRatio = static_cast<float>(deadEnds) / static_cast<float>(total);
    float branchRatio = static_cast<float>(branches) / static_cast<float>(total);
    float pathRatio = static_cast<float>(maxCoord) / static_cast<float>(R + C);
    float score = (deadRatio * 40.0f) + (branchRatio * 35.0f) + (pathRatio * 25.0f);
    score = min(score * 100.0f, 100.0f);
    return score;
}

// ---------- Multi-maze generate & pick (simple) ---------- //
void GenerateMultipleMazesAndPick()
{
    generatedMazes.clear();
    const int N = 6;
    for (int i = 0; i < N; ++i)
    {
        generateFullMaze(mazeRows, mazeCols);
        float sc = evaluateDifficulty(mazeRows, mazeCols);
        MazeDifficulty md(i, sc, mazeRows, mazeCols);
        md.cellBytes.resize(mazeRows * mazeCols);
        for (int y = 0; y < mazeRows; ++y)
            for (int x = 0; x < mazeCols; ++x)
            {
                unsigned char b = 0;
                if (maze[y][x].walls[0])
                    b |= 1 << 0;
                if (maze[y][x].walls[1])
                    b |= 1 << 1;
                if (maze[y][x].walls[2])
                    b |= 1 << 2;
                if (maze[y][x].walls[3])
                    b |= 1 << 3;
                if (maze[y][x].isPath)
                    b |= 1 << 4;
                md.cellBytes[y * mazeCols + x] = b;
            }
        generatedMazes.push_back(md);
    }
    float minS = 0, maxS = 100;
    if (currentDifficulty == DIFF_EASY)
    {
        minS = 0;
        maxS = 33;
    }
    else if (currentDifficulty == DIFF_MEDIUM)
    {
        minS = 34;
        maxS = 66;
    }
    else
    {
        minS = 67;
        maxS = 100;
    }
    sort(generatedMazes.begin(), generatedMazes.end(), [](const MazeDifficulty &a, const MazeDifficulty &b)
         { return a.score > b.score; });
    bool found = false;
    for (auto &md : generatedMazes)
    {
        if (md.score >= minS && md.score <= maxS)
        {
            maze.clear();
            maze.resize(md.rows, vector<Cell>(md.cols));
            for (int y = 0; y < md.rows; ++y)
                for (int x = 0; x < md.cols; ++x)
                {
                    maze[y][x] = Cell(x, y);
                    unsigned char b = md.cellBytes[y * md.cols + x];
                    maze[y][x].walls[0] = (b & (1 << 0)) != 0;
                    maze[y][x].walls[1] = (b & (1 << 1)) != 0;
                    maze[y][x].walls[2] = (b & (1 << 2)) != 0;
                    maze[y][x].walls[3] = (b & (1 << 3)) != 0;
                    maze[y][x].isPath = (b & (1 << 4)) != 0;
                }
            currentMazeScore = md.score;
            found = true;
            break;
        }
    }
    if (!found)
        currentMazeScore = evaluateDifficulty(mazeRows, mazeCols);
}

int main()
{
    rng.seed((unsigned)chrono::system_clock::now().time_since_epoch().count());

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Maze Runner");
    SetTargetFPS(FPS);

    InitUI();
    generateFullMaze(mazeRows, mazeCols);
    solutionPath.clear();
    for (auto *c : BFSSolve())
    {
        solutionPath.push_back({c->x, c->y});
    }
    playerPath.clear();
    playerPath.add(PlayerMove(0, 0, 0.0f));

    bool shouldClose = false;

    InitAudioDevice();
    SetMasterVolume(1.0f);
    backgroundMusic = LoadMusicStream("bg_music.mp3");
    backgroundMusic.looping = true;
    musicPlaying = false;
    musicEnabled = true;

    while (!WindowShouldClose() && !shouldClose)
    {
        float dt = GetFrameTime();
        UpdateHoversMain();
        if (currentState == STATE_PLAYING && musicEnabled)
        {
            if (!musicPlaying)
            {
                PlayMusicStream(backgroundMusic);
                musicPlaying = true;
            }
            UpdateMusicStream(backgroundMusic);
        }

        if (currentState != STATE_PLAYING && musicPlaying)
        {
            StopMusicStream(backgroundMusic);
            musicPlaying = false;
        }
        switch (currentState)
        {
        case STATE_HOME:
        {
            Vector2 m = GetMousePosition();
            if (homeButtons[0].clicked(m))
            {
                currentState = STATE_SIZE_SELECT;
            }
            if (homeButtons[1].clicked(m))
            {
                currentState = STATE_SCOREBOARD;
            }
            if (homeButtons[2].clicked(m))
            {
                shouldClose = true;
            }
            if (homeButtons[0].clicked(m))
            {
                playerInput.clear();
                currentState = STATE_PLAYER_NAME;
            }
            break;
        }
        case STATE_PLAYER_NAME:
        {
            Vector2 m = GetMousePosition();
            if (IsKeyPressed(KEY_ENTER) && !playerInput.empty())
            {
                playerName = playerInput;
                currentState = STATE_SIZE_SELECT;
            }
            if (IsKeyPressed(KEY_BACKSPACE) && !playerInput.empty())
            {
                playerInput.pop_back();
            }
            if (IsKeyPressed(KEY_Q))
            {
                currentState = STATE_HOME;
            }
            int key = GetCharPressed();
            while (key > 0)
            {
                if ((key >= 32 && key <= 125) && playerInput.length() < 12)
                {
                    playerInput += (char)key;
                }
                key = GetCharPressed();
            }
            break;
        }
        case STATE_SIZE_SELECT:
        {
            Vector2 m = GetMousePosition();
            if (sizeButtons[0].clicked(m))
            {
                mazeRows = mazeCols = SIZE_SMALL;
                currentState = STATE_DIFFICULTY_SELECT;
            }
            if (sizeButtons[1].clicked(m))
            {
                mazeRows = mazeCols = SIZE_MEDIUM;
                currentState = STATE_DIFFICULTY_SELECT;
            }
            if (sizeButtons[2].clicked(m))
            {
                mazeRows = mazeCols = SIZE_LARGE;
                currentState = STATE_DIFFICULTY_SELECT;
            }
            if (IsKeyPressed(KEY_Q))
                currentState = STATE_HOME;
            break;
        }
        case STATE_DIFFICULTY_SELECT:
        {
            Vector2 m = GetMousePosition();
            if (diffButtons[0].clicked(m))
            {
                currentDifficulty = DIFF_EASY;
                GenerateMultipleMazesAndPick();
                currentState = STATE_COUNTDOWN;
                countdownTimer = 3.0f;
            }
            if (diffButtons[1].clicked(m))
            {
                currentDifficulty = DIFF_MEDIUM;
                GenerateMultipleMazesAndPick();
                currentState = STATE_COUNTDOWN;
                countdownTimer = 3.0f;
            }
            if (diffButtons[2].clicked(m))
            {
                currentDifficulty = DIFF_HARD;
                GenerateMultipleMazesAndPick();
                currentState = STATE_COUNTDOWN;
                countdownTimer = 3.0f;
            }
            if (IsKeyPressed(KEY_Q))
                currentState = STATE_SIZE_SELECT;
            break;
        }
        case STATE_COUNTDOWN:
        {
            countdownTimer -= dt;
            if (countdownTimer <= 0.0f)
            {
                currentState = STATE_PLAYING;
                ResetGameWithCurrentMaze();
            }
            break;
        }
        case STATE_PLAYING:
        {
            if (!gamePaused)
            {
                gameTimer += dt;
                if (IsKeyPressed(KEY_UP) && playerY > 0 && !maze[playerY][playerX].walls[0])
                {
                    playerY--;
                    movesCount++;
                    playerPath.add(PlayerMove(playerX, playerY, gameTimer));
                }
                if (IsKeyPressed(KEY_DOWN) && playerY < mazeRows - 1 && !maze[playerY][playerX].walls[1])
                {
                    playerY++;
                    movesCount++;
                    playerPath.add(PlayerMove(playerX, playerY, gameTimer));
                }
                if (IsKeyPressed(KEY_LEFT) && playerX > 0 && !maze[playerY][playerX].walls[2])
                {
                    playerX--;
                    movesCount++;
                    playerPath.add(PlayerMove(playerX, playerY, gameTimer));
                }
                if (IsKeyPressed(KEY_RIGHT) && playerX < mazeCols - 1 && !maze[playerY][playerX].walls[3])
                {
                    playerX++;
                    movesCount++;
                    playerPath.add(PlayerMove(playerX, playerY, gameTimer));
                }

                if (playerX == mazeCols - 1 && playerY == mazeRows - 1)
                {
                    gameCompleted = true;
                    PlayerScore ps(playerName, gameTimer, currentDifficulty, CurrentDateTime());
                    if (currentDifficulty == DIFF_EASY)
                        easyScores.add(ps);
                    else if (currentDifficulty == DIFF_MEDIUM)
                        mediumScores.add(ps);
                    else
                        hardScores.add(ps);
                    SaveScoreToFile(ps);
                    currentState = STATE_END;
                }
            }

            Vector2 m = GetMousePosition();
            if (pauseBtn.clicked(m))
            {
                gamePaused = !gamePaused;
                if (gamePaused)
                {
                    PauseMusicStream(backgroundMusic);
                }
                else
                {
                    ResumeMusicStream(backgroundMusic);
                }
            }

            if (musicBtn.clicked(m))
            {
                musicEnabled = !musicEnabled;
                if (musicEnabled)
                {
                    PlayMusicStream(backgroundMusic);
                }
                else
                {
                    StopMusicStream(backgroundMusic);
                }
            }

            if (IsKeyPressed(KEY_Q))
            {
                currentState = STATE_HOME;
                gameCompleted = false;
            }
            break;
        }
        case STATE_END:
        {
            StopMusicStream(backgroundMusic);
            Vector2 m = GetMousePosition();
            if (endButtons[0].clicked(m))
            {
                ResetGameWithCurrentMaze();
                countdownTimer = 3.0f;
                currentState = STATE_COUNTDOWN;
            }
            else if (endButtons[1].clicked(m))
            {
                currentState = STATE_SIZE_SELECT;
            }
            else if (endButtons[2].clicked(m))
            {
                currentState = STATE_HOME;
            }
            break;
        }
        case STATE_SCOREBOARD:
        {
            if (IsKeyPressed(KEY_Q))
                currentState = STATE_HOME;
            break;
        }
        case STATE_REPLAY:
        {
            replayTimer += dt;

            if (replayTimer >= 0.12f &&
                replayIndex < (int)replayPath.size())
            {

                for (auto &row : maze)
                    for (auto &c : row)
                        c.isReplay = false;

                const Coord pos = replayPath[replayIndex];
                maze[pos.y][pos.x].isReplay = true;
                replayIndex++;
                replayTimer = 0.0f;
            }

            if (replaySessionIsSolution)
            {
                replayDisplayMoves = (int)solutionPath.size() - 1;
                replayDisplayHasTime = false;
            }
            else
            {
                replayDisplayMoves = movesCount;
                replayDisplayTime = gameTimer;
                replayDisplayHasTime = true;
            }

            if (IsKeyPressed(KEY_Q))
            {
                for (auto &row : maze)
                    for (auto &c : row)
                        c.isReplay = false;
                currentState = STATE_END;
            }
            break;
        }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        if (currentState == STATE_HOME)
        {
            // main maze area background
            DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color{18, 18, 18, 255});
            DrawCenteredTextShadow("MAZE RUNNER - MICE STYLE", (SCREEN_WIDTH) / 2, 60, 32, WHITE);
            DrawCenteredTextShadow("A clean & stylish maze experience", (SCREEN_WIDTH) / 2, 100, 16, Color{200, 200, 255, 200});
            for (auto &b : homeButtons)
                b.draw();
            DrawText("Instructions: Use arrow keys to move", 24, SCREEN_HEIGHT - 40, 14, Color{200, 200, 200, 220});
        }
        else if (currentState == STATE_PLAYER_NAME)
        {
            DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color{18, 18, 18, 255});
            DrawCenteredTextShadow("ENTER YOUR NAME", SCREEN_WIDTH / 2, 200, 32, WHITE);
            DrawCenteredTextShadow("(Press ENTER to continue)", SCREEN_WIDTH / 2, 280, 20, Color{200, 200, 200, 255});
            DrawRectangle(SCREEN_WIDTH / 2 - 150, 320, 300, 60, UI_BLUE);
            DrawRectangleLinesEx({SCREEN_WIDTH / 2 - 150, 320, 300, 60}, 2, BLACK);
            DrawText(playerInput.c_str(), SCREEN_WIDTH / 2 - MeasureText(playerInput.c_str(), 32) / 2, 335, 32, WHITE);
            DrawText((GetTime() * 2 > (int)(GetTime() * 2)) ? "|" : "", SCREEN_WIDTH / 2 + MeasureText(playerInput.c_str(), 32) / 2 + 5, 335, 32, WHITE);
            DrawText("Q: Back to Home", 24, SCREEN_HEIGHT - 40, 14, Color{200, 200, 200, 220});
        }

        else if (currentState == STATE_SIZE_SELECT)
        {
            DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color{18, 18, 18, 255});
            DrawCenteredTextShadow("SELECT SIZE", (SCREEN_WIDTH) / 2, 60, 32, WHITE);
            for (auto &b : sizeButtons)
                b.draw();
            DrawText("Q: Back to Home", 24, SCREEN_HEIGHT - 40, 14, Color{200, 200, 200, 220});
        }
        else if (currentState == STATE_DIFFICULTY_SELECT)
        {
            DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color{18, 18, 18, 255});
            DrawCenteredTextShadow("SELECT DIFFICULTY", (SCREEN_WIDTH) / 2, 60, 28, WHITE);
            for (auto &b : diffButtons)
                b.draw();
            DrawText("Q: Back to Home", 24, SCREEN_HEIGHT - 40, 14, Color{200, 200, 200, 220});
        }
        else if (currentState == STATE_COUNTDOWN)
        {
            DrawRectangle(0, 0, SCREEN_WIDTH - UI_PANEL_WIDTH, SCREEN_HEIGHT, Color{18, 18, 18, 255});
            DrawRectangle(SCREEN_WIDTH - UI_PANEL_WIDTH, 0, UI_PANEL_WIDTH, SCREEN_HEIGHT, UI_BLUE);
            DrawLine(SCREEN_WIDTH - UI_PANEL_WIDTH, 0, SCREEN_WIDTH - UI_PANEL_WIDTH, SCREEN_HEIGHT, UI_BLUE);
            DrawMaze();
            DrawPlayer();
            if (countdownTimer > 2.0f)
                DrawCenteredTextShadow("3", (SCREEN_WIDTH - UI_PANEL_WIDTH) / 2, SCREEN_HEIGHT / 2 - 40, 120, RED);
            else if (countdownTimer > 1.0f)
                DrawCenteredTextShadow("2", (SCREEN_WIDTH - UI_PANEL_WIDTH) / 2, SCREEN_HEIGHT / 2 - 40, 120, MY_ORANGE);
            else if (countdownTimer > 0.0f)
                DrawCenteredTextShadow("1", (SCREEN_WIDTH - UI_PANEL_WIDTH) / 2, SCREEN_HEIGHT / 2 - 40, 120, YELLOW);
            else
                DrawCenteredTextShadow("GO!", (SCREEN_WIDTH - UI_PANEL_WIDTH) / 2, SCREEN_HEIGHT / 2 - 40, 120, Color{120, 255, 120, 255});
        }
        else if (currentState == STATE_PLAYING)
        {
            DrawRectangle(0, 0, SCREEN_WIDTH - UI_PANEL_WIDTH, SCREEN_HEIGHT, Color{18, 18, 18, 255});
            DrawRectangle(SCREEN_WIDTH - UI_PANEL_WIDTH, 0, UI_PANEL_WIDTH, SCREEN_HEIGHT, PANEL_GRAY);
            DrawLine(SCREEN_WIDTH - UI_PANEL_WIDTH, 0, SCREEN_WIDTH - UI_PANEL_WIDTH, SCREEN_HEIGHT, GRAY);
            DrawMaze();
            DrawDecorations();
            DrawPlayer();
            DrawHUD();
            pauseBtn.draw();
            DrawText(gamePaused ? "Play" : "Pause", (SCREEN_WIDTH - UI_PANEL_WIDTH) + 190, 20, 14, WHITE);
            if (gamePaused)
                DrawCenteredTextShadow("PAUSED", (SCREEN_WIDTH - UI_PANEL_WIDTH) / 2, 30, 32, RED);
        }
        else if (currentState == STATE_END)
        {
            UpdateHoversMain();
            DrawRectangle(0, 0, SCREEN_WIDTH - UI_PANEL_WIDTH, SCREEN_HEIGHT, Color{18, 18, 18, 255});
            DrawRectangle(SCREEN_WIDTH - UI_PANEL_WIDTH, 0, UI_PANEL_WIDTH, SCREEN_HEIGHT, PANEL_GRAY);
            DrawLine(SCREEN_WIDTH - UI_PANEL_WIDTH, 0, SCREEN_WIDTH - UI_PANEL_WIDTH, SCREEN_HEIGHT, GRAY);
            DrawMaze();
            DrawDecorations();
            DrawPlayer();
            DrawHUD();
            ShowEndComparison();
            if (!endButtons.empty())
                for (auto &b : endButtons)
                    b.draw();
        }
        else if (currentState == STATE_SCOREBOARD)
        {
            DrawRectangle(100, 100, SCREEN_WIDTH - 200, SCREEN_HEIGHT - 200, Fade(LIGHTGRAY, 0.95f));
            DrawRectangleLines(100, 100, SCREEN_WIDTH - 200, SCREEN_HEIGHT - 200, BLACK);
            DrawCenteredTextShadow("SCOREBOARD", SCREEN_WIDTH / 2, 120, 34, BLUEDARK);
            DrawText("Press Q to return", SCREEN_WIDTH - 250, SCREEN_HEIGHT - 50, 16, DARKGRAY);
            vector<PlayerScore> scores;
            if (currentDifficulty == DIFF_EASY)
                scores = easyScores.getSorted();
            else if (currentDifficulty == DIFF_MEDIUM)
                scores = mediumScores.getSorted();
            else
                scores = hardScores.getSorted();
            if (!scores.empty())
            {
                int startY = 270;
                int cnt = min(static_cast<int>(scores.size()), 10);
                DrawText("RANK", 220, 240, 18, BLACK);
                DrawText("NAME", 320, 240, 18, BLACK);
                DrawText("TIME", 520, 240, 18, BLACK);
                DrawText("DATE", 620, 240, 18, BLACK);
                for (int i = 0; i < cnt; ++i)
                {
                    int y = startY + i * 28;
                    DrawText(TextFormat("%d", i + 1), 220, y, 18, BLACK);
                    DrawText(scores[i].name.c_str(), 320, y, 18, BLACK);
                    DrawText(TextFormat("%.2fs", scores[i].time), 520, y, 18, BLACK);
                    DrawText(scores[i].date.c_str(), 620, y, 18, BLACK);
                }
            }
            else
            {
                DrawText("No scores yet", SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT / 2 - 10, 16, DARKGRAY);
            }
        }
        else if (currentState == STATE_REPLAY)
        {
            DrawRectangle(0, 0, SCREEN_WIDTH - UI_PANEL_WIDTH, SCREEN_HEIGHT, Color{18, 18, 18, 255});
            DrawRectangle(SCREEN_WIDTH - UI_PANEL_WIDTH, 0, UI_PANEL_WIDTH, SCREEN_HEIGHT, PANEL_GRAY);
            DrawLine(SCREEN_WIDTH - UI_PANEL_WIDTH, 0, SCREEN_WIDTH - UI_PANEL_WIDTH, SCREEN_HEIGHT, GRAY);
            DrawMaze();
            DrawDecorations();
            DrawPlayer(); // player at final position
            DrawHUD();
            if (replaySessionIsSolution)
                DrawCenteredTextShadow("OPTIMAL PATH (REVEAL)", (SCREEN_WIDTH - UI_PANEL_WIDTH) / 2, 30, 22, BLUEDARK);
            else
                DrawCenteredTextShadow("REPLAYING YOUR PATH", (SCREEN_WIDTH - UI_PANEL_WIDTH) / 2, 30, 22, BLUEDARK);

            DrawCenteredTextShadow("Press Q to stop", (SCREEN_WIDTH - UI_PANEL_WIDTH) / 2, 60, 14, DARKGRAY);
        }

        EndDrawing();
    }
    UnloadMusicStream(backgroundMusic);
    CloseAudioDevice();

    CloseWindow();
    return 0;
}
