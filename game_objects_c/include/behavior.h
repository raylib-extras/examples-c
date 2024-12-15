#pragma once
#include "game_object.h"

#include "raylib.h"

typedef struct Behavior
{
    void (*UpdateFunction)(GameObject*);
}Behavior;

Behavior* CreateBahavior(void (*updateFunction)(GameObject*));
Behavior* GetBahaviorComponent(GameObject* object);