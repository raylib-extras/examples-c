#include "behavior.h"

#include <stdlib.h>


Behavior* CreateBahavior(void (*updateFunction)(GameObject*))
{
    Behavior* behavior = malloc(sizeof(Behavior));
    behavior->UpdateFunction = updateFunction;
    return behavior;
}

Behavior* GetBahaviorComponent(GameObject* object)
{
    if (object == NULL)
        return NULL;

    return (Behavior*)GameObjectGetComponent(object, BehaviorComponent);
}