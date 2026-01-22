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
        anim->framesLeft[i] = NULL;
        anim->framesRight[i] = NULL;
    }

    anim->currentFrame = 0;
    anim->currentDirection = ANIM_RIGHT;  // Direction par défaut
    anim->speed = speed;
    anim->isPlaying = true;  // ? ACTIF dès le début !

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

    printf("=== Chargement des animations ===\n");

    // Charger les frames GAUCHE (Move_Left)
    anim->framesLeft[0] = sfTexture_createFromFile("./Assets/Characters/Move_Left/Bot_L0.png", NULL);
    anim->framesLeft[1] = sfTexture_createFromFile("./Assets/Characters/Move_Left/Bot_L1.png", NULL);
    anim->framesLeft[2] = sfTexture_createFromFile("./Assets/Characters/Move_Left/Bot_L2.png", NULL);

    // Charger les frames DROITE (Move_Right)
    anim->framesRight[0] = sfTexture_createFromFile("./Assets/Characters/Move_Right/Bot-R0.png", NULL);
    anim->framesRight[1] = sfTexture_createFromFile("./Assets/Characters/Move_Right/Bot_R01.png", NULL);
    anim->framesRight[2] = sfTexture_createFromFile("./Assets/Characters/Move_Right/Bot_R02.png", NULL);

    // Vérifier le chargement
    bool leftOk = true, rightOk = true;

    for (int i = 0; i < ANIMATION_FRAMES; i++) {
        if (anim->framesLeft[i]) {
            printf("Left Frame %d OK\n", i);
        }
        else {
            printf("WARNING: Left Frame %d failed\n", i);
            leftOk = false;
        }

        if (anim->framesRight[i]) {
            printf("Right Frame %d OK\n", i);
        }
        else {
            printf("WARNING: Right Frame %d failed\n", i);
            rightOk = false;
        }
    }

    if (leftOk && rightOk) {
        printf("=== Toutes les animations chargees ! ===\n");
    }

    return leftOk && rightOk;
}

void UpdateAnimation(BotAnimation* anim, sfSprite* sprite)
{
    if (!anim || !sprite) return;

    // ? L'animation tourne TOUJOURS
    if (anim->isPlaying) {
        sfTime elapsed = sfClock_getElapsedTime(anim->clock);
        float seconds = sfTime_asSeconds(elapsed);

        if (seconds >= anim->speed) {
            // Passer à la frame suivante
            anim->currentFrame = (anim->currentFrame + 1) % ANIMATION_FRAMES;

            // Sélectionner les frames selon la direction
            sfTexture* currentTexture = NULL;
            if (anim->currentDirection == ANIM_LEFT) {
                currentTexture = anim->framesLeft[anim->currentFrame];
            }
            else {
                currentTexture = anim->framesRight[anim->currentFrame];
            }

            if (currentTexture) {
                sfSprite_setTexture(sprite, currentTexture, sfTrue);
            }

            sfClock_restart(anim->clock);
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

    // Revenir à la première frame de la direction actuelle
    if (sprite) {
        sfTexture* firstFrame = NULL;
        if (anim->currentDirection == ANIM_LEFT) {
            firstFrame = anim->framesLeft[0];
        }
        else {
            firstFrame = anim->framesRight[0];
        }

        if (firstFrame) {
            sfSprite_setTexture(sprite, firstFrame, sfTrue);
        }
    }
}

void SetAnimationDirection(BotAnimation* anim, AnimDirection direction, sfSprite* sprite)
{
    if (!anim) return;

    // Si la direction change, redemarrer l'animation depuis la frame 0
    if (anim->currentDirection != direction) {
        anim->currentDirection = direction;
        anim->currentFrame = 0;
        if (anim->clock) {
            sfClock_restart(anim->clock);
        }

        // Appliquer immediatement la texture de la nouvelle direction
        if (sprite) {
            sfTexture* newTexture = NULL;
            if (direction == ANIM_LEFT) {
                newTexture = anim->framesLeft[0];
            }
            else {
                newTexture = anim->framesRight[0];
            }

            if (newTexture) {
                sfSprite_setTexture(sprite, newTexture, sfTrue);
            }
        }
    }
}

void DestroyBotAnimation(BotAnimation* anim)
{
    if (!anim) return;

    // Libérer toutes les textures
    for (int i = 0; i < ANIMATION_FRAMES; i++) {
        if (anim->framesLeft[i]) {
            sfTexture_destroy(anim->framesLeft[i]);
            anim->framesLeft[i] = NULL;
        }
        if (anim->framesRight[i]) {
            sfTexture_destroy(anim->framesRight[i]);
            anim->framesRight[i] = NULL;
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