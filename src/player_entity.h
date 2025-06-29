#pragma once

#include "config.h" 
#include <vector>
#include <string>

class Entity
{
public:
    Position pos;
    bool active;

    Entity(int r, int c, bool start_active = false) : pos({r, c}), active(start_active) {}
    virtual ~Entity() = default;

    virtual void Move(int next_row, int next_col) {
        pos.first = next_row;
        pos.second = next_col;
    }

    int GetRow() const { return pos.first; }
    int GetCol() const { return pos.second; }
    Position GetPos() const { return pos; }
    bool IsActive() const { return active; }
    void SetActive(bool val) { active = val; }
};

// Clase para el Jugador
class Player : public Entity
{
public:
    MovementHistory movement_history;

    Player(int r, int c) : Entity(r, c, true) { 
    }

    void RecordMovement() {
        movement_history.push_back(pos);
    }

    const MovementHistory& GetMovementHistory() const {
        return movement_history;
    }
};

class Clone : public Entity
{
public:
    Clone(int r = -1, int c = -1) : Entity(r, c, false) {}

    void UpdatePositionFromHistory(const MovementHistory& player_history, int current_turn);
};

