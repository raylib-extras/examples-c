#include "shape.h"

#include <stdlib.h>

Shape* CreateShape(float radius)
{
	Shape* shape = malloc(sizeof(Shape));
	shape->Radius = radius;
	return shape;
}

Shape* GetShapeComponent(GameObject* object)
{
	if (object == NULL)
		return NULL;

	return (Shape*)GameObjectGetComponent(object, ShapeComponent);
}