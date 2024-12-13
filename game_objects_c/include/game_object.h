#pragma once

#include <stdbool.h>

typedef enum ComponentType
{
	TransformComponent,
	SpriteComponent,
	ShapeComponent,
}ComponentType;

typedef struct GameObjectComponent
{
	ComponentType CompType;
	void* CompValue;
}GameObjectComponent;

typedef struct GameObject 
{
	GameObjectComponent* Components;
	int CompoentSize;
}GameObject;

void InitalizeGameObject(GameObject* object);
void DestoryGameObject(GameObject* object);

void GameObjectAddComponent(GameObject* object, ComponentType type, void* componentValue);
bool GameObjectHasComponent(GameObject* object, ComponentType type);
void* GameObjectGetComponent(GameObject* object, ComponentType type);