
/**********************************************************************************************
*
*   raylib-extras, examples-c * examples for Raylib in C++
*
*   lua embbd clamp * an example showing how to use lua scripting in a game
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

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

typedef struct 
{
    Vector2 Position;
    float Angle;
    float ReloadTime;
}Entity;

typedef struct
{
    Vector2 Position;
    Vector2 Velocity;
    float Lifetime;
}Bullet;

Entity Player = { 0 };

#define MAX_ENIMIES 3
#define MAX_BULLETS 100
Entity Enemies[MAX_ENIMIES] = { 0 };
Bullet Bullets[MAX_BULLETS] = { 0 };


// functions bound to lua
// these are a series of functions that let the lua behavior script get info and change the game state.

int LuaGetEnemyCount(lua_State* luaState)
{
    lua_pushinteger(luaState, MAX_ENIMIES);
    return 1;
}

int LuaGetEnemyPosX(lua_State* luaState)
{
    int index = (int)luaL_checkinteger(luaState, 1);
    lua_pushnumber(luaState, Enemies[index].Position.x);
    return 1;
}

int LuaGetEnemyPosY(lua_State* luaState)
{
    int index = (int)luaL_checkinteger(luaState, 1);
    lua_pushnumber(luaState, Enemies[index].Position.y);
    return 1;
}

int LuaMovePlayer(lua_State* luaState)
{
    int index = (int)luaL_checkinteger(luaState, 1);
    Enemies[index].Position.x = (float)luaL_checknumber(luaState, 2);
    Enemies[index].Position.y = (float)luaL_checknumber(luaState, 3);
    return 0;
}

int LuaGetEnemyAngle(lua_State* luaState)
{
    int index = (int)luaL_checkinteger(luaState, 1);
    lua_pushnumber(luaState, Enemies[index].Angle);
    return 1;
}

int LuaDistanceToPlayer(lua_State* luaState)
{
    int index = (int)luaL_checkinteger(luaState, 1);

    Vector2 vectorToPlayer = Vector2Subtract(Player.Position, Enemies[index].Position);

    lua_pushnumber(luaState, Vector2Length(vectorToPlayer));
    return 1;
}

int LuaTurnTowardPlayer(lua_State* luaState)
{
    int index = (int)luaL_checkinteger(luaState, 1);
    float speed = (float)luaL_checknumber(luaState, 2);

    Vector2 vectorToPlayer = Vector2Normalize(Vector2Subtract(Player.Position, Enemies[index].Position));

    float angle = atan2(vectorToPlayer.y, vectorToPlayer.x) * RAD2DEG;
    Enemies[index].Angle = angle;

    lua_pushboolean(luaState, true);
    return 1;
}

int LuaEnemyCanFire(lua_State* luaState)
{
    int index = (int)luaL_checkinteger(luaState, 1);

    lua_pushboolean(luaState, Enemies[index].ReloadTime <= 0);
    return 1;
}

int LuaEnemyFire(lua_State* luaState)
{
    int index = (int)luaL_checkinteger(luaState, 1);
    float speed = (float)luaL_checknumber(luaState, 2);

    bool canFire = false;

    if (Enemies[index].ReloadTime <= 0)
    {
        int slot = -1;
        // find an empty bullet slot
        for (int i = 0; i < MAX_BULLETS; i++)
        {
            if (Bullets[i].Lifetime <= 0)
            {
                slot = i;
                break;
            }
        }

        if (slot >= 0)
        {
            canFire = true;

            Bullets[slot].Velocity = (Vector2){ cosf(DEG2RAD * Enemies[index].Angle), sinf(DEG2RAD * Enemies[index].Angle) };
            Bullets[slot].Position = Vector2Add(Enemies[index].Position, Vector2Scale(Bullets[slot].Velocity, 25));
            Bullets[slot].Velocity = Vector2Scale(Bullets[slot].Velocity, speed);
            Bullets[slot].Lifetime = 3;

            Enemies[index].ReloadTime = 1;
        }
    }

    lua_pushboolean(luaState, canFire);
    return 1;
}

// loads bound functions into lua state
void PushLuaAPI(lua_State* luaState)
{
    lua_register(luaState, "GetEnemyCount", LuaGetEnemyCount);
    lua_register(luaState, "EnemyFire", LuaEnemyFire);
    lua_register(luaState, "EnemyCanFire", LuaEnemyCanFire);
    lua_register(luaState, "GetEnemyPosX", LuaGetEnemyPosX);
    lua_register(luaState, "GetEnemyPosY", LuaGetEnemyPosY);
    lua_register(luaState, "GetEnemyAngle", LuaGetEnemyAngle);
    lua_register(luaState, "MovePlayer", LuaMovePlayer);
    lua_register(luaState, "DistanceToPlayer", LuaDistanceToPlayer);
    lua_register(luaState, "TurnTowardPlayer", LuaTurnTowardPlayer);
}

// runs the file as a lua script
void RunLuaScript(lua_State* luaState, const char* scriptFile)
{
    if (!scriptFile)
        return;

    if (luaL_dofile(luaState, scriptFile) == LUA_OK)
    {
        if (lua_pcall(luaState, 0, 0, 0) == LUA_OK)
        {
            lua_pop(luaState, lua_gettop(luaState));
        }
    }
}

Texture PlayerTexture;
Texture EnemyTexture;
Texture BulletTexture;

void LoadResources()
{
    PlayerTexture = LoadTexture("resources/textures/player.png");
    EnemyTexture = LoadTexture("resources/textures/enemy.png");
    BulletTexture = LoadTexture("resources/textures/bullet.png");
}

void SetupGame()
{
    Player.Position = (Vector2){ GetScreenWidth() * 0.5f, GetScreenHeight() * 0.5f };
    Player.Angle = 0;

    for (int i = 0; i < MAX_ENIMIES; i++)
    {
        Enemies[i].Position = (Vector2){ (float)GetRandomValue(10,GetScreenWidth() - 10), (float)GetRandomValue(10,GetScreenHeight() - 10) };
        Enemies[i].Angle = (float)GetRandomValue(-180, 180);
        Enemies[i].ReloadTime = 0;
    }
}

void DoEnemyBehaviors(lua_State* luaState)
{
    for (int i = 0; i < MAX_ENIMIES; i++)
    {
        lua_pushinteger(luaState, (lua_Integer)i);
        lua_setglobal(luaState, "CurrentEnemy");

        RunLuaScript(luaState, "resources/scripts/enemy_behavior.lua");
    }
}

void UpdatePlayer()
{
    float rotationSpeed = GetFrameTime() * 180.0f;

    if (IsKeyDown(KEY_A))
        Player.Angle -= rotationSpeed;

    if (IsKeyDown(KEY_D))
        Player.Angle += rotationSpeed;

    Vector2 facingVector = (Vector2){ cosf(Player.Angle * DEG2RAD), sinf(Player.Angle * DEG2RAD) };

    Vector2 movementVector = Vector2Scale(facingVector, GetFrameTime() * 200);

    if (IsKeyDown(KEY_W))
        Player.Position = Vector2Add(Player.Position, movementVector);
    if (IsKeyDown(KEY_S))
        Player.Position = Vector2Subtract(Player.Position, movementVector);
}

void DrawEntity(Vector2 position, float angle, Texture2D sprite, Color tint)
{
    Rectangle sourceRect = { 0,0,(float)sprite.width, (float)sprite.height };
    Vector2 center = { sourceRect.width * 0.5f, sourceRect.height * 0.5f };
    Rectangle destRect = { position.x, position.y,sourceRect.width,sourceRect.height };

    DrawTexturePro(sprite, sourceRect, destRect, center,angle + 90, tint);
}

void UpdateGameState()
{
    UpdatePlayer();

    DrawEntity(Player.Position, Player.Angle, PlayerTexture, WHITE);

    for (int i = 0; i < MAX_ENIMIES; i++)
    {
        DrawEntity(Enemies[i].Position, Enemies[i].Angle, EnemyTexture, RED);

        if (Enemies[i].ReloadTime > 0)
            Enemies[i].ReloadTime -= GetFrameTime();
    }

    for (int i = 0; i < MAX_BULLETS; i++)
    {
        Bullets[i].Lifetime -= GetFrameTime();
        if (Bullets[i].Lifetime > 0)
        {
            Bullets[i].Position = Vector2Add(Bullets[i].Position, Vector2Scale(Bullets[i].Velocity, GetFrameTime()));
            DrawEntity(Bullets[i].Position, (float)GetTime() * 270, BulletTexture, YELLOW);
        }
    }
}

void DoFixedTimeStep(float dt)
{

}

int main()
{
    // set up the window
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(1280, 800, "Hello Lua");
    SetTargetFPS(250);

    LoadResources();

    SetupGame();

    // setup our lua state/context
    lua_State* scriptState = luaL_newstate();
    luaL_openlibs(scriptState);

    // push our exposed API functions into lua
    PushLuaAPI(scriptState);

    float accumulator = 0;
    float fixedTimeStep = 1.0f / 60.0f;

    // game loop
    while (!WindowShouldClose())
    {
        accumulator += GetFrameTime();
        while (accumulator < fixedTimeStep)
        {
            DoFixedTimeStep(fixedTimeStep);
            accumulator -= fixedTimeStep;
        }

        // drawing
        BeginDrawing();
        ClearBackground(BLACK);

        UpdateGameState();

        DoEnemyBehaviors(scriptState);
        EndDrawing();
    }

    lua_close(scriptState);

    // cleanup
    CloseWindow();
    return 0;
}