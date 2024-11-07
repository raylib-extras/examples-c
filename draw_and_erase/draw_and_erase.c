/*
Drawing function with erase
*/

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "external/glad.h"

RenderTexture Canvas = { 0 };

int main()
{
    // set up the window
    InitWindow(1000, 800, "Draw and Erase");
    SetTargetFPS(800);

    Canvas = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());

    BeginTextureMode(Canvas);
    ClearBackground(BLANK);
    EndTextureMode();

    bool draw = true;
    Color DrawColor = BLUE;

    float brushSize = 10;

    // game loop
    while (!WindowShouldClose())
    {
        Vector2 mousePos = GetMousePosition();

        if (IsKeyPressed(KEY_E))
            draw = false;
        if (IsKeyPressed(KEY_R))
            draw = true;

        if (IsKeyPressed(KEY_ONE))
            DrawColor = BLUE;
        if (IsKeyPressed(KEY_TWO))
            DrawColor = RED;
        if (IsKeyPressed(KEY_THREE))
            DrawColor = YELLOW;

        if (IsKeyPressed(KEY_UP))
            brushSize += 1;

        if (IsKeyPressed(KEY_DOWN))
            brushSize -= 1;

        if (brushSize < 2)
            brushSize = 2;

        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
        {
            BeginTextureMode(Canvas);

            Color drawColor = DrawColor;

            if (!draw)
            {
                rlSetBlendFactors(GL_SRC_ALPHA, GL_SRC_ALPHA, GL_MIN);
                rlSetBlendMode(BLEND_CUSTOM);
                drawColor = BLANK;
            }

            Vector2 lastPos = Vector2Subtract(mousePos, GetMouseDelta());

            Vector2 strokeVector = Vector2Normalize(GetMouseDelta());
            float strokeDistance = Vector2Length(GetMouseDelta()) / 20;

            for (float i = 0; i <= 20; i += 1)
            {
                Vector2 pos = Vector2Add(lastPos, Vector2Scale(strokeVector, strokeDistance * i));
                DrawCircleV(pos, brushSize, drawColor);
            }

            // go back to normal
            rlSetBlendMode(BLEND_ALPHA);

            EndTextureMode();
        }

        BeginDrawing();
        ClearBackground(BLACK);
        DrawTextureRec(Canvas.texture, (Rectangle) { 0, 0, (float)Canvas.texture.width, -(float)Canvas.texture.height }, (Vector2) { 0, 0 }, WHITE);

        if (draw)
        {
            DrawCircleV(mousePos, brushSize, DrawColor);
            DrawCircleLinesV(mousePos, brushSize, WHITE);
        }
        else
        {
            DrawCircleLinesV(mousePos, brushSize, RED);
        }

        DrawRectangle(0, 0, 20, 20, BLUE);
        DrawText("1", 0, 0, 20, WHITE);
        DrawRectangle(30, 0, 20, 20, RED);
        DrawText("2", 30, 0, 20, WHITE);
        DrawRectangle(60, 0, 20, 20, YELLOW);
        DrawText("3", 60, 0, 20, BLACK);

        if (draw)
            DrawText("Drawing [erase(E)]", 10, 20, 20, WHITE);
        else
            DrawText("Erasing [draw(P)]", 10, 20, 20, WHITE);

        DrawText("Up/Down change brush size", 10, 40, 20, WHITE);

        EndDrawing();
    }

    UnloadRenderTexture(Canvas);
    // cleanup
    CloseWindow();
    return 0;
}