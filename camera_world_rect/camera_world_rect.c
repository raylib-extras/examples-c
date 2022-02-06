
/**********************************************************************************************
*
*   raylib-extras, examples-c * examples for Raylib in C++
*
*   camera in world rect * an example showing how to get the screen rectangle in world space for a 2d camera
*
*   LICENSE: ZLib
*
*   Copyright (c) 2022 Jeffery Myers
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

int main(void)
{
	// Initialization
	//--------------------------------------------------------------------------------------
	const int screenWidth = 800;
	const int screenHeight = 450;

	InitWindow(screenWidth, screenHeight, "raylib [camera] example -  camera clamp");

	// make a huge image for the map, a checkerboard with a red outline
	Image img = GenImageChecked(2000, 2000, 128, 128, BLACK, DARKBLUE);
	ImageDrawRectangleLines(&img, (Rectangle) { 0, 0, 2000, 2000 }, 10, RED);
	Texture background = LoadTextureFromImage(img);
	UnloadImage(img);

	SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

	// setup a camera
	Camera2D cam = { 0 };
	cam.zoom = 1;
	// center the camera on the middle of the screen
	cam.offset = (Vector2){ screenWidth / 2, screenHeight / 2 };


	// Main game loop
	while (!WindowShouldClose())    // Detect window close button or ESC key
	{

		if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
		{
			Vector2 delta = Vector2Scale(GetMouseDelta(), -1.0f / cam.zoom);

			// set the camera target to follow the player
			cam.target = Vector2Add(cam.target, delta);
		}

		cam.zoom += GetMouseWheelMove() * 0.125f;
		if (cam.zoom < 0.125f)
			cam.zoom = 0.125f;

		Vector2 screenInWorldStart = GetScreenToWorld2D((Vector2) { 0, 0 }, cam);
		Vector2 screenInWorldEnd = GetScreenToWorld2D((Vector2) { GetScreenWidth(), GetScreenHeight() }, cam);
		Rectangle screenInWorldRect = (Rectangle){ screenInWorldStart.x, screenInWorldStart.y, screenInWorldEnd.x - screenInWorldStart.x, screenInWorldEnd.y - screenInWorldStart.y };

		// draw the world
		BeginDrawing();

		ClearBackground(BLACK);

		BeginMode2D(cam);

		// draw the entire background image for the entire world. The camera will clip it to the screen
		DrawTexture(background, 0, 0, WHITE);

		DrawRectangleLinesEx(screenInWorldRect, 3 / cam.zoom, GREEN);

		EndMode2D();

		DrawText(TextFormat("Screen In World x%f y%f w%f h%f", screenInWorldRect.x, screenInWorldRect.y, screenInWorldRect.width, screenInWorldRect.height),
			4, 4, 20, WHITE);
		EndDrawing();
		//----------------------------------------------------------------------------------
	}

	// De-Initialization
	//--------------------------------------------------------------------------------------
	UnloadTexture(background);
	CloseWindow();        // Close window and OpenGL context
	//--------------------------------------------------------------------------------------

	return 0;
}