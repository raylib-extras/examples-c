#include "sprite.h"

#include <stdlib.h>

Sprite* CreateSprite(Texture2D texture)
{
	Sprite* sprite = malloc(sizeof(Sprite));
	sprite->Texture = texture;
	return sprite;
}

Sprite* GetSpriteComponent(GameObject* object)
{
	if (object == NULL)
		return NULL;

	return (Sprite*)GameObjectGetComponent(object, SpriteComponent);
}