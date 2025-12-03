/**********************************************************************************************
*
*   raylib-extras, rotated bounding box collision example
*   LICENSE: MIT
*
*   Copyright (c) 2025 Jeffery Myers
*
*   Permission is hereby granted, free of charge, to any person obtaining a copy
*   of this software and associated documentation files (the "Software"), to deal
*   in the Software without restriction, including without limitation the rights
*   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*   copies of the Software, and to permit persons to whom the Software is
*   furnished to do so, subject to the following conditions:
*
*   The above copyright notice and this permission notice shall be included in all
*   copies or substantial portions of the Software.
*
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
*   SOFTWARE.
*
**********************************************************************************************/

#include "raylib.h"
#include "raymath.h"


Rectangle Bounds = { 600,400, 200,50 };
float Rotation = -45;

Vector2 CollisionPos = { 100, 100 };
float Radius = 20;

bool Colliding = false;

void GameInit()
{
}

bool GameUpdate()
{
	// spin the rectangle
	Rotation += 20 * GetFrameTime();

	// move the point to the mouse pos if the button is down
	if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
	 CollisionPos = GetMousePosition();

	// rotate the mouse point into the space of the rotated rectangle by using the inverse rotation
	Vector2 vecToCollision = Vector2Subtract(CollisionPos , (Vector2){ Bounds.x, Bounds.y });
	vecToCollision = Vector2Rotate(vecToCollision , -Rotation * DEG2RAD);

	// check collision in rotated space
	Colliding = CheckCollisionCircleRec(vecToCollision, Radius, (Rectangle){ 0,0,Bounds.width,Bounds.height });
	return true;
}

void GameDraw()
{
	BeginDrawing();
	ClearBackground(DARKGRAY);

	DrawRectanglePro(Bounds, Vector2Zero(), Rotation, BLUE);
	DrawCircleV(CollisionPos, Radius, Colliding ? RED : PURPLE);

	EndDrawing();
}

int main()
{
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
	InitWindow(1280, 800, "Example");
	SetTargetFPS(144);

	GameInit();

	while (!WindowShouldClose())
	{
		if (!GameUpdate())
			break;
		
		GameDraw();
	}

	CloseWindow();
	return 0;
}