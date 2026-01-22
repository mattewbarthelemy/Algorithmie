#pragma once

#include <SFML/System.h>
#include <SFML/Graphics.h>
#include <stdbool.h>
#include "basics.h"
#include "grid.h"
#include "AnimationBot.h"

#define MAX_MOVES 1024

struct Map;

struct Move
{
    enum MovementType type;
    enum Direction direction;
};

struct Bot
{
    sfVector2i position;
    sfSprite* sprite;
    struct Move MoveQueue[MAX_MOVES];

    BotAnimation* animation;
};

struct GameData
{
    struct Bot* bot;
    Grid* grid;
    int step;
    int pathResult;
};

struct Bot* CreateBot();
void SpawnBotAtStartCell(struct Bot* bot, Grid* grid);
void DestroyBot(struct Bot* bot);
void DrawBot(sfRenderWindow* window, struct Bot* bot);
int MoveBot(struct Bot* bot, Grid* grid, enum MovementType type, enum Direction direction);
void AddMovement(struct Bot* bot, enum MovementType type, enum Direction direction);
void MoveBot_AI(struct GameData* data);
bool SearchPath_AI(struct Bot* bot, Grid* grid);