#pragma once
#include <vector>

using MazeGrid = std::vector<std::vector<int>>;

MazeGrid LoadMazeFromFile(const char* filename);