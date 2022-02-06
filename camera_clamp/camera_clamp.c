
/**********************************************************************************************
*
*   raylib-extras, examples-c * examples for Raylib in C++
*
*   camera clamp * an example showing how to clamp a camera to a map size
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

	// make a player
	Vector2 pos = { 100,100 };

	// Main game loop
	while (!WindowShouldClose())    // Detect window close button or ESC key
	{
		// move the player
		float move = 200 * GetFrameTime();

		// update the player position based on movement keys
		if (IsKeyDown(KEY_W))
			pos.y -= move;
		if (IsKeyDown(KEY_S))
			pos.y += move;

		if (IsKeyDown(KEY_A))
			pos.x -= move;
		if (IsKeyDown(KEY_D))
			pos.x += move;

		// clamp the player to the map
		// player can't be less that its radius from any edge
		if (pos.x < 25)
			pos.x = 25;

		if (pos.x > background.width-25)
			pos.x = background.width - 25;

		if (pos.y < 25)
			pos.y = 25;

		if (pos.y > background.height - 25)
			pos.y = background.height - 25;

		// set the camera target to follow the player
		cam.target = pos;

		// clamp the camera target to the rectangle that is inset by the offset 
		// camera target can't be closer to a wall than the offset, since the offset is at the center of the vissible screen.
		if (cam.target.x < cam.offset.x)
			cam.target.x = cam.offset.x;

		if (cam.target.x > background.width - cam.offset.x)
			cam.target.x = background.width - cam.offset.x;

		if (cam.target.y < cam.offset.y)
			cam.target.y = cam.offset.y;

		if (cam.target.y > background.height - cam.offset.y)
			cam.target.y = background.height - cam.offset.y;

		// draw the world
		BeginDrawing();

		ClearBackground(BLACK);

		BeginMode2D(cam);

		// draw the entire background image for the entire world. The camera will clip it to the screen
		DrawTexture(background, 0, 0, WHITE);

		// draw the player where they are in world space
		DrawCircleV(pos, 25, RED);

		EndMode2D();

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