#include "grid.h"

Grid* CreateGrid(int gridData[20][20]) {
    Grid* grid = (Grid*)malloc(sizeof(Grid));
    if (!grid) {
        return NULL;
    }

    LoadAllCellTextures();

    // Initialize each cell in the grid
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 20; j++) {
            grid->cell[i][j] = CreateCell((sfVector2i) { j, i }, CELL_SIZE, (enum CellType)gridData[i][j], gridData);
            if (!grid->cell[i][j]) {
                // CORRECTION: Cleanup en cas d'échec
                // Ne pas free grid->cell[x] car c'est un tableau statique
                for (int x = 0; x <= i; x++) {
                    for (int y = 0; y < (x == i ? j : 20); y++) {
                        DestroyCell(grid->cell[x][y]);
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

    // CORRECTION: Libérer seulement les Cell* individuelles
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 20; j++) {
            if (grid->cell[i][j]) {
                DestroyCell(grid->cell[i][j]);
                grid->cell[i][j] = NULL;  // Sécurité
            }
        }
    }

    // CORRECTION: Libérer seulement la Grid elle-même
    // PAS grid->cell[i] ni grid->cell (tableau statique)
    free(grid);
}

void DrawGrid(sfRenderWindow* window, Grid* grid) {
    if (window && grid) {
        for (int i = 0; i < 20; i++) {
            for (int j = 0; j < 20; j++) {
                if (grid->cell[i][j]) {
                    DrawCell(window, grid->cell[i][j]);
                }
            }
        }
    }
}