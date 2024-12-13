#pragma once

#include "game_object.h"

#include "raylib.h"

typedef struct Transform2D
{
	Vector2 Position;
	float Rotation;
}Transform2D;

Transform2D* CreateTransform();
Transform2D* GetTransformComponent(GameObject* object);