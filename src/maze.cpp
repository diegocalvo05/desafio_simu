#include "maze.h"
#include <fstream>
#include <sstream>
#include <iostream>

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