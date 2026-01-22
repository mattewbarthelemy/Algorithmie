#pragma once

#include "basics.h"
#include "map.h"

typedef struct MapSelectionMenu
{
    sfText* currentMapText;
    sfSprite* leftArrowSprite;
    sfSprite* rightArrowSprite;
    sfSprite* modeIconSprite;
    sfText* modeText;
} MapSelectionMenu;

void LoadAllMapSelectionMenuTextures();

MapSelectionMenu* CreateMapSelectionMenu();

void ChangeMode(MapSelectionMenu* mapSelectionMenu, bool aiMode);

void ChangeMap(MapSelectionMenu* mapSelectionMenu, Map* map);

void DestroyMapSelectionMenu(MapSelectionMenu* menu);

void DrawMapSelectionMenu(sfRenderWindow* window, MapSelectionMenu* menu);