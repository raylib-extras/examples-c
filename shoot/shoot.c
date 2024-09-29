/**********************************************************************************************
*
*   raylib-extras, examples-c * examples for Raylib in C
*
*   Shoot * an example of movement and shots
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
#include "rlgl.h"

// constants
const float BulletSpeed = 600.0f;
const float MaxBulletLife = 2.0f;
#define MaxBullets 64
const float ShotReloadTime = 0.125f;

#define MaxTreadMarks 32

const float TreadmarkGenerationTime = 0.1f;

const float RotationSpeed = 270;
const float MoveSpeed = 200;

// treadmark data
typedef struct Treadmark
{
    float Lifetime;
    Vector2 Position;
    float Angle;
}Treadmark;
Treadmark Treadmarks[MaxTreadMarks] = { 0 };

// player data
Vector2 PlayerPos = { 0, 0 };
float PlayerAngle = 0;
float LastShotTime = 0;
float LastTreadMarkTime = 0;

// bullet data
typedef struct Bullet
{
    float Lifetime;
    Vector2 Position;
    Vector2 Direction;
}Bullet;
Bullet Bullets[MaxBullets] = { 0 };

// initialization
void GameInit()
{
    // start the player in the center
    PlayerPos =(Vector2){ GetScreenWidth() * 0.5f, GetScreenHeight() * 0.5f};
}

// game logic update
bool GameUpdate()
{
    // rotate player
    if (IsKeyDown(KEY_A))
        PlayerAngle -= GetFrameTime() * RotationSpeed;
    if (IsKeyDown(KEY_D))
        PlayerAngle += GetFrameTime() * RotationSpeed;

    // turn the player angle into a normal vector so we can use it to modify positions below
    Vector2 facing = { cosf(PlayerAngle * DEG2RAD), sinf(PlayerAngle * DEG2RAD) };

    // move the player forward and backwards
    if (IsKeyDown(KEY_W))
        PlayerPos = Vector2Add(PlayerPos, Vector2Scale(facing, MoveSpeed * GetFrameTime()));
    if (IsKeyDown(KEY_S))
        PlayerPos = Vector2Add(PlayerPos, Vector2Scale(facing, MoveSpeed * GetFrameTime() * -0.5f)); // backwards is half speed

    // Optional TODO
    // ensure the player stays on screen or do collisions with any obstacles

    // update treadmarks so they fade out over time
    for (int i = 0; i < MaxTreadMarks; i++)
    {
        if (Treadmarks[i].Lifetime > 0)
            Treadmarks[i].Lifetime -= GetFrameTime();
    }

    // add a new teadmark
    LastTreadMarkTime -= GetFrameTime();
    while (LastTreadMarkTime <= 0)
    {
        LastTreadMarkTime += TreadmarkGenerationTime;
        for (int i = 0; i < MaxTreadMarks; i++)
        {
            if (Treadmarks[i].Lifetime <= 0)
            {
                Treadmarks[i].Lifetime = 1;
                Treadmarks[i].Position = PlayerPos;
                Treadmarks[i].Angle = PlayerAngle;

                break;
            }
        }
    }

    // update the shot time
    if (LastShotTime > 0)
        LastShotTime -= GetFrameTime();

    // update bullet positions, and let old bullets die
    for (int i = 0; i < MaxBullets; i++)
    {
        if (Bullets[i].Lifetime > 0)
        {
            Bullets[i].Lifetime -= GetFrameTime();
            Bullets[i].Position = Vector2Add(Bullets[i].Position, Vector2Scale(Bullets[i].Direction, GetFrameTime()));

            // Optional TODO,
            // check if the bullet is off the screen and kill it
        }
    }

    // shoot
    if (IsKeyDown(KEY_SPACE) && LastShotTime <= 0)
    {
        for (int i = 0; i < MaxBullets; i++)
        {
            // find the first empty bullet slot
            if (Bullets[i].Lifetime <= 0)
            {
                // add a new bullet a little distance away from the player
                Bullets[i].Position = Vector2Add(PlayerPos, Vector2Scale(facing, 30));
                Bullets[i].Direction = Vector2Scale(facing, BulletSpeed); // scale the bullet direction by the speed
                    
                Bullets[i].Lifetime = MaxBulletLife;
                LastShotTime = ShotReloadTime;

                // Optional TODO
                // play a sound
                break;
            }
        }
    }

    return true;
}

// rendering
void GameDraw()
{
    BeginDrawing();
    ClearBackground(DARKGRAY);

    // track how many active bullets and marks there are
    int marks = 0;
    int bullets = 0;

    // draw each treadmark, fading it out over time
    for (int i = 0; i < MaxTreadMarks; i++)
    {
        if (Treadmarks[i].Lifetime > 0)
        {
            // set the transform matrix to where the treadmark is
            rlPushMatrix();
            rlTranslatef(Treadmarks[i].Position.x, Treadmarks[i].Position.y, 0);
            rlRotatef(Treadmarks[i].Angle, 0, 0, 1);

            Color color = ColorAlpha(GRAY, Treadmarks[i].Lifetime * 0.75f);

            DrawRectangleRec((Rectangle){ -15,10, 30,5 }, color);
            DrawRectangleRec((Rectangle){ -15,-15, 30,5 }, color);
            rlPopMatrix();
            marks++;
        }
    }

    // draw the player
    // set the transform matrix to where the player is and it's angle
    rlPushMatrix();
    rlTranslatef(PlayerPos.x, PlayerPos.y, 0);
    rlRotatef(PlayerAngle, 0, 0, 1);

    // draw a little tank out of rectangles
    DrawRectangleRec((Rectangle) { -10, -10, 20, 20 }, GREEN);
    DrawRectangleRec((Rectangle) { 0,-3, 30,6 }, DARKGREEN);
    DrawRectangleRec((Rectangle) { -15,10, 30,5 }, BLACK);
    DrawRectangleRec((Rectangle) { -15,-15, 30,5 }, BLACK);

    // reset the transform matrix
    rlPopMatrix();

    // draw each bullet
    for (int i = 0; i < MaxBullets; i++)
    {
        if (Bullets[i].Lifetime > 0)
        {
            DrawCircleV(Bullets[i].Position, 5, RED);
            bullets++;
        }
    }

    // instructions and stats
    DrawFPS(0, 0);
    DrawText("A/D = rotation W/S = movement Space = shoot", 2, GetScreenHeight() - 20, 20, BLACK);
    DrawText(TextFormat("%d Bullets, %d Marks", bullets, marks), GetScreenWidth() - 200, GetScreenHeight() - 20, 20, RED);
    EndDrawing();
}

int main()
{
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(1280, 800, "Shoot");
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