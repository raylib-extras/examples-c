
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
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

// functions bound to lua
// examples of exposing native C/C++ functions to lua and getting arguments from lua
int LuaDrawText(lua_State* state)
{
    // get the first argument as a string. Note lua is 1 based
    size_t strSize = 0;
    const char* text = luaL_checklstring(state, 1, &strSize);

    // get the X and Y arguments
    int x = (int)luaL_checkinteger(state, 2);
    int y = (int)luaL_checkinteger(state, 3);

    // call our raylib function
    DrawText(text, x, y, 20, GREEN);

    return 0;
}

int LuaDrawCircle(lua_State* state)
{
    // Get X Y and radius argumentsS
    int x = (int)luaL_checkinteger(state, 1);
    int y = (int)luaL_checkinteger(state, 2);

    lua_Number radius = luaL_checknumber(state, 3);

    // call our raylib function
    DrawCircle(x, y, (float)radius, PURPLE);

    return 0;
}

// example of returning a value to lua
int LuaGetDeltaTime(lua_State* state)
{
    lua_pushnumber(state, (lua_Number)GetFrameTime());
    return 1;
}

// loads bound functions into lua state
void PushLuaAPI(lua_State* luaState)
{
    lua_register(luaState, "DrawText", LuaDrawText);
    lua_register(luaState, "DrawCircle", LuaDrawCircle);
    lua_register(luaState, "GetDeltaTime", LuaGetDeltaTime);
}

// runs the file as a lua script
void RunLuaScript(lua_State* luaState, const char* scriptFile)
{
    if (!scriptFile)
        scriptFile;

    if (luaL_dofile(luaState, scriptFile) == LUA_OK)
    {
        if (lua_pcall(luaState, 0, 0, 0) == LUA_OK)
        {
            lua_pop(luaState, lua_gettop(luaState));
        }
    }
}

int main()
{
    // set up the window
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 800, "Hello Lua");
    SetTargetFPS(250);

    // setup our lua state/context
    lua_State* luaState = luaL_newstate();
    luaL_openlibs(luaState);

    // push our exposed API functions into lua
    PushLuaAPI(luaState);

    // game loop
    while (!WindowShouldClose())
    {
        // drawing
        BeginDrawing();
        ClearBackground(BLACK);

        // push the current screen width to lua as the global variable "ScreenWidth"
        // example of pushing data to lua
        lua_pushnumber(luaState, (lua_Number)GetScreenWidth());
        lua_setglobal(luaState, "ScreenWidth");

        RunLuaScript(luaState, "resources/scripts/script.lua");

        // see if the script set a result global
        // example of getting data out of lua
        lua_getglobal(luaState, "result");

        if (lua_isstring(luaState, -1))
        {
            const char* message = lua_tostring(luaState, -1);
            lua_pop(luaState, 1);
            DrawText(TextFormat("Message from script: %s\n", message), 10, GetScreenHeight()-40, 20, WHITE);
        }

        EndDrawing();
    }

    lua_close(luaState);

    // cleanup
    CloseWindow();
    return 0;
}