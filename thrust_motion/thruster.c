/*
Raylib thruster example
Simple example showing the math for thrust based motion
*/

#include "raylib.h"
#include "raymath.h"

bool SearchAndSetResourceDir(const char* folderName);

bool UseGravity = false;
bool UseFriction = false;

int main()
{
    // set up the window
    InitWindow(1280, 800, "In Thust We Trust");
    SetTargetFPS(240);
    SearchAndSetResourceDir("resources");
    Texture ship = LoadTexture("Ship.png");

    float shipAngle = -90; // default graphic faces in the Y- direction so that is -90 degrees
    Vector2 shipPosition = { GetScreenWidth() * 0.5f, GetScreenHeight() * 0.5f };    // center us in the screen
    Vector2 shipVelocity = { 0,0 };    // we start not moving

    // game loop
    while (!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_SPACE))
            UseGravity = !UseGravity;

        if (IsKeyPressed(KEY_F))
            UseFriction = !UseFriction;

        // rotation
        float rotationDelta = 180.0f * GetFrameTime();
        if (IsKeyDown(KEY_A))
            shipAngle -= rotationDelta;
        if (IsKeyDown(KEY_D))
            shipAngle += rotationDelta;
        
        // compute the facing vector of the ship from the angle, the 
        Vector2 shipFacing = (Vector2){ cosf((shipAngle) * DEG2RAD), sinf((shipAngle) * DEG2RAD) };

        // thrust
        // compute the thrust(acceleration) to add this frame (use time)
        float thrustAmount = 0;
        if (IsKeyDown(KEY_W))
            thrustAmount = 300 * GetFrameTime();

        // see how much thrust we are going to add to our motion vector
        Vector2 thrustVector = Vector2Scale(shipFacing, thrustAmount);

        // add thrust to our velocity (units/second)
        shipVelocity = Vector2Add(shipVelocity, thrustVector);

        // add in gravity
        Vector2 gravity = { 0, 150.0f };
  
        // add friction
        if (UseFriction)
         {
            float frictionAmount = 100.0f * GetFrameTime();
            if (frictionAmount > Vector2Length(shipVelocity))
                frictionAmount = Vector2Length(shipVelocity);

            Vector2 frictionVector = Vector2Scale(Vector2Normalize(shipVelocity), -frictionAmount);
            shipVelocity = Vector2Add(shipVelocity, frictionVector);
        }

        if (UseGravity)
            shipVelocity = Vector2Add(shipVelocity, Vector2Scale(gravity, GetFrameTime()));

        // clamp the velocity to some sane limit
        if (Vector2Length(shipVelocity) > 1000)
            shipVelocity = Vector2Scale(Vector2Normalize(shipVelocity), 1000);

        // if you want to add some friction, do it here, slowly reduce the length of shipVelocity

        // position
        // update our position based on our velocity for this frame
        shipPosition = Vector2Add(shipPosition, Vector2Scale(shipVelocity, GetFrameTime()));

        // collisions
        float radius = 25;

        if (shipPosition.x - radius < 0)
        {
            // we hit the left edge, bounce
            shipPosition.x = 0 + radius;
            shipVelocity.x *= -1;
        }

        if (shipPosition.x + radius > GetScreenWidth())
        {
            // we hit the right edge, bounce
            shipPosition.x = GetScreenWidth() - radius;
            shipVelocity.x *= -1;
        }

        if (shipPosition.y - radius < 0)
        {
            // we hit the top edge, bounce
            shipPosition.y = 0 + radius;
            shipVelocity.y *= -1;
        }

        if (shipPosition.y + radius > GetScreenHeight())
        {
            // we hit the top edge, bounce
            shipPosition.y = GetScreenHeight() - radius;
            shipVelocity.y *= -1;
        }

        BeginDrawing();
        ClearBackground(DARKGRAY);

        DrawRectangleLinesEx((Rectangle){ 1,1, (float)GetScreenWidth()-1, (float)GetScreenHeight()-1 }, 3, RED);

        if (thrustAmount > 0)
        {
            // draw a flame
            DrawTexturePro(ship,
                (Rectangle) { 0, 0, (float)ship.width, (float)ship.height },
                (Rectangle) { shipPosition.x, shipPosition.y, (float)ship.width * 0.5f, (float)ship.height*0.5f },
                (Vector2) { ship.width * 0.25f, ship.height * 0.7f },
                shipAngle - 90 /*compensate for our rotated ship graphic*/,
                ORANGE);
        }

        // draw the ship centered
        DrawTexturePro(ship,
            (Rectangle) { 0, 0, (float)ship.width, (float)ship.height },
            (Rectangle) { shipPosition.x, shipPosition.y, (float)ship.width, (float)ship.height },
            (Vector2) { ship.width * 0.5f, ship.height * 0.5f },
            shipAngle+90 /*compensate for our rotated ship graphic*/,
            WHITE);



        DrawLineEx(shipPosition, Vector2Add(shipPosition, Vector2Scale(shipFacing, 100)), 2, PURPLE);

        if (UseGravity)
            DrawText("Gravity is ON (Space)", 10, 10, 20, GREEN);
        else
            DrawText("Gravity is OFF (Space)", 10, 10, 20, RED);

        if (UseFriction)
            DrawText("Friction is ON (F)", 10, 40, 20, GREEN);
        else
            DrawText("Friction is OFF (F)", 10, 40, 20, RED);

        EndDrawing();
    }

    UnloadTexture(ship);
    // cleanup
    CloseWindow();
    return 0;
}


bool SearchAndSetResourceDir(const char* folderName)
{
    // check the working dir
    if (DirectoryExists(folderName))
    {
        ChangeDirectory(TextFormat("%s/%s", GetWorkingDirectory(), folderName));
        return true;
    }

    const char* appDir = GetApplicationDirectory();

    // check the applicationDir
    const char* dir = TextFormat("%s%s", appDir, folderName);
    if (DirectoryExists(dir))
    {
        ChangeDirectory(dir);
        return true;
    }

    // check one up from the app dir
    dir = TextFormat("%s../%s", appDir, folderName);
    if (DirectoryExists(dir))
    {
        ChangeDirectory(dir);
        return true;
    }

    // check two up from the app dir
    dir = TextFormat("%s../../%s", appDir, folderName);
    if (DirectoryExists(dir))
    {
        ChangeDirectory(dir);
        return true;
    }

    // check three up from the app dir
    dir = TextFormat("%s../../../%s", appDir, folderName);
    if (DirectoryExists(dir))
    {
        ChangeDirectory(dir);
        return true;
    }

    return false;
}
