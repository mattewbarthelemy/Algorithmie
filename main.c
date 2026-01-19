#include <SFML/Audio.h>
#include <SFML/Graphics.h>

#include "basics.h"
#include "Bot.h"
#include "grid.h"
#include "map.h"
#include "MapSelectionMenu.h"


int main(void)
{
    sfVideoMode mode = { WINDOW_WIDTH, WINDOW_HEIGHT, 32 };
    sfRenderWindow* window;
    sfEvent event;

    /* Create the main window */
    window = sfRenderWindow_create(mode, WINDOW_TITLE, sfClose, NULL);
    if (!window)
    {
        return NULL_WINDOW;
    }

    MapSelectionMenu* mapSelectionMenu = CreateMapSelectionMenu();

    int scene = MAP_SELECTION;
    int currentMap = 0;
    enum MovementType movementType = MOVE_TO;
    bool AIMode = false;
    bool AIMoveInProgess = false;

    sfThread* aiThread = NULL;
    struct GameData* aiData = (struct GameData*)malloc(sizeof(struct GameData));
    aiData->bot = CreateBot();
    aiData->grid = NULL;
    aiData->step = 0;
    aiData->pathResult = NOTHING;
    aiThread = sfThread_create(MoveBot_AI, aiData);
    bool threadLaunched = false;

    Map maps[20];
    for (int i = 0; i < 20; i++)
    {
        maps[i] = CreateMap("Empty", MAP_NULL);
    }

    // Define actual maps
    maps[0] = CreateMap("The Line", MAP_01);
    maps[1] = CreateMap("OBSTACLES !", MAP_02);
    maps[2] = CreateMap("The S", MAP_03);
    maps[3] = CreateMap("Make a Choice", MAP_04);
    maps[4] = CreateMap("The Maze", MAP_05);
    maps[5] = CreateMap("06", MAP_06);
    maps[6] = CreateMap("07", MAP_07);

    ChangeMap(mapSelectionMenu, &maps[currentMap]);

    /* Start the game loop */
    while (sfRenderWindow_isOpen(window))
    {
        /* Process events */
        while (sfRenderWindow_pollEvent(window, &event))
        {
            /* Close window : exit */
            if (event.type == sfEvtClosed)
            {
                sfRenderWindow_close(window);
            }

            // Handle input based on current scene
            switch (scene) {
            case MAP_SELECTION:
                if (event.type == sfEvtKeyPressed)
                {
                    switch (event.key.code)
                    {
                    case sfKeyRight:
                        currentMap++;
                        if (maps[currentMap].name == "Empty")
                            currentMap--;
                        ChangeMap(mapSelectionMenu, &maps[currentMap]);
                        break;
                    case sfKeyLeft:
                        currentMap--;
                        if (currentMap < 0)
                            currentMap = 0;
                        ChangeMap(mapSelectionMenu, &maps[currentMap]);
                        break;
                    case sfKeyEnter:
                        // CORRECTION: Nettoyer l'ancienne grille avant d'en créer une nouvelle
                        if (aiData->grid != NULL)
                        {
                            DestroyGrid(aiData->grid);
                        }

                        // Créer la nouvelle grille
                        aiData->grid = CreateGrid(maps[currentMap].data);
                        SpawnBotAtStartCell(aiData->bot, aiData->grid);

                        // CORRECTION: Réinitialiser complètement l'état du bot
                        aiData->bot->MoveQueue[0].type = INVALID;
                        aiData->step = 0;
                        aiData->pathResult = NOTHING;

                        if (AIMode)
                        {
                            bool found = SearchPath_AI(aiData->bot, aiData->grid);
                            if (!found)
                            {
                                printf("No path found by AI\n");
                            }
                        }
                        scene = GAME;
                        break;
                    case sfKeyA:
                        AIMode = !AIMode;
                        ChangeMode(mapSelectionMenu, AIMode);
                        break;
                    default:
                        break;
                    }
                }
                break;
            case GAME:
                if (AIMode)
                {
                    if (event.type == sfEvtKeyPressed)
                    {
                        switch (event.key.code)
                        {
                        case sfKeyEnter:
                            AIMoveInProgess = !AIMoveInProgess;
                            if (AIMoveInProgess)
                            {
                                printf("AI movement started...\n");
                            }
                            else
                            {
                                printf("AI movement paused.\n");
                            }
                            break;
                        case sfKeyBackspace:
                            // CORRECTION: Nettoyer proprement le thread au retour au menu
                            if (threadLaunched)
                            {
                                sfThread_terminate(aiThread);
                                threadLaunched = false;
                            }
                            AIMoveInProgess = false;
                            aiData->step = 0;
                            aiData->pathResult = NOTHING;
                            printf("Returning to map selection...\n");
                            scene = MAP_SELECTION;
                            break;
                        default:
                            break;
                        }
                    }
                    if (AIMoveInProgess && scene == GAME)
                    {
                        // Lancer le thread si pas encore fait
                        if (!threadLaunched)
                        {
                            sfThread_launch(aiThread);
                            threadLaunched = true;
                        }

                        // Vérifier le résultat
                        switch (aiData->pathResult)
                        {
                        case NO_MOVE_LEFT:
                            printf("No movement left\n");
                            // CORRECTION: Réinitialiser TOUTES les variables critiques
                            sfThread_terminate(aiThread);
                            threadLaunched = false;
                            AIMoveInProgess = false;
                            aiData->step = 0;
                            aiData->pathResult = NOTHING;
                            aiData->bot->MoveQueue[0].type = INVALID;
                            scene = MAP_SELECTION;
                            break;
                        case DEAD:
                            printf("Bot is dead - Fell off the map!\n");
                            // CORRECTION: Réinitialiser TOUTES les variables critiques
                            sfThread_terminate(aiThread);
                            threadLaunched = false;
                            AIMoveInProgess = false;
                            aiData->step = 0;
                            aiData->pathResult = NOTHING;
                            aiData->bot->MoveQueue[0].type = INVALID;
                            scene = MAP_SELECTION;
                            break;
                        case REACH_END:
                            printf("======================\n");
                            printf("SUCCESS! Bot reached the end!\n");
                            printf("======================\n\n");
                            // CORRECTION: Réinitialiser TOUTES les variables critiques
                            sfThread_terminate(aiThread);
                            threadLaunched = false;
                            AIMoveInProgess = false;
                            aiData->step = 0;
                            aiData->pathResult = NOTHING;
                            aiData->bot->MoveQueue[0].type = INVALID;
                            scene = MAP_SELECTION;
                            break;
                        case NOTHING:
                        default:
                            break;
                        }
                    }
                }
                else // Mode manuel
                {
                    if (event.type == sfEvtKeyPressed)
                    {
                        enum MoveResult result = NOTHING;
                        switch (event.key.code)
                        {
                        case sfKeyBackspace:
                            scene = MAP_SELECTION;
                            break;
                        case sfKeyRight:
                            result = MoveBot(aiData->bot, aiData->grid, movementType, EAST);
                            movementType = MOVE_TO;
                            break;
                        case sfKeyLeft:
                            result = MoveBot(aiData->bot, aiData->grid, movementType, WEST);
                            movementType = MOVE_TO;
                            break;
                        case sfKeyUp:
                            result = MoveBot(aiData->bot, aiData->grid, movementType, NORTH);
                            movementType = MOVE_TO;
                            break;
                        case sfKeyDown:
                            result = MoveBot(aiData->bot, aiData->grid, movementType, SOUTH);
                            movementType = MOVE_TO;
                            break;
                        case sfKeySpace:
                            movementType = JUMP;
                            printf("Next move will be a JUMP!\n");
                            break;
                        default:
                            break;
                        }
                        switch (result)
                        {
                        case DEAD:
                            printf("Unfortunately you fell off the parkour..\n");
                            scene = MAP_SELECTION;
                            break;
                        case REACH_END:
                            printf("Congratulations! You reached the end!\n");
                            scene = MAP_SELECTION;
                            break;
                        case NOTHING:
                        case NO_MOVE_LEFT:
                            break;
                        }
                    }
                }
                break;
            default:
                break;
            }
        }

        /* Clear the screen */
        sfRenderWindow_clear(window, sfColor_fromRGB(33, 79, 158));

        // Draw everything
        switch (scene) {
        case MAP_SELECTION:
            DrawMapSelectionMenu(window, mapSelectionMenu);
            break;
        case GAME:
            DrawGrid(window, aiData->grid);
            DrawBot(window, aiData->bot);
            break;
        }

        /* Update the window */
        sfRenderWindow_display(window);
    }

    /* Cleanup resources */
    if (threadLaunched)
    {
        sfThread_terminate(aiThread);
    }
    sfThread_destroy(aiThread);

    if (aiData->grid != NULL)
    {
        DestroyGrid(aiData->grid);
    }
    DestroyBot(aiData->bot);
    free(aiData);
    DestroyMapSelectionMenu(mapSelectionMenu);
    sfRenderWindow_destroy(window);

    return SUCCESS;
}