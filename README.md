# Maze Runner – Data Structures Project  
*A C++ Maze Generator & Solver Game using Raylib*

A graphical **maze game** built in **C++** using the **Raylib** library. The game generates random mazes, lets the player navigate from start to goal, and then compares the player’s path with the optimal path computed using **Breadth‑First Search (BFS)**. Multiple data structures (stack, queue, linked list, hash table, BST, vectors) are implemented to power the game mechanics for a Data Structures course.

***

## Requirements

1. **C++ Compiler (C++17 or later)**  
   - Any modern compiler (MSVC, g++, clang++) will work.

2. **Raylib (Graphics & Audio Library)**  
   - Install Raylib for your platform:  
     https://www.raylib.com

3. **Build Tools / IDE**  
   - Visual Studio, Visual Studio Code, CLion, or any C++ IDE configured with Raylib.

***

## Setup Instructions

### 1. Install Raylib

Follow the official Raylib installation guide for your OS and compiler (Windows / Linux / macOS).  
- For Windows + MSVC, you can use the prebuilt binaries and Visual Studio templates.  
- For VS Code, configure include paths and linker settings as described in the Raylib docs.

The tutorial we followed for Windows and Visual Studio 2026:
https://youtu.be/UiZGTIYld1M?si=887pEBkqj-ts2IVh

### 2. Clone or Download the Project

```bash
git clone <your-repo-url>.git
cd <your-repo-folder>
```

Ensure `maze-runner.cpp` is in the project directory.

### 3. Add a Background Music File

Place a music file named `bg_music.mp3` in the same folder as the executable (or update the filename in the code where `LoadMusicStream` is called).

***

## Build and Run Instructions

### Example (Windows + MSVC, Developer Command Prompt)

```bash
cl /EHsc /std:c++17 maze-runner.cpp /I"path\to\raylib\include" ^
   /link /LIBPATH:"path\to\raylib\lib" raylib.lib
maze-runner.exe
```

If you are using Visual Studio 2026 (or 2022, etc.), you can use the run button.

### Example (g++ on Linux / WSL)

```bash
g++ maze-runner.cpp -o maze_runner \
    -std=c++17 \
    -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
./maze_runner
```
Adjust the include and lib paths to match your Raylib installation.

***

## Gameplay Overview

- Enter your **name**.  
- Choose a **maze size**: 20×20, 30×30, or 40×40.  
- Choose a **difficulty**: Easy, Medium, Hard.  
- After a 3‑2‑1 countdown, navigate the maze using the arrow keys from the **red starting point** to the **cheese (goal)**.  
- The game tracks **time**, **moves**, and visually shows a **difficulty bar** on the right‑side HUD.  
- When you reach the goal:
  - The game computes the **shortest path** using BFS.
  - It compares your path with the optimal path.
  - If both match, you get a **“Perfect Run”** message.  
  - Otherwise, you can:
    - **Reveal Optimal Path** (auto-solver replay).  
    - **Replay Your Path** step‑by‑step.

Your run (name, time, difficulty, date) is then stored in `scores.txt` and displayed in the **scoreboard**, sorted by time using a Binary Search Tree.

***

## Gameplay Controls

**Menus & Navigation**

- Mouse: Click buttons (Start Game, Scoreboard, Exit, etc.).  
- `Q`: Go back (from name, size, difficulty, and scoreboard screens).

**In-Game (Maze Screen)**

- Arrow keys: Move the player through the maze.  
- `Q`: Quit the current run and go back to the Home screen (does NOT save the score).  
- Pause button (top-right): Pause/resume the game and music.  
- `M` button: Toggle music ON/OFF.

**Replay / Optimal Path Screen**

- `OPTIMAL PATH`: Show the BFS shortest path as a golden highlight.  
- `REPLAY MY PATH`: Replay the exact path you took.  
- `Q`: Stop the replay and return to the End screen.

***

## Data Structures & Algorithms Highlight

This project was built for a **Data Structures** course and showcases:

- Custom **Stack** (DFS maze generation)  
- Custom **Queue** (BFS shortest path)  
- **Linked List** (player move history & replay)  
- **Hash Table** (visited cells during generation)  
- **Binary Search Tree** (scoreboard per difficulty)  
- **Dynamic 2D Array** using `vector<vector<Cell>>` (maze grid)  
- **DFS** for maze generation and **BFS** for pathfinding  
- File-based persistence of scores (`scores.txt`)

***

## Game Preview

```
<img width="1004" height="712" alt="Game_screen_easy-small" src="https://github.com/user-attachments/assets/2fdc8e36-d4a3-4cfb-ac7c-d279e8e56f86" />

```

***

## Troubleshooting

1. **Raylib Not Found (linker errors):**  
   - Check that your compiler’s include and lib paths point to the correct Raylib directories.  
   - Ensure `raylib.lib` (Windows) or `-lraylib` (Linux/macOS) is linked correctly.

3. **Scores Not Saved:**  
   - Scores are only saved when you actually reach the goal.  
   - Quitting with `Q` during the game returns to Home without recording the run.

Hope you enjoy exploring the maze and the data structures behind it!
