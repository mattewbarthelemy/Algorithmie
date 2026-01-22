#pragma once

#include <SFML/Graphics.h>
#include <SFML/System.h>
#include <stdbool.h>

#define ANIMATION_FRAMES 3  // 3 images par direction

// Direction de l'animation
typedef enum {
    ANIM_LEFT,
    ANIM_RIGHT
} AnimDirection;

// Structure d'animation pour le bot avec directions
typedef struct {
    // Frames pour chaque direction
    sfTexture* framesLeft[ANIMATION_FRAMES];   // Gauche
    sfTexture* framesRight[ANIMATION_FRAMES];  // Droite

    int currentFrame;
    AnimDirection currentDirection;
    sfClock* clock;
    float speed;
    bool isPlaying;
} BotAnimation;

// Créer et initialiser l'animation
BotAnimation* CreateBotAnimation(float speed);

// Charger toutes les frames (2 directions)
bool LoadAnimationFrames(BotAnimation* anim);

// Mettre à jour l'animation
void UpdateAnimation(BotAnimation* anim, sfSprite* sprite);

// Démarrer l'animation
void StartAnimation(BotAnimation* anim);

// Arrêter l'animation
void StopAnimation(BotAnimation* anim, sfSprite* sprite);

// Changer la direction de l'animation
void SetAnimationDirection(BotAnimation* anim, AnimDirection direction, sfSprite* sprite);

// Détruire l'animation
void DestroyBotAnimation(BotAnimation* anim);

// Obtenir la frame actuelle
int GetCurrentFrame(BotAnimation* anim);

// Changer la vitesse
void SetAnimationSpeed(BotAnimation* anim, float speed);