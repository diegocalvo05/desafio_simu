#include "grid_manager.h"
#include "config.h" // Para CellType, DynamicWall, etc.
#include <fstream>
#include <sstream>
#include <iostream> // Para std::cerr y std::cout (debug)
#include <queue>
#include <algorithm> // Para std::reverse
#include <set>       // Para el BFS con estados (si se usa)
#include <tuple>     // Para std::tuple en BFS
#include <vector>    // Para std::vector de neighbors

GridManager::GridManager() : num_rows(0), num_cols(0)
{
}

bool GridManager::LoadMaze(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Error: No se pudo abrir el archivo de laberinto: " << filename << std::endl;
        return false;
    }

    grid_data.clear();
    dynamic_walls_list.clear();
    std::string line_str;
    int r_idx = 0;
    while (std::getline(file, line_str))
    {
        std::vector<int> row_vec;
        std::stringstream ss(line_str);
        int cell_val;
        int c_idx = 0;
        while (ss >> cell_val)
        {
            row_vec.push_back(cell_val);
            if (cell_val == static_cast<int>(CellType::DYNAMIC_WALL))
            {
                dynamic_walls_list.push_back({r_idx, c_idx, 3, CellType::DYNAMIC_WALL}); 
            }
            c_idx++;
        }
        if (!row_vec.empty())
        {
            grid_data.push_back(row_vec);
        }
        r_idx++;
    }
    file.close();

    if (grid_data.empty() || grid_data[0].empty())
    {
        std::cerr << "Error: El laberinto está vacío o mal formateado." << std::endl;
        num_rows = 0;
        num_cols = 0;
        return false;
    }

    num_rows = grid_data.size();
    num_cols = grid_data[0].size();
    return true;
}

int GridManager::GetCellType(int r, int c) const
{
    if (r < 0 || r >= num_rows || c < 0 || c >= num_cols)
    {
        return -1; 
    }
    return grid_data[r][c];
}

bool GridManager::IsCellWalkable(int r, int c, int current_turn, bool is_player, Position clone_pos, bool clone_is_active) const
{
    if (r < 0 || r >= num_rows || c < 0 || c >= num_cols) return false; 

    if (is_player && clone_is_active && r == clone_pos.first && c == clone_pos.second) return false;

    int cell_type_val = grid_data[r][c];

    if (cell_type_val == static_cast<int>(CellType::WALL)) return false;    

    if (cell_type_val == static_cast<int>(CellType::ALTERNATING_WALL)) return (current_turn % 2 == 0);
    
    if (cell_type_val == static_cast<int>(CellType::DYNAMIC_WALL))
    {
        for (const auto &dw : dynamic_walls_list)
        {
            if (dw.row == r && dw.col == c)
            {
                return dw.turns_to_open <= 0; 
            }
        }
        return false; 
    }
    return true;
}

void GridManager::UpdateDynamicWalls(int current_turn)
{
    for (auto &dw : dynamic_walls_list)
    {
        if (dw.turns_to_open > 0)
        {
            dw.turns_to_open--;
            if (dw.turns_to_open == 0)
            {
                if (dw.row >= 0 && dw.row < num_rows && dw.col >= 0 && dw.col < num_cols)
                {
                    if (grid_data[dw.row][dw.col] == static_cast<int>(CellType::DYNAMIC_WALL)) {
                        grid_data[dw.row][dw.col] = static_cast<int>(CellType::PATH);
                    }
                }
            }
        }
    }
}

bool GridManager::IsCellOpenForPathfinding(
    int r, int c,
    const MazeGrid &current_grid_config, 
    const std::vector<std::vector<bool>> &visited,
    const std::vector<DynamicWall> &initial_dynamic_walls, 
    int turn_when_reaching_cell, 
    int start_turn_of_bfs 
) const
{
    if (r < 0 || r >= num_rows || c < 0 || c >= num_cols || visited[r][c]) return false;
    

    int cell_type_val = current_grid_config[r][c];

    if (cell_type_val == static_cast<int>(CellType::WALL)) return false;
    
    if (cell_type_val == static_cast<int>(CellType::ALTERNATING_WALL))return (turn_when_reaching_cell % 2 == 0);
    
    if (cell_type_val == static_cast<int>(CellType::DYNAMIC_WALL))
    {
        for (const auto &dw_initial_state : initial_dynamic_walls)
        {
            if (dw_initial_state.row == r && dw_initial_state.col == c)
            {
                int turns_to_open_at_bfs_start = dw_initial_state.turns_to_open;
                int steps_taken_in_bfs = turn_when_reaching_cell - start_turn_of_bfs;
                if (turns_to_open_at_bfs_start > steps_taken_in_bfs) {
                    return false; 
                }
                return true; 
            }
        }
        return false; 
    }
    return true;
}

std::vector<Position> GridManager::CalculateShortestPath(Position start_pos, Position end_pos, int current_turn_for_calc) const
{
    std::vector<Position> path;
    if (num_rows == 0 || num_cols == 0) return path;
    if (start_pos.first < 0 || start_pos.first >= num_rows || start_pos.second < 0 || start_pos.second >= num_cols ||
        end_pos.first < 0 || end_pos.first >= num_rows || end_pos.second < 0 || end_pos.second >= num_cols) {
        return path;
    }

    if (start_pos == end_pos) {
        path.push_back(start_pos);
        return path;
    }

    std::vector<std::vector<bool>> visited(num_rows, std::vector<bool>(num_cols, false));
    std::vector<std::vector<Position>> predecessor(num_rows, std::vector<Position>(num_cols, {-1, -1}));
    std::queue<std::tuple<int, int, int>> q;
    std::vector<DynamicWall> dynamic_walls_snapshot = dynamic_walls_list;

    if (!IsCellOpenForPathfinding(start_pos.first, start_pos.second, grid_data, visited, dynamic_walls_snapshot, current_turn_for_calc, current_turn_for_calc)) {
        return path;
    }

    q.push({start_pos.first, start_pos.second, current_turn_for_calc});
    visited[start_pos.first][start_pos.second] = true;

    bool found_path = false;
    while (!q.empty()) {
        auto [r, c, turn_at_current_cell] = q.front();
        q.pop();

        if (r == end_pos.first && c == end_pos.second) {
            found_path = true;
            break;
        }

        int next_turn_for_neighbor = turn_at_current_cell + 1;

        std::vector<Position> potential_neighbors;
        if (r % 2 == 0) {
            potential_neighbors = {
                {r - 1, c},
                {r, c - 1},
                {r, c + 1},
                {r + 1, c - 1},
                {r + 1, c}
            };
        } else {
            potential_neighbors = {
                {r + 1, c},
                {r, c - 1},
                {r, c + 1},
                {r - 1, c - 1},
                {r - 1, c}
            };
        }

        for (const auto& move : potential_neighbors) {
            int nr = move.first;
            int nc = move.second;

            if (IsCellOpenForPathfinding(nr, nc, grid_data, visited, dynamic_walls_snapshot, next_turn_for_neighbor, current_turn_for_calc)) {
                visited[nr][nc] = true;
                predecessor[nr][nc] = {r, c};
                q.push({nr, nc, next_turn_for_neighbor});
            }
        }
    }

    if (found_path) {
        Position current_pos_in_path = end_pos;
        while (current_pos_in_path.first != -1 && current_pos_in_path.second != -1 && current_pos_in_path != start_pos) {
            path.push_back(current_pos_in_path);
            current_pos_in_path = predecessor[current_pos_in_path.first][current_pos_in_path.second];
            if (path.size() > num_rows * num_cols) {
                return {};
            }
        }
        path.push_back(start_pos);
        std::reverse(path.begin(), path.end());
    }

    return path;
}
