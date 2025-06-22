#include <iostream>
#include <raylib.h>
#include "pentagon.h"
#include "maze.h"

int main() 
{
    const Color darkGreen = {20, 160, 133, 255};
    constexpr int screenWidth = 800;
    constexpr int screenHeight = 600;
    bool showPathSolution = false;

    MazeGrid maze = LoadMazeFromFile("maze.txt");

    if (maze.empty() || maze[0].empty()) {
        std::cerr << "Error: El archivo maze.txt no existe, está vacío o tiene formato incorrecto.\n";
        return 1;
    }

    
    int rows = maze.size(), cols = maze[0].size();
    float radius = 40;
    float dx = radius * 1.6f, dy = radius * 1.5f;
    auto path = ShortestPathBfs(maze, {0, 0}, {rows-1, cols-1});

    InitWindow(screenWidth, screenHeight, "Pentagon Maze");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(darkGreen);

        DrawText("Presiona S para mostrar la solucion", 20, 20, 20, WHITE);

        if (IsKeyPressed(KEY_S)) {
            showPathSolution = true;
        }

        for (int row = 0; row < rows; row++)
        for (int col = 0; col < cols; col++) {
            float x = 100 + col * dx + (row % 2) * (dx / 2);
            float y = 100 + row * dy;
            Color c = maze[row][col] == 1 ? GRAY : RAYWHITE;
            DrawPentagon({x, y}, radius, c);
        }
        
        // show solution path
        if (showPathSolution) {
            if(path.empty())
                DrawText("No hay un camino posible", 20, 50, 20, RED);
            else {
                for (auto& p : path) {
                    float x = 100 + p.second * dx + (p.first % 2) * (dx / 2);
                    float y = 100 + p.first * dy;
                    DrawPentagon({x, y}, radius * 0.7f, RED);
                }
            }
        }

        EndDrawing();
    }
    CloseWindow();
}