#include "AnimationBot.h"
#include <stdlib.h>
#include <stdio.h>

BotAnimation* CreateBotAnimation(float speed)
{
    BotAnimation* anim = (BotAnimation*)malloc(sizeof(BotAnimation));
    if (!anim) {
        printf("ERROR: Failed to allocate BotAnimation!\n");
        return NULL;
    }

    // Initialiser toutes les frames à NULL
    for (int i = 0; i < ANIMATION_FRAMES; i++) {
        anim->frames[i] = NULL;
    }

    anim->currentFrame = 0;
    anim->speed = speed;
    anim->isPlaying = false;

    anim->clock = sfClock_create();
    if (!anim->clock) {
        printf("ERROR: Failed to create animation clock!\n");
        free(anim);
        return NULL;
    }

    return anim;
}

bool LoadAnimationFrames(BotAnimation* anim)
{
    if (!anim) return false;

    // Charger les 6 frames directement
    anim->frames[0] = sfTexture_createFromFile("./Assets/Characters/Bot0.png", NULL);
    anim->frames[1] = sfTexture_createFromFile("./Assets/Characters/Bot01.png", NULL);
    anim->frames[2] = sfTexture_createFromFile("./Assets/Characters/Bot02.png", NULL);
    anim->frames[3] = sfTexture_createFromFile("./Assets/Characters/Bot03.png", NULL);
    anim->frames[4] = sfTexture_createFromFile("./Assets/Characters/Bot04.png", NULL);
    anim->frames[5] = sfTexture_createFromFile("./Assets/Characters/Bot05.png", NULL);

    // Vérifier et afficher
    bool allLoaded = true;
    for (int i = 0; i < ANIMATION_FRAMES; i++) {
        if (anim->frames[i]) {
            printf("Animation Frame %d OK\n", i);
        }
        else {
            printf("WARNING: Failed to load frame %d\n", i);
            allLoaded = false;
        }
    }

    return allLoaded;
}

void UpdateAnimation(BotAnimation* anim, sfSprite* sprite)
{
    if (!anim || !sprite) return;

    if (anim->isPlaying) {
        // Animation en cours : changer les frames
        sfTime elapsed = sfClock_getElapsedTime(anim->clock);
        float seconds = sfTime_asSeconds(elapsed);

        if (seconds >= anim->speed) {
            // Passer à la frame suivante
            anim->currentFrame = (anim->currentFrame + 1) % ANIMATION_FRAMES;

            if (anim->frames[anim->currentFrame]) {
                sfSprite_setTexture(sprite, anim->frames[anim->currentFrame], sfTrue);
            }

            sfClock_restart(anim->clock);
        }
    }
    else {
        // Animation arrêtée : revenir à frame 0
        if (anim->currentFrame != 0) {
            anim->currentFrame = 0;
            if (anim->frames[0]) {
                sfSprite_setTexture(sprite, anim->frames[0], sfTrue);
            }
        }
    }
}

void StartAnimation(BotAnimation* anim)
{
    if (!anim) return;

    anim->isPlaying = true;
    if (anim->clock) {
        sfClock_restart(anim->clock);
    }
}

void StopAnimation(BotAnimation* anim, sfSprite* sprite)
{
    if (!anim) return;

    anim->isPlaying = false;
    anim->currentFrame = 0;

    if (sprite && anim->frames[0]) {
        sfSprite_setTexture(sprite, anim->frames[0], sfTrue);
    }
}

void DestroyBotAnimation(BotAnimation* anim)
{
    if (!anim) return;

    // Libérer toutes les textures
    for (int i = 0; i < ANIMATION_FRAMES; i++) {
        if (anim->frames[i]) {
            sfTexture_destroy(anim->frames[i]);
            anim->frames[i] = NULL;
        }
    }

    // Libérer la clock
    if (anim->clock) {
        sfClock_destroy(anim->clock);
    }

    free(anim);
}

int GetCurrentFrame(BotAnimation* anim)
{
    if (!anim) return 0;
    return anim->currentFrame;
}

void SetAnimationSpeed(BotAnimation* anim, float speed)
{
    if (!anim) return;
    anim->speed = speed;
}