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

Rectangle Rects[3] = { {450,100,100,100}, {550,100,100,100} , {550,200,100,100 } };

void PointNearestRectanglePoint(Rectangle rect, Vector2 point, Vector2* nearest)
{
    // get the closest point on the vertical sides
    float hValue = rect.x;
    if (point.x > rect.x + rect.width)
        hValue = rect.x + rect.width;

    Vector2 vecToPoint = Vector2Subtract((Vector2) { hValue, rect.y }, point);
	// get the dot product between the ray and the vector to the point
	float dotForPoint = Vector2DotProduct((Vector2) { 0,-1 }, vecToPoint);
    Vector2 nearestPoint = { hValue,0 };

    if (dotForPoint < 0)
        nearestPoint.y = rect.y;
    else if (dotForPoint >= rect.height)
        nearestPoint.y = rect.y + rect.height;
    else
        nearestPoint.y = rect.y + dotForPoint;

    // get the closest point on the horizontal sides
	float vValue = rect.y;
	if (point.y > rect.y + rect.height)
		vValue = rect.y + rect.height;

	vecToPoint = Vector2Subtract((Vector2) { rect.x, vValue }, point);
	// get the dot product between the ray and the vector to the point
	dotForPoint = Vector2DotProduct((Vector2) { -1, 0 }, vecToPoint);
    *nearest = (Vector2){ 0,vValue };

	if (dotForPoint < 0)
        nearest->x = rect.x;
	else if (dotForPoint >= rect.width)
        nearest->x = rect.x + rect.width;
	else
        nearest->x = rect.x + dotForPoint;

    if (Vector2LengthSqr(Vector2Subtract(point,nearestPoint)) < Vector2LengthSqr(Vector2Subtract(point, *nearest)))
        *nearest = nearestPoint;
}

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib extras [movement] example - Circle Rectangle Collisions");

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

    Vector2 PlayerOrigin = { 300,300 };
    Vector2 PlayerDirection = { 1, 0 };

    float Radius = 25;

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        if (IsKeyDown(KEY_A))
        {
            Matrix mat = MatrixRotateZ(180 * DEG2RAD * GetFrameTime());
            PlayerDirection = Vector2Transform(PlayerDirection, mat);
        }

		if (IsKeyDown(KEY_D))
		{
			Matrix mat = MatrixRotateZ(-180 * DEG2RAD * GetFrameTime());
            PlayerDirection = Vector2Transform(PlayerDirection, mat);
		}

        Vector2 newPosOrigin = PlayerOrigin;

        if (IsKeyDown(KEY_W))
            newPosOrigin = Vector2Add(newPosOrigin, Vector2Scale(PlayerDirection, 200 * GetFrameTime()));

		if (IsKeyDown(KEY_S))
            newPosOrigin = Vector2Add(newPosOrigin, Vector2Scale(PlayerDirection, -200 * GetFrameTime()));

        Vector2 intersectPoint[2] = { { -100,-100 },{ -100,-100 } };
        bool collided = false;

        int collisionCount = 0;
        for (int i = 0; i < 3; i++)
        {
            Vector2 hitPoint = { -100,-100 };
            PointNearestRectanglePoint(Rects[i], newPosOrigin, &hitPoint);

            Vector2 vectorToHit = Vector2Subtract(hitPoint, newPosOrigin);

            bool inside = Vector2LengthSqr(vectorToHit) < Radius * Radius;

            if (inside)
			{
                collided = true;
                intersectPoint[collisionCount++] = hitPoint;

                // compute how far past the hit the radius is
                float distToHit = Vector2Length(vectorToHit);
                float backOff = Radius - distToHit;
                vectorToHit.x /= distToHit;
                vectorToHit.y /= distToHit;

                // back off along the vector to the hit by however far we went into the rectangle by
                newPosOrigin = Vector2Add(newPosOrigin, Vector2Scale(vectorToHit, -backOff));
            }
        }

        PlayerOrigin = newPosOrigin;

        BeginDrawing();
            ClearBackground(BLACK);

            for (int i = 0; i < 3; i++)
                DrawRectangleRec(Rects[i], RED);

            DrawCircleV(PlayerOrigin, collided ? 10 : 2, collided ? YELLOW : DARKGREEN);
            DrawCircleLines(PlayerOrigin.x, PlayerOrigin.y, Radius, DARKGREEN);
            DrawLineV(PlayerOrigin, Vector2Add(PlayerOrigin, Vector2Scale(PlayerDirection, Radius)), GREEN);
            for (int i = 0; i < 2; i++)
                DrawCircleV(intersectPoint[i], 5, PURPLE);

        EndDrawing();
    }
    CloseWindow();

    return 0;
}