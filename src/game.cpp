#include "game.h"
#include "config.h" // Para constantes globales
#include <iostream>  // Para std::cerr

Game::Game(const std::string& maze_filename)
    : player(0, 0), // Posición inicial por defecto, se ajustará en InitializeGame
      clone(-1, -1),
      renderer(SCREEN_WIDTH, SCREEN_HEIGHT, "Escape the Grid - Modular"),
      current_turn(0),
      current_game_state(GameState::PLAYING),
      show_path_solution(false),
      maze_render_start_x(0.0f),
      maze_render_start_y(0.0f)
{
    InitializeGame(maze_filename);
}

Game::~Game()
{
    // El destructor de Renderer se encargará de CloseWindow()
}

void Game::InitializeGame(const std::string& maze_filename)
{
    if (!grid_manager.LoadMaze(maze_filename))
    {
        std::cerr << "Error crítico: No se pudo cargar el laberinto '" << maze_filename << "'." << std::endl;
        current_game_state = GameState::GAME_OVER; 
        return;
    }

    bool found_start = false;
    for (int r = 0; r < grid_manager.GetRows(); ++r) {
        for (int c = 0; c < grid_manager.GetCols(); ++c) {
            if (grid_manager.GetCellType(r,c) == static_cast<int>(CellType::PATH) || 
                (grid_manager.GetCellType(r,c) == static_cast<int>(CellType::ALTERNATING_WALL) && (0 % 2 == 0)) ) { // Asumiendo que el turno 0 es par para ALTERNATING_WALL
                // Considerar DYNAMIC_WALL si turns_to_open es 0 inicialmente (aunque LoadMaze las convierte a PATH)
                player.Move(r,c);
                found_start = true;
                break;
            }
        }
        if (found_start) break;
    }

    if (!found_start) {
         std::cerr << "Error: No se encontró una posición inicial válida para el jugador." << std::endl;
        current_game_state = GameState::GAME_OVER; 
        return;
    }
    
    player.RecordMovement(); 

    CalculateMazeRenderOffsets();
    current_turn = 0; 
    clone.SetActive(false); 
    current_game_state = GameState::PLAYING;
}

void Game::CalculateMazeRenderOffsets()
{
    if (grid_manager.GetRows() == 0 || grid_manager.GetCols() == 0) return;

    float total_maze_width = (grid_manager.GetCols() - 0.5f) * PENTAGON_DX; // Aproximación considerando el offset
    float total_maze_height = (grid_manager.GetRows() -1 ) * PENTAGON_DY + PENTAGON_RADIUS * 2; // Alto total

    float drawable_screen_width = SCREEN_WIDTH - 230; // UI panel width
    
    maze_render_start_x = 230 + (drawable_screen_width - total_maze_width) / 2.0f;
    // Asegurar que el inicio X no sea menor que el ancho del panel UI si el laberinto es muy ancho
    if (total_maze_width > drawable_screen_width) maze_render_start_x = 230 + PENTAGON_RADIUS;


    maze_render_start_y = (SCREEN_HEIGHT - total_maze_height) / 2.0f;
    if (total_maze_height > SCREEN_HEIGHT) maze_render_start_y = PENTAGON_RADIUS;


}


void Game::Run()
{
    while (!WindowShouldClose() && current_game_state != GameState::GAME_OVER) 
    {
        ProcessInput();
        if (current_game_state == GameState::PLAYING) { 
            Update();
        }
        Render();
    }
    while (!WindowShouldClose() && current_game_state == GameState::GAME_OVER) {
        Render(); 
         if (IsKeyPressed(KEY_R)) { 
            InitializeGame("maze.txt"); 
        }
    }
}

void Game::ProcessInput()
{
    if (current_game_state == GameState::PLAYING)
    {
        int current_player_row = player.GetRow();
        int current_player_col = player.GetCol();
        int next_player_row = current_player_row;
        int next_player_col = current_player_col;
        bool player_attempted_move = false;

        // TODO: Implementar movimiento pentagonal (Q,W,E,A,D)
        // Por ahora, mantenemos el movimiento cardinal simple y lo adaptaremos.
        // El movimiento cardinal simple NO FUNCIONARÁ BIEN con el layout hexagonal de pentágonos.
        // Se necesita definir cómo las teclas cardinales se mapean a los 5 vecinos.

        if (IsKeyPressed(KEY_RIGHT)) { 
            // Moverse a la derecha en la misma fila
            next_player_col++; 
            player_attempted_move = true;
        } else if (IsKeyPressed(KEY_LEFT)) {
            // Moverse a la izquierda en la misma fila
            next_player_col--;
            player_attempted_move = true;
        } else if (IsKeyPressed(KEY_UP)) { 
            // Esto es más complicado. Depende de si la fila actual es par o impar
            // y a cuál de los dos vecinos superiores (o uno si es el borde) se quiere mover.
            // Para un sistema de 5 direcciones, necesitaremos teclas dedicadas (QWEAD).
            // TEMPORALMENTE: KEY_UP podría ir al vecino superior-derecha en filas pares, superior-izquierda en filas impares.
            next_player_row--;
            if (current_player_row % 2 == 0) { // Fila par (punta arriba), moviendo hacia arriba
                // next_player_col se mantiene o aumenta (para sup-der)
            } else { // Fila impar (punta abajo), moviendo hacia arriba
                 next_player_col--; // para sup-izq
            }
            player_attempted_move = true;

        } else if (IsKeyPressed(KEY_DOWN)) {
            // Similarmente complicado.
            // TEMPORALMENTE: KEY_DOWN podría ir al vecino inferior-derecha en filas pares, inferior-izquierda en filas impares.
            next_player_row++;
            if (current_player_row % 2 == 0) { // Fila par (punta arriba), moviendo hacia abajo
                // next_player_col se mantiene o aumenta (para inf-der)
            } else { // Fila impar (punta abajo), moviendo hacia abajo
                next_player_col--; // para inf-izq
            }
            player_attempted_move = true;
        }

        if (player_attempted_move)
        {
            if (grid_manager.IsCellWalkable(next_player_row, next_player_col, current_turn + 1, true, clone.GetPos(), clone.IsActive()))
            {
                player.Move(next_player_row, next_player_col);
                player.RecordMovement();
                current_turn++; 

                grid_manager.UpdateDynamicWalls(current_turn);

                if (!clone.IsActive() && current_turn >= CLONE_ACTIVATION_TURNS) {
                    clone.SetActive(true);
                }

                if (clone.IsActive()) {
                    clone.UpdatePositionFromHistory(player.GetMovementHistory(), current_turn);
                    if (player.GetPos() == clone.GetPos()) {
                         current_game_state = GameState::GAME_OVER; 
                         std::cout << "Game Over: Colisión con el clon en el turno " << current_turn << std::endl;
                         return; 
                    }
                }

                if (player.GetRow() == grid_manager.GetRows() - 1 && player.GetCol() == grid_manager.GetCols() - 1) {
                    current_game_state = GameState::GAME_OVER;
                    std::cout << "¡Has escapado en el turno " << current_turn << "!" << std::endl;
                }
            }
        }
    }

    if (IsKeyPressed(KEY_S)) {
        show_path_solution = !show_path_solution;
    }
     if (current_game_state == GameState::GAME_OVER && IsKeyPressed(KEY_R)) {
        InitializeGame("maze.txt"); 
    }
}

void Game::Update()
{
    // Lógica de actualización principal movida a ProcessInput post-movimiento.
}

void Game::Render()
{
    renderer.BeginDrawingSequence();

    renderer.DrawGrid(grid_manager, current_turn, maze_render_start_x, maze_render_start_y);
    
    // Pasar player.GetRow() para la rotación correcta del pentágono de la entidad
    renderer.DrawEntity(player, player.GetRow(), maze_render_start_x, maze_render_start_y, 0.8f, PLAYER_GREEN, "TÚ");
    
    if (clone.IsActive()) {
        // Pasar clone.GetRow() para la rotación correcta del pentágono de la entidad
        renderer.DrawEntity(clone, clone.GetRow(), maze_render_start_x, maze_render_start_y, 0.6f, CLONE_BLUE, "CLON");
    }

    if (show_path_solution) {
        std::vector<Position> shortest_path = grid_manager.CalculateShortestPath(
            player.GetPos(),
            {grid_manager.GetRows() - 1, grid_manager.GetCols() - 1},
            current_turn 
        );
        if (!shortest_path.empty()) {
            renderer.DrawPath(shortest_path, maze_render_start_x, maze_render_start_y, 0.65f, PATH_SOLUTION_COLOR);
        }
    }

    renderer.DrawUI(current_turn, current_game_state);

    renderer.EndDrawingSequence();
}
