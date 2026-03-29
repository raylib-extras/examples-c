#include "raylib.h"
#include "raymath.h"

Vector2 NominalScreenSize = { 320, 175 };
float NominalAspectRatio = 320.0f/175.0f;

Texture Ship = { 0 };
Texture NPC = { 0 };
Texture Bullet = { 0 };

Camera2D ViewportCamera = { 0 };

Rectangle LetterBoxBars[2] = { 0 };

Vector2 ShipPosition = { 0,0 };
float ShipSize = 20;
float ShotCooldown = 0.5f;
float LastShotAccumulator = 0.5f;

float LastSpawnAccumulator = 0.0f;
float SpawnCooldown = 2.0f;

typedef struct {
	Vector2 Position;
	Vector2 Velocity;
	bool Alive;
} Entity;

#define MaxNPCs 10
#define MaxBullets 100

Entity NPCs[MaxNPCs] = { 0 };
Entity Bullets[MaxBullets] = { 0 };

float BulletSize = 20.0f;
float NPCSize = 30;

void CheckCameraZoom()
{
	ViewportCamera.offset = (Vector2){ GetScreenWidth() * 0.5f, GetScreenHeight() * 0.5f };

	float zoomToWidth = GetScreenWidth() / NominalScreenSize.x;
	float zoomToHeight = GetScreenHeight() / NominalScreenSize.y;

	if (zoomToWidth < zoomToHeight)
	{
		// zoom to fit the width, so blackbar the top and bottom
		ViewportCamera.zoom = zoomToWidth;

		float nominalHeight = NominalScreenSize.y * zoomToWidth;
		float delta = GetScreenHeight() - nominalHeight;
		LetterBoxBars[0] = (Rectangle){ 0, 0, GetScreenWidth(), delta * 0.5f };
		LetterBoxBars[1] = (Rectangle){ 0, GetScreenHeight() - delta * 0.5f, GetScreenWidth(), delta * 0.5f };
	}
	else
	{
		// zoom to fit the width, so blackbar the sides
		ViewportCamera.zoom = zoomToHeight;
		float nominalWidth = NominalScreenSize.x * zoomToHeight;
		float delta = GetScreenWidth() - nominalWidth;
		LetterBoxBars[0] = (Rectangle){ 0, 0, delta * 0.5f, GetScreenHeight() };
		LetterBoxBars[1] = (Rectangle){ GetScreenWidth() - delta * 0.5f, 0, delta * 0.5f, GetScreenHeight() };
	}
}

void DrawSprite(Vector2 pos, Texture sprite, float size, float angle, Color tint)
{
	Vector2 offset = (Vector2){ size * 0.5f, size * 0.5f };
	Rectangle source = { 0, 0, sprite.width, sprite.height };
	Rectangle destination = { pos.x, pos.y, size, size };

	DrawTexturePro(sprite, source, destination, offset, angle, tint);
}

void DrawScene()
{
	DrawSprite(ShipPosition, Ship, ShipSize, 90.0f, WHITE);

	for (int i = 0; i < MaxBullets; i++)
	{
		if (Bullets[i].Alive)
			DrawSprite(Bullets[i].Position, Bullet,BulletSize, GetTime() * RAD2DEG * 90, YELLOW);
	}

	for (int i = 0; i < MaxNPCs; i++)
	{
		if (NPCs[i].Alive)
			DrawSprite(NPCs[i].Position, NPC, NPCSize, 90.0f, RED);
	}
}

void DrawSceneOverlay()
{
	Vector2 halfBounds = Vector2Scale(NominalScreenSize, 0.5f);
	DrawText(TextFormat("WS X%0.0f, Y%0.0f", -halfBounds.x, -halfBounds.y), -halfBounds.x, -halfBounds.y, 1, DARKGRAY);
	DrawText(TextFormat("WS X%0.0f, Y%0.0f", halfBounds.x, halfBounds.y), halfBounds.x -70, halfBounds.y-10, 1, DARKGRAY);
}

void DrawLetterboxOverlay()
{
	for (int i = 0; i < 2; i++)
		DrawRectangleRec(LetterBoxBars[i], BLACK);

	DrawFPS(2, 2);
	DrawText(TextFormat("Camera Zoom, %0.2f, Real Resolution W%d H%d", ViewportCamera.zoom, GetScreenWidth(), GetScreenHeight()), 120, 2, 20, WHITE);
}

bool UpdatePositionInBounds(Vector2* pos, Vector2 delta, float size)
{
	Vector2 halfBounds = Vector2Scale(NominalScreenSize, 0.5f);

	// check collsions in worldspace
	Vector2 newPos = Vector2Add(*pos, delta);
	bool hit = false;

	if (newPos.x - size * 0.5f < -halfBounds.x)
	{
		newPos.x = -halfBounds.x + size * 0.5f;
		hit = true;
	}
	if (newPos.x + size * 0.5f > halfBounds.x)
	{
		newPos.x = halfBounds.x - size * 0.5f;
		hit = true;
	}
	if (newPos.y - size * 0.5f < -halfBounds.y)
	{
		newPos.y = -halfBounds.y + size * 0.5f;
		hit = true;
	}
	if (newPos.y + size * 0.5f > halfBounds.y)
	{
		newPos.y = halfBounds.y - size * 0.5f;
		hit = true;
	}

	*pos = newPos;

	return hit;
}

void UpdateScene()
{
	Vector2 movement = { 0,0 };
	if (IsKeyDown(KEY_W))
		movement.y -= 1;
	if (IsKeyDown(KEY_S))
		movement.y += 1;
	if (IsKeyDown(KEY_A))
		movement.x -= 1;
	if (IsKeyDown(KEY_D))
		movement.x += 1;

	Vector2 moveVector = Vector2Scale(movement, 50 * GetFrameTime());

	UpdatePositionInBounds(&ShipPosition, moveVector, ShipSize);

	if (IsKeyDown(KEY_SPACE))
	{
		LastShotAccumulator += GetFrameTime();
		while (LastShotAccumulator > ShotCooldown)
		{
			LastShotAccumulator -= ShotCooldown;
			for (int i = 0; i < MaxBullets; i++)
			{
				if (!Bullets[i].Alive)
				{
					Bullets[i].Alive = true;
					Bullets[i].Position = ShipPosition;
					Bullets[i].Velocity = (Vector2){ 200, 0 };
					break;
				}
			}
		}
	}

	for (int i = 0; i < MaxBullets; i++)
	{
		if (Bullets[i].Alive)
		{
			Vector2 delta = Vector2Scale(Bullets[i].Velocity, GetFrameTime());
			if (UpdatePositionInBounds(&Bullets[i].Position, delta, BulletSize))
			{
				Bullets[i].Alive = false;
			}
		}
	}

	LastSpawnAccumulator += GetFrameTime();
	while (LastSpawnAccumulator > SpawnCooldown)
	{
		LastSpawnAccumulator -= SpawnCooldown;

		for (int i = 0; i < MaxNPCs; i++)
		{
			if (!NPCs[i].Alive)
			{
				NPCs[i].Alive = true;
				NPCs[i].Position = (Vector2){ NominalScreenSize.x * 0.5f - NPCSize * 0.5f, GetRandomValue(-NominalScreenSize.y * 0.5f - NPCSize, NominalScreenSize.y * 0.5f + NPCSize) };
				NPCs[i].Velocity = (Vector2){ -40, 0 };
				break;
			}
		}
	}

	for (int i = 0; i < MaxNPCs; i++)
	{
		if (NPCs[i].Alive)
		{
			Vector2 delta = Vector2Scale(NPCs[i].Velocity, GetFrameTime());
			if (UpdatePositionInBounds(&NPCs[i].Position, delta, ShipSize))
			{
				NPCs[i].Alive = false;
			}
		}
	}
}

void SetupScene()
{

}

int main ()
{
	SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
	InitWindow(1280,800,"Example");

	CheckCameraZoom();

	Ship = LoadTexture("resources/textures/player.png");
	Bullet = LoadTexture("resources/textures/bullet.png");
	NPC = LoadTexture("resources/textures/enemy.png");

	SetupScene();

	while (!WindowShouldClose())
	{
		BeginDrawing();

		if (IsWindowResized())
			CheckCameraZoom();

		UpdateScene();

		ClearBackground((Color){32,32,32,255});
		BeginMode2D(ViewportCamera);

		DrawScene();
		DrawSceneOverlay();
		EndMode2D();

		DrawLetterboxOverlay();
		
		EndDrawing();
	}
	CloseWindow();
	return 0;
}