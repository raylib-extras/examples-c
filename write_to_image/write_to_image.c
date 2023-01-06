/*
Raylib example file.
This example shows drawing to an image and updating a texture for display

*/

#include "raylib.h"

int main ()
{
	// set up the window
	InitWindow(320, 200, "Write To Image");
	SetTargetFPS(800);

	// create an image the same size as the window for us to draw too
	Image imageBuffer = GenImageColor(GetScreenWidth(), GetScreenHeight(), BLACK);

	// make a texture that matches the format and size of the image
	Texture displayTexture = LoadTextureFromImage(imageBuffer);

	// some state data for us to use while drawing
	bool useRed = true;

	int x = 0;
	int y = 0;

	// game loop
	while (!WindowShouldClose())
	{
		// draw a pixel to the image
		ImageDrawPixel(&imageBuffer, x, y, useRed ? RED : BLUE);
		// Note, you can also directly access imageBuffer.data if you want to do raw pixel buffer manipulation
		
		// update to the next pixel.
		x++;
		if (x >= GetScreenWidth())
		{
			// we went off the end of the screen, move to the next line
			y++;
			x = 0;
		}

		if (y >= GetScreenHeight())
		{
			// we filled the screen start over with a different color
			y = 0;
			useRed = !useRed;
		}

		// update the texture with our new pixel data (this uploads the new pixel data to the GPU)
		UpdateTexture(displayTexture, imageBuffer.data);
		
		BeginDrawing();
		ClearBackground(WHITE);

		// display the texture from the GPU to the screen
		DrawTexture(displayTexture, 0, 0, WHITE);

		EndDrawing();
	}

	UnloadImage(imageBuffer);
	UnloadTexture(displayTexture);
	// cleanup
	CloseWindow();
	return 0;
}