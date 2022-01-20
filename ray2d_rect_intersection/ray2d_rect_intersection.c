/*******************************************************************************************
*
*   raylib [Vector Maths] example - Circle In View Cone
*
*   Welcome to raylib!
*
*   You can find all basic examples on C:\raylib\raylib\examples folder or
*   raylib official webpage: www.raylib.com
*
*   Enjoy using raylib. :)
*
*   This example has been created using raylib 4.0 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2022 Jeffery Myers
*
********************************************************************************************/
#include "raylib.h"
#include "raymath.h"
#include "stdlib.h"

// intersection using the slab method
// https://tavianator.com/2011/ray_box.html#:~:text=The%20fastest%20method%20for%20performing,remains%2C%20it%20intersected%20the%20box.

bool RayIntersectRect(Rectangle rect, Vector2 origin, Vector2 direction)
{
	float minParam = -INFINITY, maxParam = INFINITY;

	if (direction.x != 0.0)
	{
		float txMin = (rect.x - origin.x) / direction.x;
		float txMax = ((rect.x + rect.width) - origin.x) / direction.x;

		minParam = max(minParam, min(txMin, txMax));
		maxParam = min(maxParam, max(txMin, txMax));
	}

	if (direction.y != 0.0)
	{
		float tyMin = (rect.y - origin.y) / direction.y;
		float tyMax = ((rect.y + rect.height) - origin.y) / direction.y;

		minParam = max(minParam, min(tyMin, tyMax));
		maxParam = min(maxParam, max(tyMin, tyMax));
	}

	return maxParam >= minParam;
}

int main(void)
{
	const int screenWidth = 800;
	const int screenHeight = 450;

	InitWindow(screenWidth, screenHeight, "raylib extras [Math2d] example - ray rect intersecction");

	SetTargetFPS(60); 

	Rectangle rect = { 100,100,100,100 };

	Vector2 origin = { 250,300 };
	Vector2 direction = { 0, -1 };

	// Main game loop
	while (!WindowShouldClose())    // Detect window close button or ESC key
	{
		float angleDelta = 0;
		if (IsKeyDown(KEY_LEFT))
			angleDelta += GetFrameTime() * 90;
		if (IsKeyDown(KEY_RIGHT))
			angleDelta += GetFrameTime() * -90;

		Matrix rotMat = MatrixRotateZ(angleDelta * DEG2RAD);
		direction = Vector2Transform(direction, rotMat);

		BeginDrawing();
		ClearBackground(BLACK);

		DrawRectangleRec(rect, RayIntersectRect(rect, origin, direction) ? RED : GRAY);
		DrawLineV(origin, Vector2Add(origin, Vector2Scale(direction, 500)), BLUE);
		DrawCircleV(origin, 10, YELLOW);

		EndDrawing();
	}
	CloseWindow();
	return 0;
}