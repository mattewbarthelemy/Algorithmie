#include "grid.h"
#include <stdio.h>
#include <string.h>

Grid* CreateGrid(int gridData[20][20], bool CustomTexture) {
    Grid* grid = (Grid*)malloc(sizeof(Grid));
    if (!grid) {
        printf("ERROR: Failed to allocate Grid!\n");
        return NULL;
    }

    memset(grid->cell, 0, sizeof(grid->cell));

    LoadAllCellTextures(CustomTexture);

    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 20; j++) {
            grid->cell[i][j] = CreateCell((sfVector2i) { j, i }, CELL_SIZE, (enum CellType)gridData[i][j], gridData);
            if (!grid->cell[i][j]) {
                printf("ERROR: Failed to create cell at [%d][%d]!\n", i, j);
                for (int x = 0; x <= i; x++) {
                    for (int y = 0; y < (x == i ? j : 20); y++) {
                        if (grid->cell[x][y]) {
                            DestroyCell(grid->cell[x][y]);
                            grid->cell[x][y] = NULL;
                        }
                    }
                }
                free(grid);
                return NULL;
            }
        }
    }

    printf("Grid initiated.\n");
    return grid;
}

void DestroyGrid(Grid* grid) {
    if (!grid) {
        return;
    }

    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 20; j++) {
            if (grid->cell[i][j]) {
                DestroyCell(grid->cell[i][j]);
                grid->cell[i][j] = NULL;
            }
        }
    }

    free(grid);
}

void DrawGrid(sfRenderWindow* window, Grid* grid) {
    if (!window || !grid) {
        return;
    }

    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 20; j++) {
            if (grid->cell[i][j]) {
                DrawCell(window, grid->cell[i][j]);
            }
        }
    }
}