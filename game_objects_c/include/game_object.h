#pragma once

#include <stdbool.h>

typedef enum ComponentType
{
	TransformComponent,
	SpriteComponent,
	ShapeComponent,
	BehaviorComponent
}ComponentType;

typedef struct GameObjectComponent
{
	ComponentType CompType;
	void* CompValue;
}GameObjectComponent;

#define MAX_NAME_SIZE 32
typedef struct GameObject 
{
	char Name[MAX_NAME_SIZE];

	GameObjectComponent* Components;
	int CompoentSize;

	struct GameObject* Parent;
	struct GameObject* Children;
	int ChildCount;
}GameObject;

void InitalizeGameObject(GameObject* object);
GameObject* AddChildObject(GameObject* parent);
void DestoryGameObject(GameObject* object);

void GameObjectAddComponent(GameObject* object, ComponentType type, void* componentValue);
bool GameObjectHasComponent(GameObject* object, ComponentType type);
void* GameObjectGetComponent(GameObject* object, ComponentType type);