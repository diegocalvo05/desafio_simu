#include "maze.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <queue>
#include <algorithm>

// Loads a maze from a txt file and returns it as a mazeGrid (vector<vector<int>>))
MazeGrid LoadMazeFromFile(const char* filename) {
    MazeGrid maze;
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "No se pudo abrir el archivo: " << filename << std::endl;
        return maze;
    }

    std::string line;

    while (std::getline(file, line)) {
        std::cout << "Leyendo línea: " << line << std::endl; // Debug
        std::istringstream iss(line);
        std::vector<int> row;
        int val;
        while (iss >> val) row.push_back(val);
        if (!row.empty()) maze.push_back(row);
    }

    std::cout << "Filas leídas: " << maze.size() << std::endl; // Debug
    return maze;
}

// Check if the cell (x, y) is valid for the maze
bool IsCellValid(int x, int y, const MazeGrid& grid, const std::vector<std::vector<bool>>& visited) {
    int n = grid.size();
    int m = grid[0].size();

    return (x >= 0 && y >= 0 && x < n && y < m && grid[x][y] == 0 && !visited[x][y]);
}

std::vector<std::pair<int, int>> ShortestPathBfs(const MazeGrid& grid, std::pair<int, int> start, std::pair<int, int> end) {
    int n = grid.size();
    int m = grid[0].size();

    std::vector<std::vector<bool>> visited(n, std::vector<bool>(m, false));
    std::vector<std::vector<std::pair<int, int>>> predecessor(n, std::vector<std::pair<int, int>>(m, {-1, -1}));

    std::queue<std::pair<int, int>> q;
    q.push(start);
    visited[start.first][start.second] = true;

    const int dx[] = {-1, 1, 0, 0}; // up, down
    const int dy[] = {0, 0, -1, 1}; // left, right

    while (!q.empty()) {
        auto [x, y] = q.front(); q.pop();

        if (std::make_pair(x, y) == end) break;

        for (int i = 0; i < 4; ++i) {
            int nx = x + dx[i];
            int ny = y + dy[i];

            if (IsCellValid(nx, ny, grid, visited)) {
                visited[nx][ny] = true;
                predecessor[nx][ny] = {x, y};
                q.push({nx, ny});
            }
        }
    }

    // backtrack to find the path
    std::vector<std::pair<int, int>> path;
    if (!visited[end.first][end.second]) return path; // no path found

    std::pair<int, int> current = end;
    while (current != start) {
        path.push_back(current);
        current = predecessor[current.first][current.second];
    }

    path.push_back(start);
    std::reverse(path.begin(), path.end());

    return path;
}