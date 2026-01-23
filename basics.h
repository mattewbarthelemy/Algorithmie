#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <SFML/Graphics/Types.h>

#define WINDOW_TITLE "LiteBot"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800

#define GRID_ROWS 20
#define GRID_COLS 20
#define CELL_SIZE 40
#define GRAPHIC_OFFSET 20

static sfFont* GAME_FONT;

enum ExitCode {
    FAILURE = -1,
    SUCCESS = 1,
    NULL_TEXT = 60,
    NULL_WINDOW = 61,
    NULL_SPRITE = 62,
    NULL_TEXTURE = 63,
    NULL_FONT = 64,
};

enum Scene {
    MAP_SELECTION,
    GAME,
};

enum CellType {
    EMPTY,
    WALKABLE,
    START,
    END,
    OBSTACLE,
};

enum Direction
{
    NONE = 0,
    EAST = 1,
    WEST = 2,
    EAST_WEST = 3,
    NORTH = 10,
    NORTH_EAST = 11,
    NORTH_WEST = 12,
    NORTH_EAST_WEST = 13,
    SOUTH = 20,
    SOUTH_EAST = 21,
    SOUTH_WEST = 22,
    SOUTH_EAST_WEST = 23,
    NORTH_SOUTH = 30,
    NORTH_SOUTH_EAST = 31,
    NORTH_SOUTH_WEST = 32,
    ALL = 33,
};

enum MovementType
{
    INVALID,
    MOVE_TO,
    JUMP,
};

enum MoveResult
{
    NO_MOVE_LEFT = -2,
    DEAD = -1,
    NOTHING = 0,
    REACH_END = 1,
};