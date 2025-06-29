#pragma once

#include <raylib.h>
#include <vector>
#include <string>

// Constantes del juego
constexpr int SCREEN_WIDTH = 1500;
constexpr int SCREEN_HEIGHT = 900;
constexpr int CLONE_ACTIVATION_TURNS = 6;
constexpr int UI_PANEL_WIDTH = 300;
constexpr float PENTAGON_RADIUS = 40.0f;
constexpr float PENTAGON_DX = PENTAGON_RADIUS * 2.0f;
constexpr float PENTAGON_DY = PENTAGON_RADIUS * 1.5f;

// Colores (podrían moverse a una clase Renderer o UI si se vuelven más complejos)
const Color DARK_GREEN = {30, 35, 40, 255};
const Color UI_PANEL_COLOR = {45, 50, 60, 255};
const Color TEXT_WHITE = {240, 240, 240, 255};
const Color ACCENT_BLUE = {102, 255, 255, 255};
const Color PLAYER_GREEN = {0, 200, 120, 255};
const Color CLONE_BLUE = {90, 120, 250, 255};
const Color WALL_GRAY = {102, 204, 255, 255};
const Color DYNAMIC_WALL_COLOR = {60, 60, 60, 255};
const Color EVEN_CELL_COLOR = {0, 200, 100, 255}; // Verde vivo y legible
const Color ODD_CELL_COLOR = {220, 60, 60, 255}; // Rojo fuerte, sin saturar
const Color PATH_SOLUTION_COLOR = ACCENT_BLUE;


// Estados del juego
enum class GameState
{
    PLAYING,
    GAME_OVER,
    PATH_SHOWN // Podría integrarse o manejarse de otra forma
};


enum class CellType
{
    PATH = 0,
    WALL = 1,
    ALTERNATING_WALL = 2, 
    DYNAMIC_WALL = 3      
};

// Estructura para paredes dinámicas
struct DynamicWall
{
    int row, col;
    int turns_to_open; // Contador de turnos restantes
    CellType original_type = CellType::DYNAMIC_WALL; // Para saber qué era originalmente
};

// Definición para la grid del laberinto
using MazeGrid = std::vector<std::vector<int>>; // Se mantendrá por ahora por la carga desde archivo
using Position = std::pair<int, int>;
using MovementHistory = std::vector<Position>;
