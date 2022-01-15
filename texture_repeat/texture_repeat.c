
/**********************************************************************************************
*
*   raylib-extras, examples-c * examples for Raylib in C++
*
*   texture repeat * an example showing how to repeat a texture with a camera
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

    InitWindow(screenWidth, screenHeight, "raylib [textures] example - texture repeat");

    Image img = GenImageChecked(256, 256, 128, 128, BLACK, DARKBLUE);
    Texture background = LoadTextureFromImage(img);
    UnloadImage(img);

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

    Camera2D cam = { 0 };
    cam.zoom = 1;

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        float move = 200 * GetFrameTime();
  
        if (IsKeyDown(KEY_W))
            cam.target.y -= move;
		if (IsKeyDown(KEY_S))
			cam.target.y += move;

		if (IsKeyDown(KEY_A))
			cam.target.x -= move;
		if (IsKeyDown(KEY_D))
			cam.target.x += move;

        cam.zoom += GetMouseWheelMove() * 0.0125f;
        if (cam.zoom < 0.0001f)
            cam.zoom = 0.0001f;

        BeginDrawing();

            ClearBackground(GREEN);

            BeginMode2D(cam);

            Rectangle rec = { cam.target.x, cam.target.y, GetScreenWidth() / cam.zoom, GetScreenHeight() / cam.zoom };
            DrawTexturePro(background, rec, rec, Vector2Zero(), 0, WHITE);

            DrawRectangleRec((Rectangle){ 100,100,100,100 }, RED);
            DrawRectangleRec((Rectangle) { 500, 400, 100, 100 }, RED);

            EndMode2D();

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}