#include "game_object.h"

#include <stdlib.h>


void InitalizeGameObject(GameObject* object)
{
	if (object == NULL)
		return;

	object->Name[0] = '/0';
	object->CompoentSize = 0;
	object->Components = NULL;

    object->Parent = NULL;

    object->ChildCount= 0;
    object->Children = NULL;
}

GameObject* AddChildObject(GameObject* parent)
{
	if (parent == NULL)
		return;

    parent->ChildCount++;
    parent->Children = realloc(parent->Children, sizeof(GameObject) * parent->ChildCount);
    GameObject * object = parent->Children + (parent->ChildCount - 1);

	InitalizeGameObject(object);

	return object;
}

void DestoryGameObject(GameObject* object)
{
	for (int i = 0; i < object->CompoentSize; i++)
	{
		free(object->Components[i].CompValue);
	}
	free(object->Components);
	object->CompoentSize = 0;
	object->Components = NULL;

	object->Parent = NULL;

	for (int i = 0; i < object->ChildCount; i++)
	{
		DestoryGameObject(object->Children + i);
	}

	object->ChildCount = 0;
	object->Children = NULL;
}

void GameObjectAddComponent(GameObject* object, ComponentType type, void* componentValue)
{
	if (GameObjectHasComponent(object, type))
		return;

	object->CompoentSize++;
	object->Components = realloc(object->Components, sizeof(GameObjectComponent) * object->CompoentSize);
	object->Components[object->CompoentSize - 1].CompType = type;
	object->Components[object->CompoentSize - 1].CompValue = componentValue;
}

bool GameObjectHasComponent(GameObject* object, ComponentType type)
{
	for (int i = 0; i < object->CompoentSize; i++)
	{
		if (object->Components[i].CompType == type)
			return true;
	}

	return false;
}

void* GameObjectGetComponent(GameObject* object, ComponentType type)
{
	for (int i = 0; i < object->CompoentSize; i++)
	{
		if (object->Components[i].CompType == type)
			return object->Components[i].CompValue;
	}

	return NULL;
}