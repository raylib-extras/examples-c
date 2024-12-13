#pragma once
#include "game_object.h"

#include "raylib.h"

typedef struct Sprite
{
	Texture2D Texture;
}Sprite;

Sprite* CreateSprite(Texture2D texture);
Sprite* GetSpriteComponent(GameObject* object);