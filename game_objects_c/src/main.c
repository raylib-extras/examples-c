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

#include "resource_dir.h"	// utility header for SearchAndSetResourceDir

#include "game_object.h"
#include "transform.h"
#include "sprite.h"
#include "shape.h"

typedef struct Scene
{
	GameObject* Objects;
	int ObjectCount;
}Scene;

Scene TheScene = { 0 };

Texture wabbit = { 0 };

void InitScene()
{
	TheScene.ObjectCount = 50;
	TheScene.Objects = malloc(sizeof(GameObject) * TheScene.ObjectCount);

	for (int i = 0; i < TheScene.ObjectCount; i++)
		InitalizeGameObject(TheScene.Objects + i);

	for (int index = 0; index < TheScene.ObjectCount; index++)
	{ 
		GameObjectAddComponent(TheScene.Objects + index, TransformComponent, CreateTransform());
		Transform2D* transform = GetTransformComponent(TheScene.Objects + index);
		transform->Position.x = (float)GetRandomValue(10, 1200);
		transform->Position.y = (float)GetRandomValue(10, 700);

		if (GetRandomValue(0,100) < 50)
			GameObjectAddComponent(TheScene.Objects + index, SpriteComponent, CreateSprite(wabbit));
		else
			GameObjectAddComponent(TheScene.Objects + index, ShapeComponent, CreateShape((float)GetRandomValue(10, 30)));
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
	Shape* shape = GetShapeComponent(object);
	DrawCircleV(transform->Position, shape->Radius, BLUE);
}

void DrawSprite(GameObject* object, Transform2D* transform)
{
	Sprite* sprite = GetSpriteComponent(object);
	DrawTextureV(sprite->Texture, transform->Position, WHITE);
}

void DrawRenderables()
{
	for (int i = 0; i < TheScene.ObjectCount; i++)
	{
		Transform2D* transform = GetTransformComponent(TheScene.Objects + i);
		if (transform == NULL)
			continue;

		// shapes
		if (GameObjectHasComponent(TheScene.Objects + i, ShapeComponent))
			DrawShape(TheScene.Objects + i, transform);

		// sprites
		if (GameObjectHasComponent(TheScene.Objects + i, SpriteComponent))
			DrawSprite(TheScene.Objects + i, transform);
	}
}

void UpdateTransforms()
{
	// update all transforms
	for (int i = 0; i < TheScene.ObjectCount; i++)
	{
		Transform2D* transform = GetTransformComponent(TheScene.Objects + i);

		if (transform == NULL)
			continue;

		transform->Position.x += GetFrameTime() * 20;
		transform->Position.y += GetFrameTime() * 10;

		if (transform->Position.x > 1200)
			transform->Position.x = 0;
		if (transform->Position.y > 700)
			transform->Position.y = 0;
	}
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
		UpdateTransforms();

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
