#include "Bot.h"
#include "AnimationBot.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define COST_MOVE 1
#define COST_JUMP 3


struct Bot* CreateBot()
{
    struct Bot* bot = (struct Bot*)malloc(sizeof(struct Bot));
    if (!bot) {
        printf("ERROR: Failed to allocate Bot!\n");
        return NULL;
    }
    memset(bot, 0, sizeof(struct Bot));

    bot->position = (sfVector2i){ 0, 0 };
    bot->sprite = sfSprite_create();
    if (!bot->sprite) {
        printf("ERROR: Failed to create sprite!\n");
        free(bot);
        return NULL;
    }

    // Créer l'animation (module séparé)
    bot->animation = CreateBotAnimation(0.5f);
    if (!bot->animation) {
        printf("ERROR: Failed to create animation!\n");
        sfSprite_destroy(bot->sprite);
        free(bot);
        return NULL;
    }

    // Charger les frames
    if (!LoadAnimationFrames(bot->animation)) {
        printf("WARNING: Some animation frames failed to load!\n");
    }

    // Appliquer la frame initiale
    if (bot->animation->frames[0]) {
        sfSprite_setTexture(bot->sprite, bot->animation->frames[0], sfTrue);
    }

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
            if (grid->cell[i][j] && grid->cell[i][j]->type == START)
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

    if (bot->animation) {
        DestroyBotAnimation(bot->animation);
    }

    if (bot->sprite) {
        sfSprite_destroy(bot->sprite);
    }

    free(bot);
}

void DrawBot(sfRenderWindow* window, struct Bot* bot)
{
    if (!window || !bot || !bot->sprite) return;
    sfRenderWindow_drawSprite(window, bot->sprite, NULL);
}

int MoveBot(struct Bot* bot, Grid* grid, enum MovementType type, enum Direction direction)
{
    if (!bot || !grid) return DEAD;

    // Activer l'animation
    StartAnimation(bot->animation);

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

    if (newPosition.x < 0 || newPosition.x >= GRID_COLS ||
        newPosition.y < 0 || newPosition.y >= GRID_ROWS)
    {
        return DEAD;
    }

    if (!grid->cell[newPosition.y][newPosition.x])
        return DEAD;

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
        UpdateAnimation(data->bot->animation, data->bot->sprite);
        sfSleep(sfMilliseconds(50));

        enum MovementType type = data->bot->MoveQueue[data->step].type;
        enum Direction direction = data->bot->MoveQueue[data->step].direction;

        data->step++;
        data->pathResult = MoveBot(data->bot, data->grid, type, direction);

        if (data->pathResult == REACH_END || data->pathResult == DEAD)
            return;
    }

    data->pathResult = NO_MOVE_LEFT;
}


static const int dx[4] = { 0, 1, 0, -1 };
static const int dy[4] = { -1, 0, 1, 0 };
static const enum Direction dirs[4] = { NORTH, EAST, SOUTH, WEST };

static bool IsInside(int x, int y)
{
    return x >= 0 && x < GRID_COLS && y >= 0 && y < GRID_ROWS;
}

typedef struct
{
    sfVector2i position;
    int cost;
} BFS_Node;

typedef struct
{
    BFS_Node data[GRID_ROWS * GRID_COLS * 4];
    int front;
    int rear;
} BFS_Queue;

static void InitQueue(BFS_Queue* queue)
{
    queue->front = 0;
    queue->rear = -1;
}

static bool IsQueueEmpty(BFS_Queue* queue)
{
    return queue->rear < queue->front;
}

static void Enqueue(BFS_Queue* queue, sfVector2i pos, int cost)
{
    if (queue->rear < GRID_ROWS * GRID_COLS * 4 - 1)
    {
        queue->rear++;
        queue->data[queue->rear].position = pos;
        queue->data[queue->rear].cost = cost;
    }
}

static BFS_Node Dequeue(BFS_Queue* queue)
{
    return queue->data[queue->front++];
}

static bool PositionsEqual(sfVector2i a, sfVector2i b)
{
    return a.x == b.x && a.y == b.y;
}

typedef struct
{
    bool visited;
    int cost;
    int jumpCount;
    int moveCount;
    sfVector2i parent;
    enum MovementType moveType;
    enum Direction direction;
} VisitInfo;


bool SearchPath_AI(struct Bot* bot, Grid* grid)
{
    static VisitInfo visited[GRID_ROWS][GRID_COLS];
    static BFS_Queue queue;

    memset(visited, 0, sizeof(visited));

    for (int i = 0; i < GRID_ROWS; i++)
    {
        for (int j = 0; j < GRID_COLS; j++)
        {
            visited[i][j].cost = 999999;
        }
    }

    InitQueue(&queue);

    sfVector2i start = bot->position;
    sfVector2i end = { -1, -1 };

    for (int i = 0; i < GRID_ROWS; i++)
    {
        for (int j = 0; j < GRID_COLS; j++)
        {
            if (grid->cell[i][j] && grid->cell[i][j]->type == END)
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


    Enqueue(&queue, start, 0);
    visited[start.y][start.x].visited = true;
    visited[start.y][start.x].cost = 0;
    visited[start.y][start.x].jumpCount = 0;
    visited[start.y][start.x].moveCount = 0;

    bool endReached = false;

    while (!IsQueueEmpty(&queue))
    {
        BFS_Node current = Dequeue(&queue);

        if (PositionsEqual(current.position, end))
        {
            endReached = true;
        }

        for (int d = 0; d < 4; d++)
        {
            int nx = current.position.x + dx[d];
            int ny = current.position.y + dy[d];

            if (!IsInside(nx, ny)) continue;

            if (!grid->cell[ny][nx])
                continue;

            enum CellType cellType = grid->cell[ny][nx]->type;

            if (cellType == EMPTY)
                continue;

            if (cellType == WALKABLE || cellType == START || cellType == END)
            {
                int newCost = current.cost + COST_MOVE;

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
            else if (cellType == OBSTACLE)
            {
                int nx2 = current.position.x + dx[d] * 2;
                int ny2 = current.position.y + dy[d] * 2;

                if (!IsInside(nx2, ny2)) continue;

                if (!grid->cell[ny2][nx2])
                    continue;

                enum CellType cellType2 = grid->cell[ny2][nx2]->type;

                if (cellType2 == WALKABLE || cellType2 == START || cellType2 == END)
                {
                    int newCost = current.cost + COST_JUMP;

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


    if (!endReached || !visited[end.y][end.x].visited)
    {
        printf("No path found to END!\n");
        return false;
    }

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

    for (int i = pathLength - 1; i >= 0; i--)
    {
        AddMovement(bot, moveTypes[i], directions[i]);
    }

    return true;
}