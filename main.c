#include <SFML/Audio.h>
#include <SFML/Graphics.h>
#include <time.h>

#include "basics.h"
#include "Bot.h"
#include "grid.h"
#include "map.h"
#include "MapSelectionMenu.h"

volatile bool stopAIThread = false;

int main(void)
{
    srand(time(NULL));

    sfVideoMode mode = { WINDOW_WIDTH, WINDOW_HEIGHT, 32 };
    sfRenderWindow* window;
    sfEvent event;
    window = sfRenderWindow_create(mode, WINDOW_TITLE, sfClose, NULL);
    if (!window)
    {
        return NULL_WINDOW;
    }

    sfFont* timerFont = sfFont_createFromFile("./Assets/Andelion Script.ttf");
    if (!timerFont) {
        printf("WARNING: Failed to load timer font\n");
    }
    sfText* timerText = sfText_create();
    if (timerFont) sfText_setFont(timerText, timerFont);
    sfText_setCharacterSize(timerText, 30);
    sfText_setFillColor(timerText, sfWhite);
    sfText_setString(timerText, "00:00.00");

    MapSelectionMenu* mapSelectionMenu = CreateMapSelectionMenu();

    int scene = MAP_SELECTION;
    int currentMap = 0;
    enum MovementType movementType = MOVE_TO;
    bool AIMode = false;
    bool AIMoveInProgess = false;
    bool CustomTexture = false;

    sfThread* aiThread = NULL;
    struct GameData* aiData = (struct GameData*)malloc(sizeof(struct GameData));
    if (!aiData) {
        printf("ERROR: Failed to allocate GameData!\n");
        return FAILURE;
    }
    aiData->bot = CreateBot();
    if (!aiData->bot) {
        printf("ERROR: Failed to create Bot!\n");
        free(aiData);
        return FAILURE;
    }
    aiData->grid = NULL;
    aiData->step = 0;
    aiData->pathResult = NOTHING;
    aiData->timer = sfClock_create();
    aiData->elapsedTime = 0.0f;
    aiData->timerRunning = false;
    aiThread = sfThread_create(MoveBot_AI, aiData);
    bool threadLaunched = false;

    Map maps[20];
    for (int i = 0; i < 20; i++)
    {
        maps[i] = CreateMap("Empty", MAP_NULL);
    }

    maps[0] = CreateMap("The Line", MAP_01);
    maps[1] = CreateMap("OBSTACLES !", MAP_02);
    maps[2] = CreateMap("The S", MAP_03);
    maps[3] = CreateMap("Make a Choice", MAP_04);
    maps[4] = CreateMap("The Maze", MAP_05);
    maps[5] = CreateMap("Easy", MAP_06);
    maps[6] = CreateMap("The Snake", MAP_07);
    maps[7] = CreateMap("Lost", MAP_08);
    maps[8] = CreateMap("The Chaos", MAP_09);
    maps[9] = CreateMap("The Maze 2", MAP_10);
    maps[10] = CreateMap("The Dungeon", MAP_11);
    maps[11] = CreateMap("The Slime", MAP_12);
    maps[12] = CreateMap("Interstellar", MAP_13);
    maps[13] = CreateMap("Crazy World", MAP_14);

    ChangeMap(mapSelectionMenu, &maps[currentMap]);

    while (sfRenderWindow_isOpen(window))
    {
        while (sfRenderWindow_pollEvent(window, &event))
        {
            if (event.type == sfEvtClosed)
            {
                sfRenderWindow_close(window);
            }

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
                        if (threadLaunched)
                        {
                            sfThread_terminate(aiThread);
                            threadLaunched = false;
                            sfSleep(sfMilliseconds(100));
                        }

                        if (aiData->grid != NULL)
                        {
                            DestroyGrid(aiData->grid);
                            aiData->grid = NULL;
                        }

                        aiData->grid = CreateGrid(maps[currentMap].data, CustomTexture);
                        if (!aiData->grid) {
                            printf("ERROR: Failed to create grid!\n");
                            scene = MAP_SELECTION;
                            break;
                        }
                        SpawnBotAtStartCell(aiData->bot, aiData->grid);

                        aiData->bot->MoveQueue[0].type = INVALID;
                        aiData->step = 0;
                        aiData->pathResult = NOTHING;
                        stopAIThread = false;

                        sfClock_restart(aiData->timer);
                        aiData->elapsedTime = 0.0f;
                        aiData->timerRunning = true;

                        if (AIMode)
                        {
                            bool found = SearchPath_AI(aiData->bot, aiData->grid);
                            if (!found)
                            {
                                printf("No path found by AI\n");
                            }
                            if (found)
                            {
                                AIMoveInProgess = true;
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
                    case sfKeyTab:
                        CustomTexture = !CustomTexture;
                        ChangeTexture(mapSelectionMenu, CustomTexture);
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
                        case sfKeyBackspace:
                            AIMoveInProgess = false;

                            // Arrêter le timer
                            if (aiData->timerRunning) {
                                aiData->elapsedTime = sfTime_asSeconds(sfClock_getElapsedTime(aiData->timer));
                                aiData->timerRunning = false;
                            }

                            if (threadLaunched)
                            {
                                sfThread_terminate(aiThread);
                                threadLaunched = false;
                                sfSleep(sfMilliseconds(100));
                            }
                            if (aiData->grid != NULL)
                            {
                                DestroyGrid(aiData->grid);
                                aiData->grid = NULL;
                            }
                            aiData->step = 0;
                            aiData->pathResult = NOTHING;
                            printf("Returning to map selection...\n");
                            scene = MAP_SELECTION;
                            break;
                        default:
                            break;
                        }
                    }
                }
                else
                {
                    if (event.type == sfEvtKeyPressed)
                    {
                        enum MoveResult result = NOTHING;
                        switch (event.key.code)
                        {
                        case sfKeyBackspace:
                            // Arrêter le timer
                            if (aiData->timerRunning) {
                                aiData->elapsedTime = sfTime_asSeconds(sfClock_getElapsedTime(aiData->timer));
                                aiData->timerRunning = false;
                            }

                            if (aiData->grid != NULL)
                            {
                                DestroyGrid(aiData->grid);
                                aiData->grid = NULL;
                            }
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
                            printf("JUMP Ready!\n");
                            break;
                        default:
                            break;
                        }
                        switch (result)
                        {
                        case DEAD:
                            printf("Unfortunately you fell off the parkour..\n");

                            // Arrêter le timer
                            if (aiData->timerRunning) {
                                aiData->elapsedTime = sfTime_asSeconds(sfClock_getElapsedTime(aiData->timer));
                                aiData->timerRunning = false;
                            }

                            if (aiData->grid != NULL)
                            {
                                DestroyGrid(aiData->grid);
                                aiData->grid = NULL;
                            }
                            scene = MAP_SELECTION;
                            break;
                        case REACH_END:
                            printf("Congratulations! You reached the end!\n");

                            // Arrêter le timer
                            if (aiData->timerRunning) {
                                aiData->elapsedTime = sfTime_asSeconds(sfClock_getElapsedTime(aiData->timer));
                                aiData->timerRunning = false;
                            }

                            if (aiData->grid != NULL)
                            {
                                DestroyGrid(aiData->grid);
                                aiData->grid = NULL;
                            }
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

        if (scene == GAME && AIMode && AIMoveInProgess)
        {
            if (!threadLaunched)
            {
                sfThread_launch(aiThread);
                threadLaunched = true;
            }

            switch (aiData->pathResult)
            {
            case REACH_END:
                printf("======================\n");
                printf("SUCCESS! Bot reached the end!\n");
                printf("======================\n\n");

                // Arrêter le timer
                if (aiData->timerRunning) {
                    aiData->elapsedTime = sfTime_asSeconds(sfClock_getElapsedTime(aiData->timer));
                    aiData->timerRunning = false;
                }

                AIMoveInProgess = false;
                if (threadLaunched)
                {
                    stopAIThread = true;
                    sfThread_wait(aiThread);
                    threadLaunched = false;
                }
                if (aiData->grid != NULL)
                {
                    DestroyGrid(aiData->grid);
                    aiData->grid = NULL;
                }
                aiData->step = 0;
                aiData->pathResult = NOTHING;
                aiData->bot->MoveQueue[0].type = INVALID;
                scene = MAP_SELECTION;
                break;
            case NO_MOVE_LEFT:
                AIMoveInProgess = false;
                if (threadLaunched)
                {
                    stopAIThread = true;
                    sfThread_wait(aiThread);
                    threadLaunched = false;
                }
                if (aiData->grid != NULL)
                {
                    DestroyGrid(aiData->grid);
                    aiData->grid = NULL;
                }
                aiData->step = 0;
                aiData->pathResult = NOTHING;
                aiData->bot->MoveQueue[0].type = INVALID;
                scene = MAP_SELECTION;
                break;
            case DEAD:
                printf("Bot is dead - Fell off the map!\n");

                // Arrêter le timer
                if (aiData->timerRunning) {
                    aiData->elapsedTime = sfTime_asSeconds(sfClock_getElapsedTime(aiData->timer));
                    aiData->timerRunning = false;
                }

                AIMoveInProgess = false;
                if (threadLaunched)
                {
                    stopAIThread = true;
                    sfThread_wait(aiThread);
                    threadLaunched = false;
                }
                if (aiData->grid != NULL)
                {
                    DestroyGrid(aiData->grid);
                    aiData->grid = NULL;
                }
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


        sfRenderWindow_clear(window, sfColor_fromRGB(33, 79, 158));

        switch (scene) {
        case MAP_SELECTION:
            DrawMapSelectionMenu(window, mapSelectionMenu);
            break;
        case GAME:
            if (aiData->grid != NULL)
            {
                UpdateAnimation(aiData->bot->animation, aiData->bot->sprite);

                if (aiData->bot->animation && aiData->bot->animation->isPlaying && aiData->bot->animation->clock) {
                    sfTime elapsed = sfClock_getElapsedTime(aiData->bot->animation->clock);
                    if (sfTime_asSeconds(elapsed) > 0.6f && !AIMoveInProgess) {
                        StopAnimation(aiData->bot->animation, aiData->bot->sprite);
                    }
                }

                DrawGrid(window, aiData->grid);
                DrawBot(window, aiData->bot);

            }
            break;
        }

        // Afficher le timer (dans toutes les scènes)
        float displayTime;
        if (aiData->timerRunning) {
            displayTime = sfTime_asSeconds(sfClock_getElapsedTime(aiData->timer));
        }
        else {
            displayTime = aiData->elapsedTime;
        }

        int minutes = (int)(displayTime / 60);
        int seconds = (int)displayTime % 60;
        int centiseconds = (int)((displayTime - (int)displayTime) * 100);

        char timerString[32];
        sprintf_s(timerString, sizeof(timerString), "%02d:%02d.%02d", minutes, seconds, centiseconds);
        sfText_setString(timerText, timerString);

        sfFloatRect textBounds = sfText_getLocalBounds(timerText);
        float textX = (WINDOW_WIDTH - textBounds.width) / 2.0f;
        sfText_setPosition(timerText, (sfVector2f) { textX, 10.0f });

        sfRenderWindow_drawText(window, timerText, NULL);

        sfRenderWindow_display(window);
    }

    if (threadLaunched)
    {
        sfThread_terminate(aiThread);
    }
    sfThread_destroy(aiThread);

    if (aiData->grid != NULL)
    {
        DestroyGrid(aiData->grid);
    }

    if (aiData->timer) {
        sfClock_destroy(aiData->timer);
    }

    DestroyBot(aiData->bot);
    free(aiData);

    if (timerText) {
        sfText_destroy(timerText);
    }
    if (timerFont) {
        sfFont_destroy(timerFont);
    }

    DestroyMapSelectionMenu(mapSelectionMenu);
    sfRenderWindow_destroy(window);

    return SUCCESS;
}