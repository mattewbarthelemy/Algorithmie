#pragma once

#include <SFML/Graphics.h>
#include <SFML/System.h>
#include <stdbool.h>

#define ANIMATION_FRAMES 6

// Structure d'animation pour le bot
typedef struct {
    sfTexture* frames[ANIMATION_FRAMES];
    int currentFrame;
    sfClock* clock;
    float speed;
    bool isPlaying;
} BotAnimation;

// Créer et initialiser l'animation
BotAnimation* CreateBotAnimation(float speed);

// Charger les frames d'animation
bool LoadAnimationFrames(BotAnimation* anim);

// Mettre à jour l'animation
void UpdateAnimation(BotAnimation* anim, sfSprite* sprite);

// Démarrer l'animation
void StartAnimation(BotAnimation* anim);

// Arrêter l'animation et revenir à frame 0
void StopAnimation(BotAnimation* anim, sfSprite* sprite);

// Détruire l'animation et libérer la mémoire
void DestroyBotAnimation(BotAnimation* anim);

// Obtenir la frame actuelle
int GetCurrentFrame(BotAnimation* anim);

// Changer la vitesse d'animation
void SetAnimationSpeed(BotAnimation* anim, float speed);