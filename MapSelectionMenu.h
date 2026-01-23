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
    sfSprite* TextureIconSprite;
    sfText* TextureSelect;
} MapSelectionMenu;

void LoadAllMapSelectionMenuTextures();

MapSelectionMenu* CreateMapSelectionMenu();

void ChangeMode(MapSelectionMenu* mapSelectionMenu, bool aiMode);

void ChangeTexture(MapSelectionMenu* mapSelectionMenu, bool customtexture);

void ChangeMap(MapSelectionMenu* mapSelectionMenu, Map* map);

void DestroyMapSelectionMenu(MapSelectionMenu* menu);

void DrawMapSelectionMenu(sfRenderWindow* window, MapSelectionMenu* menu);