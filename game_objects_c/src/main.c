/*
Raylib example file.
This is an example main file for a simple raylib project.
Use this as a starting point or replace it with your code.

For a C++ project simply rename the file to .cpp and re-run the build script 

-- Copyright (c) 2020-2024 Jeffery Myers
--
--This software is provided "as-is", without any express or implied warranty. In no event 
--will the authors be held liable for any damages arising from the use of this software.

--Permission is granted to anyone to use this software for any purpose, including commercial 
--applications, and to alter it and redistribute it freely, subject to the following restrictions:

--  1. The origin of this software must not be misrepresented; you must not claim that you 
--  wrote the original software. If you use this software in a product, an acknowledgment 
--  in the product documentation would be appreciated but is not required.
--
--  2. Altered source versions must be plainly marked as such, and must not be misrepresented
--  as being the original software.
--
--  3. This notice may not be removed or altered from any source distribution.

*/

#include <stdlib.h>

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include "resource_dir.h"	// utility header for SearchAndSetResourceDir

#include "game_object.h"
#include "transform.h"
#include "sprite.h"
#include "shape.h"
#include "behavior.h"

typedef struct Scene
{
	GameObject* Objects;
	int ObjectCount;
}Scene;

Scene TheScene = { 0 };

Texture wabbit = { 0 };

void DrawShape(GameObject* object, Transform2D* transform);
void DrawSprite(GameObject* object, Transform2D* transform);
void DrawRenderable(GameObject* object);

void UpdateTransform(GameObject* object);
void UpdateRotation(GameObject* object);

void InitScene()
{
	TheScene.ObjectCount = 10;
	TheScene.Objects = malloc(sizeof(GameObject) * TheScene.ObjectCount);

	for (int i = 0; i < TheScene.ObjectCount; i++)
		InitalizeGameObject(TheScene.Objects + i);

	for (int index = 0; index < TheScene.ObjectCount; index++)
	{ 
		GameObjectAddComponent(TheScene.Objects + index, TransformComponent, CreateTransform());
		Transform2D* transform = GetTransformComponent(TheScene.Objects + index);
		transform->Position.x = (float)GetRandomValue(10, 1200);
		transform->Position.y = (float)GetRandomValue(10, 700);

		GameObjectAddComponent(TheScene.Objects + index, BehaviorComponent, CreateBahavior(UpdateTransform));

		float radius = (float)GetRandomValue(10, 30);
		GameObjectAddComponent(TheScene.Objects + index, ShapeComponent, CreateShape(radius));

        // add a child with sprite
		
		GameObject* child = AddChildObject(TheScene.Objects + index);
        GameObjectAddComponent(child, TransformComponent, CreateTransform());
        transform = GetTransformComponent(child);
		transform->Position.x = radius * 3;
		transform->Position.y = 0;
		transform->Rotation = (float)GetRandomValue(-180, 180);
		GameObjectAddComponent(child, SpriteComponent, CreateSprite(wabbit));

		GameObjectAddComponent(child, BehaviorComponent, CreateBahavior(UpdateRotation));
	}
}

void DestoryScene()
{
	for (int i = 0; i < TheScene.ObjectCount; i++)
		DestoryGameObject(TheScene.Objects + i);

	free(TheScene.Objects);
	TheScene.Objects = NULL;
	TheScene.ObjectCount = 0;
}

void DrawShape(GameObject* object, Transform2D* transform)
{
	rlPushMatrix();
	rlTranslatef(transform->Position.x, transform->Position.y, 0);
	rlRotatef(transform->Rotation, 0, 0, 1);

	Shape* shape = GetShapeComponent(object);
	DrawCircleV(Vector2Zero(), shape->Radius, BLUE);
	
	for (int child = 0; child < object->ChildCount; child++)
		DrawRenderable(object->Children + child);
	rlPopMatrix();
}

void DrawSprite(GameObject* object, Transform2D* transform)
{
    rlPushMatrix();
    rlTranslatef(transform->Position.x, transform->Position.y, 0);
    rlRotatef(transform->Rotation, 0, 0, 1);

	Sprite* sprite = GetSpriteComponent(object);
	DrawTextureV(sprite->Texture, (Vector2){sprite->Texture.width * 0.5f, sprite->Texture.height * 0.5f }, WHITE);

    for (int child = 0; child < object->ChildCount; child++)
        DrawRenderable(object->Children + child);
    rlPopMatrix();
}

void DrawRenderable(GameObject* object)
{
    Transform2D* transform = GetTransformComponent(object);
    if (transform == NULL)
        return;

    // shapes
    if (GameObjectHasComponent(object, ShapeComponent))
        DrawShape(object, transform);

    // sprites
    if (GameObjectHasComponent(object, SpriteComponent))
        DrawSprite(object, transform);
}

void DrawRenderables()
{
	for (int i = 0; i < TheScene.ObjectCount; i++)
	{
		DrawRenderable(TheScene.Objects + i);
	}
}

void UpdateTransform(GameObject* object)
{
    Transform2D* transform = GetTransformComponent(object);

    if (transform == NULL)
        return;

    transform->Position.x += GetFrameTime() * 20;
    transform->Position.y += GetFrameTime() * 10;

    if (transform->Position.x > 1200)
        transform->Position.x = 0;
    if (transform->Position.y > 700)
        transform->Position.y = 0;
}

void UpdateRotation(GameObject* object)
{
    Transform2D* transform = GetTransformComponent(object);

    if (transform == NULL)
        return;

	transform->Rotation += GetFrameTime() * 45;

	while (transform->Rotation > 180)
		transform->Rotation -= 360;
}

void ProcessBehavior(GameObject* object)
{
    Behavior* behavior = GetBahaviorComponent(object);

    if (behavior == NULL)
        return;

	if (behavior != NULL)
		behavior->UpdateFunction(object);

    for (int child = 0; child < object->ChildCount; child++)
		ProcessBehavior(object->Children + child);
}

void ProcessBehaviors()
{
	for (int i = 0; i < TheScene.ObjectCount; i++)
		ProcessBehavior(TheScene.Objects + i);
}

int main ()
{
	// Tell the window to use vsync and work on high DPI displays
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

	// Create the window and OpenGL context
	InitWindow(1280, 800, "Hello Raylib");

	// Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
	SearchAndSetResourceDir("resources");

	wabbit = LoadTexture("wabbit_alpha.png");

	InitScene();

	// Load a texture from the resources directory

	// game loop
	while (!WindowShouldClose())		// run the loop until the user presses ESCAPE or presses the Close button on the window
	{
		ProcessBehaviors();

		// drawing
		BeginDrawing();

		// Setup the back buffer for drawing (clear color and depth buffers)
		ClearBackground(BLACK);

		DrawRenderables();
		
		// end the frame and get ready for the next one  (display frame, poll input, etc...)
		EndDrawing();
	}

	// cleanup
	// unload our texture so it can be cleaned up
	UnloadTexture(wabbit);

	DestoryScene();

	// destroy the window and cleanup the OpenGL context
	CloseWindow();
	return 0;
}
