#include "transform.h"

#include <stdlib.h>

Transform2D* CreateTransform()
{
	Transform2D* transform = malloc(sizeof(Transform));
	transform->Position = (Vector2){ 0,0 };
	transform->Rotation = 0;
	return transform;
}

Transform2D* GetTransformComponent(GameObject* object)
{
	if (object == NULL)
		return NULL;

	return (Transform2D*) GameObjectGetComponent(object, TransformComponent);
}