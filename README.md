# Maze Runner – Data Structures Project  
*A C++ Maze Generator & Solver Game using Raylib*

This repository contains **Maze Runner**, a GUI-based maze game built in **C++** using **Raylib**.  
Created as a **Data Structures & Algorithms (DSA)** project, it demonstrates custom implementations of stacks, queues, linked lists, BSTs, hash tables, and pathfinding — integrated into a playable interactive maze.

---

## Overview

Maze Runner generates random mazes, allows the player to navigate from start to exit, and then compares the player's path to the optimal **BFS shortest path**.  
Players can replay their own run, reveal the optimal route, and save scores in a file-backed scoreboard that is also stored in BSTs in memory.

---

## Key Features

### Maze & Gameplay
- Random maze generation using DFS with backtracking  
- Maze sizes: 20×20, 30×30, 40×40  
- Difficulty modes: Easy, Medium, Hard  
- Timer, movement counter, difficulty indicator  
- Player path recording and replay  
- Perfect run detection (player path equals optimal BFS path)  
- Animated:
  - Replay My Path  
  - Reveal Optimal Path  

### Score System
- Scores saved to `scores.txt`  
- Score fields include: name, time, difficulty, date  
- Separate BST for each difficulty  
- In-order traversal returns sorted scores  

### Additional Features
- Background music with toggle and pause  
- Multiple screens:
  - Home  
  - Name Input  
  - Size Select  
  - Difficulty Select  
  - Game Play  
  - End Screen  
  - Scoreboard  
- Quick navigation using the **Q** key  

---

## Data Structures & Algorithms Used

### Dynamic 2D Array
Stores the maze grid using `vector<vector<Cell>>`, with size chosen at runtime.

### Custom Stack — `SimpleStack<T>`
Used for DFS-based maze generation.

### Custom Queue — `SimpleQueue<T>`
Used for BFS shortest path computation.

### Custom Hash Table — `VisitHashTable`
Tracks visited cells during maze generation and solving.

### Linked List — `LinkedList<PlayerMove>`
Stores player movements for replay animation.

### Binary Search Tree — `ScoreBST`
Maintains sorted scores for each difficulty.

### Vector + Sorting
Used to select an appropriate maze based on difficulty ranges.

### Algorithms
- Randomized DFS → Maze generation  
- BFS → Optimal pathfinding  

---

## Controls

### Menu Screens
| Action | Control |
|--------|---------|
| Click buttons | Mouse |
| Back | Q |
| Scoreboard → Home | Q |

### In-Game
| Action | Control |
|--------|---------|
| Move | Arrow Keys |
| Pause / Resume | Pause Button |
| Quit Run | Q (score not saved) |

### Replay / Optimal Path
| Action | Control |
|--------|---------|
| Replay player path | Button |
| Show optimal path | Button |
| Stop replay | Q |

---

## Build Instructions

### Requirements
- Raylib 3.x or newer  
- A C++ compiler supporting C++17 (g++, clang++, MSVC)

### Compile (Linux example using g++)
```bash
g++ maze-runner.cpp -o maze_runner \
    -std=c++17 \
    -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
```

### Music File
Place a file named:

```
bg_music.mp3
```

in the working directory (or update the filename in the source code).

### Run
```bash
./maze_runner
```

---

## Files

| File | Description |
|------|-------------|
| `maze-runner.cpp` | Full game source including all DS implementations |
| `scores.txt` | Scoreboard file generated at runtime |

