#include "renderer.h"
#include "config.h"
#include <raylib.h>
#include <vector>
#include <cmath>

// MultipleFiles/renderer.cpp

namespace {
    void DrawLocalPentagon(Vector2 center, float radius, float rotation, Color color) {
        std::vector<Vector2> vertices(6); 

        float half_width = radius * 0.9f;
        float base_height = radius * 0.6f;
        float roof_height = radius * 0.9f;

        Vector2 base_vertices[] = {
            {0, -roof_height},
            {half_width, -base_height * 0.3f},
            {half_width, base_height},
            {-half_width, base_height},
            {-half_width, -base_height * 0.3f}
        };

        float rot_rad = rotation * DEG2RAD;
        float cos_rot = cosf(rot_rad);
        float sin_rot = sinf(rot_rad);

        for (int i = 0; i < 5; i++) {
            float x = base_vertices[i].x;
            float y = base_vertices[i].y;

            vertices[i].x = center.x + (x * cos_rot - y * sin_rot);
            vertices[i].y = center.y + (x * sin_rot + y * cos_rot);
        }

        DrawTriangleFan(vertices.data(), 5, color);
        
        Color outline_color = ColorBrightness(color, -0.15f); 
        for (int i = 0; i < 5; i++) {
            int next = (i + 1) % 5;
            DrawLineV(vertices[i], vertices[next], outline_color);
        }
    }
}


Renderer::Renderer(int screen_w, int screen_h, const std::string& window_title) {
    InitWindow(screen_w, screen_h, window_title.c_str());
    SetTargetFPS(60);
}

Renderer::~Renderer() {
    CloseWindow();
}

void Renderer::BeginDrawingSequence() {
    BeginDrawing();
    ClearBackground(DARK_GREEN);
}

void Renderer::EndDrawingSequence() {
    EndDrawing();
}

void Renderer::DrawPentagonCell(Vector2 center, float radius, float rotation, Color color) const {
    DrawLocalPentagon(center, radius, rotation, color);
}

void Renderer::DrawGrid(const GridManager& grid, int current_turn, float start_x, float start_y) const {
    for (int r = 0; r < grid.GetRows(); ++r) {
        for (int c = 0; c < grid.GetCols(); ++c) {
            float offset = ((r / 2) % 2 == 0) ? 0.0f : PENTAGON_DX / 2.0f;
            float pent_center_x = start_x + c * PENTAGON_DX + offset;
            float pent_center_y = start_y + r * PENTAGON_DY;

            Color cell_color = RAYWHITE;
            int cell_type_val = grid.grid_data[r][c];

            if (cell_type_val == static_cast<int>(CellType::WALL)) {
                cell_color = WALL_GRAY;
            } else if (cell_type_val == static_cast<int>(CellType::ALTERNATING_WALL)) {
                cell_color = (current_turn % 2 == 0) ? ODD_CELL_COLOR : EVEN_CELL_COLOR;
            } else if (cell_type_val == static_cast<int>(CellType::DYNAMIC_WALL)) {
                bool found_in_dynamic_list = false;
                for (const auto& dw : grid.dynamic_walls_list) {
                    if (dw.row == r && dw.col == c && dw.turns_to_open > 0) {
                        cell_color = DYNAMIC_WALL_COLOR;
                        DrawText(TextFormat("%d", dw.turns_to_open), 
                                 pent_center_x - MeasureText(TextFormat("%d", dw.turns_to_open), 15) / 2.0f,
                                 pent_center_y - 7, 15, WHITE);
                        found_in_dynamic_list = true;
                        break;
                    }
                }
                if (!found_in_dynamic_list) {
                    cell_color = RAYWHITE;
                }
            }

            float rotation = (r % 2 == 0) ? 0.0f : 180.0f;
            DrawPentagonCell({pent_center_x, pent_center_y}, PENTAGON_RADIUS, rotation, cell_color);

            if (r == 0 && c == 0) {
                DrawCircle(pent_center_x, pent_center_y, 8, GREEN);
            }

            if (r == grid.GetRows() - 1 && c == grid.GetCols() - 1) {
                DrawCircle(pent_center_x, pent_center_y, 8, RED);
            }
        }
    }
}


void Renderer::DrawEntity(const Entity& entity, int entity_row, float start_x, float start_y, float radius_scale, Color color, const std::string& label) const {
    if (!entity.IsActive()) return;

    float offset = ((entity_row / 2) % 2 == 0) ? 0.0f : PENTAGON_DX / 2.0f;
    float entity_center_x = start_x + entity.GetCol() * PENTAGON_DX + offset;
    float entity_center_y = start_y + entity_row * PENTAGON_DY;

    float rotation = (entity_row % 2 == 1) ? 180.0f : 0.0f;
    DrawPentagonCell({entity_center_x, entity_center_y}, PENTAGON_RADIUS * radius_scale, rotation, color);

    if (!label.empty()) {
        DrawText(label.c_str(), 
                 entity_center_x - MeasureText(label.c_str(), 14) / 2.0f,
                 entity_center_y - PENTAGON_RADIUS * radius_scale - 12,
                 14, color);
    }
}

void Renderer::DrawPath(const std::vector<Position>& path, float start_x, float start_y, float radius_scale, Color color) const {
    for (const auto& p : path) {
        int path_row = p.first;
        int path_col = p.second;
        float offset = ((path_row / 2) % 2 == 0) ? 0.0f : PENTAGON_DX / 2.0f;
        float pent_center_x = start_x + path_col * PENTAGON_DX + offset;
        float pent_center_y = start_y + path_row * PENTAGON_DY;

        float rotation = (path_row % 2 == 1) ? 180.0f : 0.0f;
        DrawPentagonCell({pent_center_x, pent_center_y}, PENTAGON_RADIUS * radius_scale, rotation, color);
    }
}

void Renderer::DrawUI(int current_turn, GameState game_state, Position player_pos, Position clone_pos) const
 {
    DrawRectangle(0, 0, UI_PANEL_WIDTH, SCREEN_HEIGHT, UI_PANEL_COLOR);
    DrawText("ESCAPE THE GRID", 20, 20, 22, ACCENT_BLUE);
    DrawText(TextFormat("Turno: %d", current_turn), 20, 60, 18, TEXT_WHITE);

    DrawText("Controles:", 20, 100, 18, TEXT_WHITE);
    DrawText("- A, D: Izquierda, Derecha", 20, 130, 16, TEXT_WHITE);
    DrawText("- Q, W, E: Mov. Arriba", 20, 150, 16, TEXT_WHITE);
    DrawText("- Z, X, C: Mov. Abajo", 20, 170, 16, TEXT_WHITE);
    DrawText("- S: Mostrar/Ocultar Soluci\xC3\xB3n", 20, 190, 16, TEXT_WHITE);

    DrawText("Objetivo:", 20, 210, 18, TEXT_WHITE);
    DrawText("- Alcanza la esquina inferior derecha.", 20, 240, 14, GRAY);
    DrawText("- Evita al Clon.", 20, 260, 14, GRAY);

    if (game_state == GameState::GAME_OVER) {
    if (player_pos == clone_pos) {
        DrawText("GAME OVER", 20, SCREEN_HEIGHT / 2.0f + 40, 22, RED);
    } else {
        DrawText("¡HAS ESCAPADO!", 20, SCREEN_HEIGHT / 2.0f + 40, 22, PLAYER_GREEN);
    }
}

    DrawText("R: Reiniciar", 20, SCREEN_HEIGHT - 40, 16, TEXT_WHITE);
}

