
#include "raylib.h"

#include <math.h>

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

static const int screenWidth = 1200;
static const int screenHeight = 800;

static const int VirtualScreenWidth = 18 * 10;

static Camera2D ViewCamera = { 0 };


void UpdateDrawFrame(void);          // Update and draw one frame

void InitResources(void);
void CleanupResources(void);

int main(void)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(screenWidth, screenHeight, "Modern Style Pixel Game");

    InitAudioDevice();      // Initialize audio device

    InitResources();
#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
    SetTargetFPS(300);       // Set a sanity cap

    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        UpdateDrawFrame();
    }
#endif

    CloseAudioDevice();     // Close audio context

    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}


Texture2D Grass = { 0 };
Texture2D Player = { 0 };
Texture2D Obstacle = { 0 };

Vector2 PlayerPos = { 0, -24 };

Shader SnapShader = { 0 };

bool UseShader = true;

void InitResources(void)
{
    SnapShader = LoadShader("resources/snap.vs", 0);

    Grass = LoadTexture("resources/tile_0022.png");
    SetTextureFilter(Grass, TEXTURE_FILTER_POINT);

    Player = LoadTexture("resources/character_0004.png");
    SetTextureFilter(Player, TEXTURE_FILTER_POINT);

    Obstacle = LoadTexture("resources/tile_0026.png");
    SetTextureFilter(Obstacle, TEXTURE_FILTER_POINT);
}

void CleanupResources(void)
{
    UnloadShader(SnapShader);
    UnloadTexture(Grass);
    UnloadTexture(Player);
    UnloadTexture(Obstacle);
}

// Update and draw game frame
void UpdateDrawFrame(void)
{
    if (IsKeyPressed(KEY_SPACE))
        UseShader = !UseShader;

    float speed = GetFrameTime() * 100; // 100 virtual units per second

    if (IsKeyDown(KEY_D))
        PlayerPos.x += speed;
    if (IsKeyDown(KEY_A))
        PlayerPos.x -= speed;

    ViewCamera.zoom = floorf(GetScreenWidth() / (float)VirtualScreenWidth);

    ViewCamera.offset.y = (float)GetScreenHeight();
    ViewCamera.offset.x = 36 * ViewCamera.zoom;
    ViewCamera.target.y = 18;

    ViewCamera.target.x = PlayerPos.x;

    float screenOriginInCamera = floorf(GetScreenToWorld2D((Vector2) { 0, 0 }, ViewCamera).x);
    float screenEdgeInCamera = floorf(GetScreenToWorld2D((Vector2) { (float)GetScreenWidth(), (float)GetScreenHeight() }, ViewCamera).x + 0.5f);

    int tileStartX = (int)(screenOriginInCamera / 18) -1;
    int tileEndX = (int)(screenEdgeInCamera / 18) + 1;

    BeginDrawing();
    ClearBackground(SKYBLUE);

    BeginMode2D(ViewCamera);
    if (UseShader)
        BeginShaderMode(SnapShader);

    // draw just the tiles in view
    for (int i = tileStartX; i <= tileEndX; i++)
    {
        DrawTexture(Grass, i * 18, 0, WHITE);

        if (i > 0)
        {
            if (i % 5 == 0)
                DrawTexture(Obstacle, i * 18, -18, WHITE);

            if (i % 9 == 0)
                DrawTexture(Obstacle, i * 18, -18, GRAY);
        }
    }

    DrawTextureEx(Player, PlayerPos, 0, 1.0f, WHITE);

    if (UseShader)
        EndShaderMode();
    EndMode2D();
    
    DrawFPS(0, 0);
    DrawText(TextFormat("Zoom = %0.1f", ViewCamera.zoom), 10, 20, 20, BLACK);

    if (UseShader)
        DrawText("Using Snap Shader [press space]", 10, 40, 20, DARKBROWN);
    else
        DrawText("Not Using Snap Shader [press space]", 10, 40, 20, DARKBROWN);
    EndDrawing();
    //----------------------------------------------------------------------------------
}
