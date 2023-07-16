/**********************************************************************************************
*
*   raylib-extras, examples-c * examples for Raylib in C
*
*   RLGL Example, an example of how to draw raw geometry in rlgl
*
*   LICENSE: ZLib
*
*   Copyright (c) 2023 Jeffery Myers
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
#include "rlgl.h"

void DrawQuad3d(Texture2D texture, float width, float height, Color tint)
{
	rlCheckRenderBatchLimit(6);

    // set our texture
	rlSetTexture(texture.id);

    // draw the entire texture
    Rectangle source = { 0,0,texture.width,texture.height };

	// draw quad
	rlBegin(RL_QUADS);

    // set vertex colors
	rlColor4ub(tint.r, tint.g, tint.b, tint.a);

    // define some normal
	rlNormal3f(0.0f, 0.0f, 1.0f);                  // Normal Pointing Towards Viewer

    // define each UV coordinate and each vertex.
    // every 4 verts makes a quad
	rlTexCoord2f((float)source.x / texture.width, (float)(source.y + source.height) / texture.height);
	rlVertex3f(-width, -height, 0);  // Bottom Left Of The Texture and Quad

	rlTexCoord2f((float)(source.x + source.width) / texture.width, (float)(source.y + source.height) / texture.height);
	rlVertex3f(+width, -height, 0);  // Bottom Right Of The Texture and Quad

	rlTexCoord2f((float)(source.x + source.width) / texture.width, (float)source.y / texture.height);
	rlVertex3f(+width, +height, 0);  // Top Right Of The Texture and Quad

	rlTexCoord2f((float)source.x / texture.width, (float)source.y / texture.height);
	rlVertex3f(-width, +height, 0);  // Top Left Of The Texture and Quad

	rlEnd();
	rlSetTexture(0);
}

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [3d] example - Drawing with RLGL");
    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

    // setup a camera
    Camera cam = { 0 };
    cam.position.z = -10;
    cam.position.y = 1;
    cam.up.y = 1;
    cam.fovy = 45;

    Texture2D box = LoadTexture("resources/panel_woodPaperDetailSquare.png");

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // rotate the camera around the center
        Matrix mat = MatrixRotate((Vector3) { 0, 1, 0 }, DEG2RAD * (GetFrameTime() * 45));
        cam.position = Vector3Transform(cam.position, mat);

        // move the camera up and down so that it has a X axis rotation too
        cam.position.y = sinf(GetTime()) * 5;

        // draw
        BeginDrawing();
        ClearBackground(WHITE);

        BeginMode3D(cam);

        DrawGrid(10, 10);

        rlDisableBackfaceCulling(); // so we can see the back side

        // just draw a quad with rlgl
        DrawQuad3d(box, 1,1, WHITE);

        EndMode3D();

        EndDrawing();
    }

    CloseWindow();

    return 0;
}