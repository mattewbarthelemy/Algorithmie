#pragma once

#include <SFML/Graphics.h>
#include <SFML/System.h>
#include <stdbool.h>

#define ANIMATION_FRAMES 5

typedef enum {
    ANIM_LEFT,
    ANIM_RIGHT
} AnimDirection;

typedef struct {

    sfTexture* framesLeft[ANIMATION_FRAMES];
    sfTexture* framesRight[ANIMATION_FRAMES];

    int currentFrame;
    AnimDirection currentDirection;
    sfClock* clock;
    float speed;
    bool isPlaying;
} BotAnimation;

BotAnimation* CreateBotAnimation(float speed);

bool LoadAnimationFrames(BotAnimation* anim);

void UpdateAnimation(BotAnimation* anim, sfSprite* sprite);

void StartAnimation(BotAnimation* anim);

void StopAnimation(BotAnimation* anim, sfSprite* sprite);

void SetAnimationDirection(BotAnimation* anim, AnimDirection direction, sfSprite* sprite);

void DestroyBotAnimation(BotAnimation* anim);

int GetCurrentFrame(BotAnimation* anim);

void SetAnimationSpeed(BotAnimation* anim, float speed);