/**********************************************************************************************
*
*   raylib-extras, examples-c * examples for Raylib in C
*
*   full 3d billboards * an example of doing billboards that rotate to face the camera in any orientation
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
#include "rlgl.h"
#include <stddef.h>

void Draw3DBillboardRec(Camera camera, Texture2D texture, Rectangle source, Vector3 position, Vector2 size, Color tint)
{
    rlPushMatrix();

    // get the camera view matrix
    Matrix mat =MatrixInvert(MatrixLookAt(camera.position, camera.target, camera.up));
    // peel off just the rotation
    Quaternion quat = QuaternionFromMatrix(mat);
    mat = QuaternionToMatrix(quat);

    // apply just the rotation
    rlMultMatrixf(MatrixToFloat(mat));

    // translate backwards in the inverse rotated matrix to put the item where it goes in world space
    position = Vector3Transform(position, MatrixInvert(mat));
    rlTranslatef(position.x, position.y, position.z);

    // draw the billboard
    float width = size.x / 2;
    float height = size.y / 2;

    rlCheckRenderBatchLimit(6);

    rlSetTexture(texture.id);

    // draw quad
    rlBegin(RL_QUADS);
    rlColor4ub(tint.r, tint.g, tint.b, tint.a);
    // Front Face
    rlNormal3f(0.0f, 0.0f, 1.0f);                  // Normal Pointing Towards Viewer

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
    rlPopMatrix();
}

void Draw3DBillboard(Camera camera, Texture2D texture, Vector3 position, float size, Color tint)
{
    Draw3DBillboardRec(camera, texture, (Rectangle){ 0,0,(float)texture.width,(float)texture.height }, position, (Vector2){ size,size }, tint);
}

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [shaders] example - Full 3D billboards");
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
        cam.position.y = sinf((float)GetTime()) * 5;

        BeginDrawing();
        ClearBackground(WHITE);

        BeginMode3D(cam);

        DrawGrid(10, 10);
        // draw a reference cube
        DrawCube((Vector3) { 0, 0, 0 }, 1, 1, 1, GRAY);

        DrawCube((Vector3){ 1, 0, 0 }, 0.25f, 0.25f, 0.25f, RED);
        DrawCube((Vector3){ 0, 0, 1 }, 0.25f, 0.25f, 0.25f, BLUE);

        Vector2 size = { 2,2 };

        // draw 4 billboards
        Draw3DBillboard(cam, box, (Vector3){ 4, 0, 0 }, 2, WHITE);
        Draw3DBillboard(cam, box, (Vector3){ -4, 0, 0 }, 2, WHITE);
        Draw3DBillboard(cam, box, (Vector3){ 0, 0, 4 }, 2, WHITE);
        Draw3DBillboard(cam, box, (Vector3){ 0, 0, -4 }, 2, WHITE);

        EndMode3D();

        EndDrawing();
    }

    CloseWindow();

    return 0;
}