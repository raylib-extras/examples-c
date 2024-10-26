/*
Raylib example file.
This is an example main file for a simple raylib project.

*/

#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"

int main ()
{
	float NominalScreenWidth = 640;

	// set up the window
	InitWindow(1280, 800, "Camera Worldspace");
	SetTargetFPS(144);

	// create a camera that computes a nominal worldspace;
	Camera2D cam = { 0 };
	cam.zoom = GetScreenWidth() / NominalScreenWidth;

	// game loop
	while (!WindowShouldClose())
	{
		BeginDrawing();
		ClearBackground(BLACK);

		BeginMode2D(cam);

		DrawLine(0, 100, (int)NominalScreenWidth, 100, RED);
		DrawText(TextFormat("%0.0f units", (int)NominalScreenWidth), (int)NominalScreenWidth / 2, 100, 20, MAROON);

		DrawCircle(200, 200, 50, PURPLE);

		Vector2 worldspaceCursor = GetScreenToWorld2D(GetMousePosition(), cam);
		DrawText(TextFormat("World Space X%0.0f Y%0.0f", worldspaceCursor.x, worldspaceCursor.y), (int)worldspaceCursor.x, (int)worldspaceCursor.y - 10, 10, YELLOW);

		EndMode2D();

		DrawLine(0, GetScreenHeight()-100, GetScreenWidth(), GetScreenHeight() - 100, DARKGREEN);
		DrawText(TextFormat("%d pixels", GetScreenWidth()), GetScreenWidth() / 2, GetScreenHeight() - 100, 20, GREEN);

		Vector2 screenspaceCursor = GetMousePosition();
		DrawText(TextFormat("Screen Space X%0.0f Y%0.0f", screenspaceCursor.x, screenspaceCursor.y), (int)screenspaceCursor.x, (int)screenspaceCursor.y + 20, 20, ORANGE);

		EndDrawing();
	}

	// cleanup
	CloseWindow();
	return 0;
}
