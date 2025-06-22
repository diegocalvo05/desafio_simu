#pragma once
#include <vector>


using MazeGrid = std::vector<std::vector<int>>;

MazeGrid LoadMazeFromFile(const char* filename);
bool IsCellValid(int x, int y, const MazeGrid& grid, const std::vector<std::vector<bool>>& visited);
std::vector<std::pair<int, int>> ShortestPathBfs(const MazeGrid& grid, std::pair<int, int> start, std::pair<int, int> end);