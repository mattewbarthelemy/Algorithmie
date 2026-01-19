#include "Bot.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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

    enum CellType destinationCellType =
        grid->cell[newPosition.y][newPosition.x]->type;

    
    if (destinationCellType == EMPTY)
    {
        return DEAD;
    }

  if (destinationCellType == OBSTACLE && type != JUMP)   return NOTHING;

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

    bot->MoveQueue[currentLength].type = type;
    bot->MoveQueue[currentLength].direction = direction;
    bot->MoveQueue[currentLength + 1].type = INVALID;
}

void MoveBot_AI(struct GameData* data)
{
    if (!data || !data->bot) return;

    while (1)
    {
        while (!data->grid)
            sfSleep(sfMilliseconds(200));

        data->step = 0;
        data->pathResult = NOTHING;

        for (int i = 0; i < MAX_MOVES; i++)
            data->bot->MoveQueue[i].type = INVALID;

        if (!SearchPath_AI(data->bot, data->grid))
        {
            data->pathResult = NO_MOVE_LEFT;
        }
        else
        {
            while (data->bot->MoveQueue[data->step].type != INVALID)
            {
                sfSleep(sfMilliseconds(180));

                enum MovementType type =
                    data->bot->MoveQueue[data->step].type;
                enum Direction direction =
                    data->bot->MoveQueue[data->step].direction;

                data->step++;

                data->pathResult =
                    MoveBot(data->bot, data->grid, type, direction);

                if (data->pathResult == DEAD ||
                    data->pathResult == REACH_END)
                    break;
            }
        }

        while (data->pathResult != NOTHING)
            sfSleep(sfMilliseconds(200));
    }
}


typedef struct
{
    bool visited;
    bool tried[4];
} AI_Cell;

static const int dx[4] = { 0, 1, 0, -1 };
static const int dy[4] = { -1, 0, 1, 0 };
static const enum Direction dirs[4] =
{
    NORTH, EAST, SOUTH, WEST
};

static bool IsInside(int x, int y)
{
    return x >= 0 && x < GRID_COLS &&
        y >= 0 && y < GRID_ROWS;
}

bool SearchPath_AI(struct Bot* bot, Grid* grid)
{
    typedef struct
    {
        sfVector2i pos;
        int prevIndex;
        enum Direction moveFromPrev;
    } BFS_Node;

    BFS_Node queue[GRID_ROWS * GRID_COLS];
    bool visited[GRID_ROWS][GRID_COLS] = { false };
    int head = 0, tail = 0;

    queue[tail++] = (BFS_Node){ bot->position, -1, NORTH };
    visited[bot->position.y][bot->position.x] = true;

    int endIndex = -1;

    while (head < tail)
    {
        BFS_Node cur = queue[head++];

        if (grid->cell[cur.pos.y][cur.pos.x]->type == END)
        {
            endIndex = head - 1;
            break;
        }

        for (int d = 0; d < 4; d++)
        {
            int nx = cur.pos.x + dx[d];
            int ny = cur.pos.y + dy[d];

            if (!IsInside(nx, ny)) continue;
            if (visited[ny][nx]) continue;

            enum CellType type = grid->cell[ny][nx]->type;
            if (type == EMPTY) continue;

            visited[ny][nx] = true;
            queue[tail++] = (BFS_Node){ {nx, ny}, head - 1, dirs[d] };
        }
    }

    if (endIndex == -1) return false;

    int pathLength = 0;
    BFS_Node path[GRID_ROWS * GRID_COLS];

    int idx = endIndex;
    while (idx != -1)
    {
        path[pathLength++] = queue[idx];
        idx = queue[idx].prevIndex;
    }

    for (int i = pathLength - 1; i > 0; i--)
    {
        sfVector2i from = path[i].pos;
        sfVector2i to = path[i - 1].pos;

        enum MovementType type = MOVE_TO;
        if (grid->cell[to.y][to.x]->type == OBSTACLE)
            type = JUMP;

        enum Direction dir;
        if (to.x > from.x) dir = EAST;
        else if (to.x < from.x) dir = WEST;
        else if (to.y > from.y) dir = SOUTH;
        else dir = NORTH;

        AddMovement(bot, type, dir);
    }

    return true;
}
void ResetAIForNewLevel(struct GameData* data)
{
    if (!data || !data->bot || !data->grid) return;

    data->step = 0;
    data->pathResult = NOTHING;

    for (int i = 0; i < MAX_MOVES; i++)
        data->bot->MoveQueue[i].type = INVALID;

}