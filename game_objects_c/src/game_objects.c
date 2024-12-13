#include "game_object.h"

#include <stdlib.h>


void InitalizeGameObject(GameObject* object)
{
	if (object == NULL)
		return;

	object->CompoentSize = 0;
	object->Components = NULL;
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
}

void GameObjectAddComponent(GameObject* object, ComponentType type, void* componentValue)
{
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