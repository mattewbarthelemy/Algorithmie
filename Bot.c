#include "Bot.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// ============================================================================
// CONFIGURATION DES COÛTS
// ============================================================================

// Coûts pour le système d'évitement d'obstacles
#define COST_MOVE 1      // Coût d'un mouvement normal
#define COST_JUMP 3      // Coût d'un saut (obstacle = point faible)

// Ajustez ces valeurs pour changer le comportement:
// - COST_JUMP = 1  → Pas de préférence (chemin le plus court)
// - COST_JUMP = 3  → Évite les obstacles de façon équilibrée (recommandé)
// - COST_JUMP = 10 → Évite fortement les obstacles (détours très longs)

// ============================================================================
// FONCTIONS DE BASE DU BOT
// ============================================================================

struct Bot* CreateBot()
{
    struct Bot* bot = (struct Bot*)malloc(sizeof(struct Bot));
    memset(bot, 0, sizeof(struct Bot));

    bot->position = (sfVector2i){ 0, 0 };

    bot->sprite = sfSprite_create();
    sfTexture* tex = sfTexture_createFromFile("./Assets/Characters/Bot01.png", NULL);
    sfSprite_setTexture(bot->sprite, tex, sfTrue);

    float scale = ((float)CELL_SIZE / 24.f) * 0.75f;
    sfSprite_setScale(bot->sprite, (sfVector2f) { scale, scale });

    bot->MoveQueue[0].type = INVALID;
    return bot;
}

void SpawnBotAtStartCell(struct Bot* bot, Grid* grid)
{
    for (int i = 0; i < GRID_ROWS; i++)
    {
        for (int j = 0; j < GRID_COLS; j++)
        {
            if (grid->cell[i][j]->type == START)
            {
                bot->position = grid->cell[i][j]->coord;
                sfVector2f startCelPosition = sfSprite_getPosition(grid->cell[i][j]->sprite);
                startCelPosition.x += 5.f;
                startCelPosition.y += 5.f;
                sfSprite_setPosition(bot->sprite, startCelPosition);
                return;
            }
        }
    }
}

void DestroyBot(struct Bot* bot)
{
    if (!bot) return;
    if (bot->sprite) sfSprite_destroy(bot->sprite);
    free(bot);
}

void DrawBot(sfRenderWindow* window, struct Bot* bot)
{
    if (!window || !bot || !bot->sprite) return;
    sfRenderWindow_drawSprite(window, bot->sprite, NULL);
}

int MoveBot(struct Bot* bot, Grid* grid, enum MovementType type, enum Direction direction)
{
    int distance = (type == JUMP) ? 2 : 1;
    sfVector2i newPosition = bot->position;

    switch (direction)
    {
    case NORTH: newPosition.y -= distance; break;
    case EAST:  newPosition.x += distance; break;
    case SOUTH: newPosition.y += distance; break;
    case WEST:  newPosition.x -= distance; break;
    default: break;
    }

    // Vérification des limites de la grille
    if (newPosition.x < 0 || newPosition.x >= GRID_COLS ||
        newPosition.y < 0 || newPosition.y >= GRID_ROWS)
    {
        return DEAD;
    }

    enum CellType destinationCellType =
        grid->cell[newPosition.y][newPosition.x]->type;

    if (destinationCellType == EMPTY)
    {
        return DEAD;
    }

    if (destinationCellType == OBSTACLE && type != JUMP)
        return NOTHING;

    bot->position = newPosition;

    sfVector2f newSpritePosition =
        sfSprite_getPosition(grid->cell[newPosition.y][newPosition.x]->sprite);
    newSpritePosition.x += 5.f;
    newSpritePosition.y += 5.f;
    sfSprite_setPosition(bot->sprite, newSpritePosition);

    if (destinationCellType == END)
        return REACH_END;

    return NOTHING;
}

void AddMovement(struct Bot* bot, enum MovementType type, enum Direction direction)
{
    if (!bot) return;

    int currentLength = 0;
    while (bot->MoveQueue[currentLength].type != INVALID &&
        currentLength < MAX_MOVES - 1)
    {
        currentLength++;
    }

    // Vérification de capacité
    if (currentLength >= MAX_MOVES - 1)
    {
        printf("Warning: Movement queue is full!\n");
        return;
    }

    bot->MoveQueue[currentLength].type = type;
    bot->MoveQueue[currentLength].direction = direction;
    bot->MoveQueue[currentLength + 1].type = INVALID;
}

void MoveBot_AI(struct GameData* data)
{
    if (!data || !data->bot || !data->grid) return;

    while (data->bot->MoveQueue[data->step].type != INVALID)
    {
        sfSleep(sfMilliseconds(100));

        enum MovementType type = data->bot->MoveQueue[data->step].type;
        enum Direction direction = data->bot->MoveQueue[data->step].direction;

        data->step++;
        data->pathResult = MoveBot(data->bot, data->grid, type, direction);

        // Sortir en cas de mort ou fin
        if (data->pathResult == REACH_END || data->pathResult == DEAD)
            return;
    }

    // Indiquer qu'il n'y a plus de mouvements
    data->pathResult = NO_MOVE_LEFT;
}


// ============================================================================
// ALGORITHME BFS AVEC SYSTÈME DE COÛTS - ÉVITE LES OBSTACLES
// ============================================================================

// Directions et deltas
static const int dx[4] = { 0, 1, 0, -1 };
static const int dy[4] = { -1, 0, 1, 0 };
static const enum Direction dirs[4] = { NORTH, EAST, SOUTH, WEST };

// Fonction utilitaire
static bool IsInside(int x, int y)
{
    return x >= 0 && x < GRID_COLS && y >= 0 && y < GRID_ROWS;
}

// Structure pour la file BFS avec coût
typedef struct
{
    sfVector2i position;
    int cost;  // Coût total depuis le début
} BFS_Node;

// Structure de file
typedef struct
{
    BFS_Node data[GRID_ROWS * GRID_COLS * 4];
    int front;
    int rear;
} BFS_Queue;

// Initialiser la file
static void InitQueue(BFS_Queue* queue)
{
    queue->front = 0;
    queue->rear = -1;
}

// Vérifier si la file est vide
static bool IsQueueEmpty(BFS_Queue* queue)
{
    return queue->rear < queue->front;
}

// Ajouter un élément
static void Enqueue(BFS_Queue* queue, sfVector2i pos, int cost)
{
    if (queue->rear < GRID_ROWS * GRID_COLS * 4 - 1)
    {
        queue->rear++;
        queue->data[queue->rear].position = pos;
        queue->data[queue->rear].cost = cost;
    }
}

// Retirer un élément
static BFS_Node Dequeue(BFS_Queue* queue)
{
    return queue->data[queue->front++];
}

// Vérifier si deux positions sont égales
static bool PositionsEqual(sfVector2i a, sfVector2i b)
{
    return a.x == b.x && a.y == b.y;
}

// Structure pour stocker les informations de visite
typedef struct
{
    bool visited;
    int cost;           // Coût pour atteindre cette case
    int jumpCount;      // Nombre de sauts effectués
    int moveCount;      // Nombre de mouvements normaux
    sfVector2i parent;
    enum MovementType moveType;
    enum Direction direction;
} VisitInfo;


bool SearchPath_AI(struct Bot* bot, Grid* grid)
{
    // Allocation statique pour éviter malloc/free
    static VisitInfo visited[GRID_ROWS][GRID_COLS];
    static BFS_Queue queue;

    // Réinitialisation
    memset(visited, 0, sizeof(visited));

    // Initialiser tous les coûts à l'infini
    for (int i = 0; i < GRID_ROWS; i++)
    {
        for (int j = 0; j < GRID_COLS; j++)
        {
            visited[i][j].cost = 999999;
        }
    }

    InitQueue(&queue);

    // Trouver START et END
    sfVector2i start = bot->position;
    sfVector2i end = { -1, -1 };

    for (int i = 0; i < GRID_ROWS; i++)
    {
        for (int j = 0; j < GRID_COLS; j++)
        {
            if (grid->cell[i][j]->type == END)
            {
                end.x = j;
                end.y = i;
                break;
            }
        }
        if (end.x != -1) break;
    }

    if (end.x == -1)
    {
        printf("Error: No END cell found!\n");
        return false;
    }

    printf("\n=== AI PATHFINDING (OBSTACLE AVOIDANCE) ===\n");
    printf("Strategy: Minimize obstacles (COST_JUMP=%d vs COST_MOVE=%d)\n",
        COST_JUMP, COST_MOVE);

    // Initialiser BFS avec la position de départ
    Enqueue(&queue, start, 0);
    visited[start.y][start.x].visited = true;
    visited[start.y][start.x].cost = 0;
    visited[start.y][start.x].jumpCount = 0;
    visited[start.y][start.x].moveCount = 0;

    int nodesExplored = 0;
    bool endReached = false;

    // EXPLORATION AVEC MISE À JOUR DES COÛTS
    while (!IsQueueEmpty(&queue))
    {
        BFS_Node current = Dequeue(&queue);
        nodesExplored++;

        // Noter si on a atteint END (mais continuer pour trouver le meilleur chemin)
        if (PositionsEqual(current.position, end))
        {
            endReached = true;
        }

        // Explorer les 4 directions
        for (int d = 0; d < 4; d++)
        {
            int nx = current.position.x + dx[d];
            int ny = current.position.y + dy[d];

            if (!IsInside(nx, ny)) continue;

            enum CellType cellType = grid->cell[ny][nx]->type;

            if (cellType == EMPTY)
                continue;

            // Case accessible (WALKABLE, START, END)
            if (cellType == WALKABLE || cellType == START || cellType == END)
            {
                int newCost = current.cost + COST_MOVE;

                // N'ajouter que si c'est un meilleur chemin (coût inférieur)
                if (newCost < visited[ny][nx].cost)
                {
                    Enqueue(&queue, (sfVector2i) { nx, ny }, newCost);
                    visited[ny][nx].visited = true;
                    visited[ny][nx].cost = newCost;
                    visited[ny][nx].jumpCount = visited[current.position.y][current.position.x].jumpCount;
                    visited[ny][nx].moveCount = visited[current.position.y][current.position.x].moveCount + 1;
                    visited[ny][nx].parent = current.position;
                    visited[ny][nx].moveType = MOVE_TO;
                    visited[ny][nx].direction = dirs[d];
                }
            }
            // Obstacle - sauter (COÛT ÉLEVÉ)
            else if (cellType == OBSTACLE)
            {
                int nx2 = current.position.x + dx[d] * 2;
                int ny2 = current.position.y + dy[d] * 2;

                if (!IsInside(nx2, ny2)) continue;

                enum CellType cellType2 = grid->cell[ny2][nx2]->type;

                if (cellType2 == WALKABLE || cellType2 == START || cellType2 == END)
                {
                    int newCost = current.cost + COST_JUMP; // COÛT ÉLEVÉ POUR OBSTACLES

                    // N'ajouter que si c'est un meilleur chemin
                    if (newCost < visited[ny2][nx2].cost)
                    {
                        Enqueue(&queue, (sfVector2i) { nx2, ny2 }, newCost);
                        visited[ny2][nx2].visited = true;
                        visited[ny2][nx2].cost = newCost;
                        visited[ny2][nx2].jumpCount = visited[current.position.y][current.position.x].jumpCount + 1;
                        visited[ny2][nx2].moveCount = visited[current.position.y][current.position.x].moveCount;
                        visited[ny2][nx2].parent = current.position;
                        visited[ny2][nx2].moveType = JUMP;
                        visited[ny2][nx2].direction = dirs[d];
                    }
                }
            }
        }
    }

    printf("Exploration complete!\n");
    printf("Nodes explored: %d\n", nodesExplored);

    // Vérifier si un chemin existe
    if (!endReached || !visited[end.y][end.x].visited)
    {
        printf("No path found to END!\n");
        return false;
    }

    // Afficher les statistiques
    int accessibleCount = 0;
    for (int i = 0; i < GRID_ROWS; i++)
    {
        for (int j = 0; j < GRID_COLS; j++)
        {
            if (visited[i][j].visited)
                accessibleCount++;
        }
    }
    printf("Accessible positions: %d / %d\n", accessibleCount, GRID_ROWS * GRID_COLS);

    int finalCost = visited[end.y][end.x].cost;
    int finalJumps = visited[end.y][end.x].jumpCount;
    int finalMoves = visited[end.y][end.x].moveCount;
    int totalMoves = finalJumps + finalMoves;

    printf("\n=== OPTIMAL PATH FOUND ===\n");
    printf("Total cost: %d\n", finalCost);
    printf("Total moves: %d (%d MOVE + %d JUMP)\n", totalMoves, finalMoves, finalJumps);
    printf("Obstacles crossed: %d\n", finalJumps);
    printf("==========================\n\n");

    // Reconstruire le chemin depuis END jusqu'à START
    sfVector2i path[MAX_MOVES];
    enum MovementType moveTypes[MAX_MOVES];
    enum Direction directions[MAX_MOVES];
    int pathLength = 0;

    sfVector2i current = end;

    while (!PositionsEqual(current, start) && pathLength < MAX_MOVES)
    {
        path[pathLength] = current;
        moveTypes[pathLength] = visited[current.y][current.x].moveType;
        directions[pathLength] = visited[current.y][current.x].direction;
        pathLength++;
        current = visited[current.y][current.x].parent;
    }

    // Ajouter les mouvements à la queue du bot (en ordre inverse)
    for (int i = pathLength - 1; i >= 0; i--)
    {
        AddMovement(bot, moveTypes[i], directions[i]);
    }

    return true;
}