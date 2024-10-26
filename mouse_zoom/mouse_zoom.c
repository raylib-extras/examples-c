/*
Raylib example file.
This is an example main file for a simple raylib project.

*/

#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"

int main ()
{
	// set up the window
	InitWindow(1280, 800, "Mouse Zoom");
	SetTargetFPS(144);

	Camera2D cam = { 0 };
	cam.zoom = 1;

	// game loop
	while (!WindowShouldClose())
	{
		// translate based on right click
		if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
		{
			Vector2 delta = GetMouseDelta();
			delta = Vector2Scale(delta, -1.0f / cam.zoom);

			cam.target = Vector2Add(cam.target, delta);
		}

		// zoom based on wheel
		float wheel = GetMouseWheelMove();
		if (wheel != 0)
		{
			// get the world point that is under the mouse
			Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), cam);
			
			// set the offset to where the mouse is
			cam.offset = GetMousePosition();

			// set the target to match, so that the camera maps the world space point under the cursor to the screen space point under the cursor at any zoom
			cam.target = mouseWorldPos;

			// zoom
			cam.zoom += wheel * 0.125f;
			if (cam.zoom < 0.125f)
				cam.zoom = 0.125f;
		}

		// drawing
		BeginDrawing();
		ClearBackground(BLACK);

		BeginMode2D(cam);

		// draw the 3d grid, rotated 90 degrees and centered around 0,0 just so we have something in the XY plane
        rlPushMatrix();
        rlTranslatef(0, 25 * 50, 0);
        rlRotatef(90, 1, 0, 0);
        DrawGrid(100, 50);
        rlPopMatrix();

		// draw a thing
		DrawCircle(100, 100, 50, YELLOW);
		EndMode2D();

		DrawText("Right drag to move, mouse wheel to zoom", 2, 2, 20, WHITE);
		
		EndDrawing();
	}

	// cleanup
	CloseWindow();
	return 0;
}