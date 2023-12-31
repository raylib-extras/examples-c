/**********************************************************************************************
*
*   raylib-extras, closest point on a line to another point
*
*   LICENSE: MIT
*
*   Copyright (c) 2024 Jeffery Myers
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


// the line we want to test against
Vector2 LineSP = { 100 , 100 };
Vector2 LineEP = { 500 , 500 };

// the point that is on the line that is closest to the mouse
Vector2 NearestPoint = { 0 };

// is the nearest point actualy on the line, or outside it's SP/EP
bool OnLine = false;

void GameInit()
{
    LineEP.x = GetScreenWidth() - LineSP.x;
    LineEP.y = GetScreenHeight() - LineSP.y;
}

bool GameUpdate()
{
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        LineSP = GetMousePosition();
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    {
        LineEP = GetMousePosition();
    }
    else
    {
        // compute the vector for the line
        Vector2 lineVec = Vector2Subtract(LineEP, LineSP);

        // see how long the line is
        float magnatude = Vector2Length(lineVec);

        // normalize the vector so it's just a direction
        lineVec = Vector2Normalize(lineVec);

        // compute the vector from a point on the line to the mouse
        Vector2 vecToMouse = Vector2Subtract(GetMousePosition(), LineSP);

        // the dot product is the distance along the vector from the point to the 'leg' of the triangle made
        //  between the vector to the mouse and the vector of the line, thus this is the distance from
        //  the start point to the nearest point on the line to the mouse
        float dot = Vector2DotProduct(vecToMouse, lineVec);

        // compute the actual point relative to the start point
        NearestPoint = Vector2Add(LineSP, Vector2Scale(lineVec, dot));

        // if the distance is less than 0, then the nearest point is behind the start point
        // if the distance is larger than the line length, then it's off the end
        OnLine = dot >= 0 && dot <= magnatude;
    }

    return true;
}

void Draw2D()
{
    // draw our line
    DrawLineV(LineSP, LineEP, WHITE);

    // draw the nearest point if we are not dragging a line
    if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        DrawCircleV(NearestPoint, 10, OnLine ? YELLOW : RED);

    DrawText("Drag to draw new line", 0, 0, 20, WHITE);
}

void GameDraw()
{
    BeginDrawing();
    ClearBackground(DARKGRAY);
    Draw2D();
    DrawFPS(0, GetScreenHeight() - 20);
    EndDrawing();
}

int main()
{
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 800, "Point on line nearest another point");
    SetTargetFPS(300);

    GameInit();

    while (!WindowShouldClose())
    {
        if (!GameUpdate())
            break;

        GameDraw();
    }

    CloseWindow();
    return 0;
}