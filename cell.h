#pragma once

#include "basics.h"

#include <SFML/System/Vector2.h>

typedef struct Cell {
    sfVector2i coord;
    enum CellType type;
    sfSprite *sprite;
} Cell;

void LoadAllCellTextures();

Cell* CreateCell(sfVector2i cellCoord, float size, enum CellType type, int grid[20][20]);

void DestroyCell(Cell* cell);

void DrawCell(sfRenderWindow* window, Cell* cell);

void GetRequiredSpriteForCell(Cell* cell, int grid[20][20]);