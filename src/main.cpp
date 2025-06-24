#include <iostream>
#include <raylib.h>
#include "pentagon.h"
#include "maze.h"

struct Player {
    int row, col;
    std::vector<std::pair<int, int>> movement_history;
};

enum GameState {
    PLAYING,
    GAME_OVER,
    PATH_SHOWN
};

constexpr int CLONE_ACTIVATION_TURNS = 6; // Cuando se activa el clon

struct DynamicWall {
    int row, col, turns_to_open;
};

int main() {
    const Color darkGreen = {20, 160, 133, 255};
    constexpr int screenWidth = 800;
    constexpr int screenHeight = 600;
    bool showPathSolution = false;

    Player player = {0, 0};
    Player clone = {-1, -1};
    bool clone_active = false;
    int current_turn = 0;
    GameState current_game_state = PLAYING;

    MazeGrid maze = LoadMazeFromFile("maze.txt");
    if (maze.empty() || maze[0].empty()) {
        std::cerr << "Error: No se pudo cargar el laberinto.\n";
        return 1;
    }

    if (maze[player.row][player.col] != 0) {
        std::cerr << "Error: El jugador no puede iniciar en una celda no transitable.\n";
        return 1;
    }

    int rows = maze.size(), cols = maze[0].size();
    float radius = 40;
    float dx = radius * 1.6f, dy = radius * 1.5f;

    std::vector<DynamicWall> dynamic_walls;
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (maze[r][c] == 3) {
                dynamic_walls.push_back({r, c, 3}); // Pared dinámica con 3 turnos para abrir
            }
        }
    }

    auto shortest_path_bfs = ShortestPathBfs(maze, {0, 0}, {rows - 1, cols - 1});

    InitWindow(screenWidth, screenHeight, "Pentagon Maze");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        
        if (current_game_state == PLAYING) {
            int next_row = player.row;
            int next_col = player.col;
            bool moved = false;

            if (IsKeyPressed(KEY_UP)) { next_row--; moved = true; }
            else if (IsKeyPressed(KEY_DOWN)) { next_row++; moved = true; }
            else if (IsKeyPressed(KEY_LEFT)) { next_col--; moved = true; }
            else if (IsKeyPressed(KEY_RIGHT)) { next_col++; moved = true; }

            if (moved) {
                bool valid_position = next_row >= 0 && next_row < rows && next_col >= 0 && next_col < cols;
                bool cellIsBlocked = false;

                if (valid_position) {
                    int cellType = maze[next_row][next_col];

                    if (cellType == 1 || (cellType == 2 && current_turn % 2 != 0)) {
                        cellIsBlocked = true;
                    }

                    for (const auto &dw : dynamic_walls) {
                        if (dw.row == next_row && dw.col == next_col && dw.turns_to_open > 0) {
                            cellIsBlocked = true;
                            break;
                        }
                    }

                    if (clone_active && next_row == clone.row && next_col == clone.col) {
                        cellIsBlocked = true;
                    }

                    if (!cellIsBlocked) {
                        player.row = next_row;
                        player.col = next_col;
                        player.movement_history.push_back({player.row, player.col});
                        current_turn++;

                        for (auto &dw : dynamic_walls) {
                            if (dw.turns_to_open > 0) {
                                dw.turns_to_open--;
                                if (dw.turns_to_open == 0) {
                                    maze[dw.row][dw.col] = 0;
                                }
                            }
                        }

                        if (current_turn >= CLONE_ACTIVATION_TURNS && !clone_active) {
                            clone_active = true;
                            clone.row = player.movement_history[0].first;
                            clone.col = player.movement_history[0].second;
                        }

                        if (clone_active && current_turn > CLONE_ACTIVATION_TURNS) {
                            int index = current_turn - CLONE_ACTIVATION_TURNS;
                            if (index < player.movement_history.size()) {
                                int cr = player.movement_history[index].first;
                                int cc = player.movement_history[index].second;

                                if (!(cr == player.row && cc == player.col)) {
                                    clone.row = cr;
                                    clone.col = cc;
                                }
                            }
                        }

                        if (player.row == rows - 1 && player.col == cols - 1) {
                            current_game_state = GAME_OVER;
                        }
                    }
                }
            }
        }

        if (IsKeyPressed(KEY_S)) {
            showPathSolution = !showPathSolution;
        }

        BeginDrawing();
        ClearBackground(darkGreen);

        DrawText(TextFormat("Turno: %d", current_turn), 20, 20, 20, WHITE);
        if (current_game_state == GAME_OVER) {
            DrawText("¡Has escapado del laberinto!", 20, 50, 25, GOLD);
        } else {
            DrawText("Presiona S para mostrar la solucion BFS", 20, 40, 20, WHITE);
            DrawText("Usa las flechas para mover al jugador", 20, 60, 20, WHITE);
        }

        for (int row = 0; row < rows; row++) {
            for (int col = 0; col < cols; col++) {
                float x = 100 + col * dx + (row % 2) * (dx / 2);
                float y = 100 + row * dy;
                Color c = RAYWHITE;

                switch (maze[row][col]) {
                    case 1: c = GRAY; break;
                    case 2: c = (current_turn % 2 == 0) ? BLUE : YELLOW; break;
                    case 3: {
                        bool dynamicClosed = false;
                        for (const auto &dw : dynamic_walls) {
                            if (dw.row == row && dw.col == col && dw.turns_to_open > 0) {
                                c = DARKGRAY;
                                DrawText(TextFormat("%d", dw.turns_to_open), x - 10, y - 10, 15, WHITE);
                                dynamicClosed = true;
                                break;
                            }
                        }
                        if (!dynamicClosed) c = RAYWHITE;
                        break;
                    }
                }

                DrawPentagon({x, y}, radius, c);
            }
        }

        // Jugador
        float px = 100 + player.col * dx + (player.row % 2) * (dx / 2);
        float py = 100 + player.row * dy;
        DrawPentagon({px, py}, radius * 0.8f, GREEN);

        // Clon
        if (clone_active) {
            float cx = 100 + clone.col * dx + (clone.row % 2) * (dx / 2);
            float cy = 100 + clone.row * dy;
            DrawPentagon({cx, cy}, radius * 0.6f, BLUE);
        }

        // Camino BFS
        if (showPathSolution) {
            if (shortest_path_bfs.empty())
                DrawText("No hay un camino posible (BFS)", 20, 80, 20, RED);
            else {
                for (auto &p : shortest_path_bfs) {
                    float x = 100 + p.second * dx + (p.first % 2) * (dx / 2);
                    float y = 100 + p.first * dy;
                    DrawPentagon({x, y}, radius * 0.7f, RED);
                }
            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
