#pragma once

#include <SFML/System.h>

#include "basics.h"
#include "cell.h"

typedef struct Grid
{
    Cell* cell[20][20];
} Grid;

Grid* CreateGrid(int gridData[20][20]);

void DestroyGrid(Grid* grid);

void DrawGrid(sfRenderWindow* window, Grid* grid);
