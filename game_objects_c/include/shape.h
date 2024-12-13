#pragma once
#include "game_object.h"

#include "raylib.h"

typedef struct Shape
{
	float Radius;
}Shape;

Shape* CreateShape(float radius);
Shape* GetShapeComponent(GameObject* object);