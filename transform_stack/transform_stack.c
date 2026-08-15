/*
Raylib transform stack example - an example of using the rlgl matrix stack to create a solar system with a moon and a ship orbiting around it.
The ship rotates around the moon, which in turn rotates around the sun.
The user can control the distance of the moon from the sun using the up and down arrow keys.
*/

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

bool SearchAndSetResourceDir(const char* folderName);

int main()
{
	// set up the window
	InitWindow(1280, 800, "In Thust We Trust");
	SetTargetFPS(240);
	SearchAndSetResourceDir("resources");
	Texture ship = LoadTexture("Ship.png");

	float shipAngle = -90; // default graphic faces in the Y- direction so that is -90 degrees
	float moonAngle = 0;

	float moonDistance = 350;
	// game loop
	while (!WindowShouldClose())
	{
		BeginDrawing();
		ClearBackground(DARKGRAY);

		if (IsKeyDown(KEY_UP)) moonDistance += GetFrameTime() * 100;
		if (IsKeyDown(KEY_DOWN)) moonDistance -= GetFrameTime() * 100;

		moonAngle += GetFrameTime() * 10;
		shipAngle += GetFrameTime() * 90;

		rlPushMatrix();
		rlTranslatef(GetScreenWidth() / 2, GetScreenHeight() / 2, 0);


		DrawCircle(0, 0, 100, SKYBLUE);

		rlPushMatrix();
		rlRotatef(moonAngle, 0, 0, 1);
		rlTranslatef(moonDistance, 0, 0);
		DrawCircle(0, 0, 25, GRAY);

		rlPushMatrix();
		rlRotatef(shipAngle, 0, 0, 1);
		rlTranslatef(100, 0, 0);

		rlRotatef(180, 0, 0, 1);
		DrawTexture(ship, -ship.width / 2, -ship.height / 2, WHITE);

		rlPopMatrix();

		rlPopMatrix();
		rlPopMatrix();

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
