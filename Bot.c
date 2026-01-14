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

    // ? Tombe dans le vide ? mort immédiate
    if (destinationCellType == EMPTY)
    {
        return DEAD;
    }

    // ? Obstacle non franchissable
    if (destinationCellType == OBSTACLE && type != JUMP)
    {
        printf("can't go there !\n");
        return NOTHING;
    }

    // ? Mouvement valide
    bot->position = newPosition;

    sfVector2f newSpritePosition =
        sfSprite_getPosition(grid->cell[newPosition.y][newPosition.x]->sprite);
    newSpritePosition.x += 5.f;
    newSpritePosition.y += 5.f;
    sfSprite_setPosition(bot->sprite, newSpritePosition);

    // ? Résultat
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
    if (!data || !data->bot || !data->grid) return;

    while (data->bot->MoveQueue[data->step].type != INVALID)
    {
        sfSleep(sfMilliseconds(100));

        enum MovementType type =
            data->bot->MoveQueue[data->step].type;
        enum Direction direction =
            data->bot->MoveQueue[data->step].direction;

        data->step++;
        data->pathResult =
            MoveBot(data->bot, data->grid, type, direction);

        if (data->pathResult == REACH_END)
            return;
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
    AI_Cell ai[GRID_ROWS][GRID_COLS] = { 0 };
    sfVector2i stack[GRID_ROWS * GRID_COLS];
    int stackTop = 0;

    sfVector2i current = bot->position;
    ai[current.y][current.x].visited = true;

    while (1)
    {
        if (grid->cell[current.y][current.x]->type == END)
            return true;

        bool moved = false;

        for (int d = 0; d < 4; d++)
        {
            if (ai[current.y][current.x].tried[d]) continue;
            ai[current.y][current.x].tried[d] = true;

            int nx = current.x + dx[d];
            int ny = current.y + dy[d];

            if (!IsInside(nx, ny)) continue;
            if (ai[ny][nx].visited) continue;

            enum CellType type = grid->cell[ny][nx]->type;
            if (type == EMPTY) continue;

            AddMovement(bot,
                (type == OBSTACLE) ? JUMP : MOVE_TO,
                dirs[d]);

            stack[stackTop++] = current;
            current = (sfVector2i){ nx, ny };
            ai[ny][nx].visited = true;

            moved = true;
            break;
        }

        if (!moved)
        {
            if (stackTop == 0) return false;

            sfVector2i prev = stack[--stackTop];
            enum Direction back;

            if (prev.x > current.x) back = EAST;
            else if (prev.x < current.x) back = WEST;
            else if (prev.y > current.y) back = SOUTH;
            else back = NORTH;

            AddMovement(bot, MOVE_TO, back);
            current = prev;
        }
    }
}