#include "raylib.h"

int main ()
{
	InitWindow(1280,800,"Example");
	
	while (!WindowShouldClose())
	{
		BeginDrawing();
		ClearBackground(BLACK);
		
		EndDrawing();
	}
	CloseWindow();
	return 0;
}