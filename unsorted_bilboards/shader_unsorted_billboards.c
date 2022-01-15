/**********************************************************************************************
*
*   raylib-extras, examples-c * examples for Raylib in C++
*
*   unsorted billboards * an example of disabling depth writes for transperant pixels
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

    InitWindow(screenWidth, screenHeight, "raylib [shaders] example - unsorted billboards");
	SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

    // setup a camera
    Camera cam = { 0 };
    cam.position.z = -10;
    cam.position.y = 1;
    cam.up.y = 1;
    cam.fovy = 45;

    // load our tree
    Texture tx = LoadTexture("resources/tree_single.png");

    // Load a fragment shader that will discard pixels with 0 alpha
    Shader shader = LoadShader(NULL, "resources/discard_alpha.fs");

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // rotate the camera around the center
        Matrix mat = MatrixRotate((Vector3) { 0, 1, 0 }, DEG2RAD *(GetFrameTime() * 90));
        cam.position = Vector3Transform(cam.position, mat);


        BeginDrawing();
            ClearBackground(WHITE);

            BeginMode3D(cam);
 
            // draw a reference cube
            DrawCube((Vector3) { 0, 0, 0 }, 1, 1, 1, RED);

            // begin our shader mode to discard 0 alpha pixels
            BeginShaderMode(shader);

            // draw 4 trees in any order
            DrawBillboard(cam, tx, (Vector3) { 2, 1, 0 }, 3, WHITE);
            DrawBillboard(cam, tx, (Vector3) { -2, 1, 0 }, 3, WHITE);
            DrawBillboard(cam, tx, (Vector3) { 0, 1, 2 }, 3, WHITE);
            DrawBillboard(cam, tx, (Vector3) { 0, 1, -2 }, 3, WHITE);

            //end the shadermode
            EndShaderMode();

            EndMode3D();

        EndDrawing();
    }

    UnloadShader(shader);
    UnloadTexture(tx);

    CloseWindow();

    return 0;
}