/**********************************************************************************************
*
*   raylib-extras, examples-c * examples for Raylib in C
*
*   basic platformer
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

// checks a moving rectangle against some static object, stopping them motion based on what side of the static object is hit.
// hit booleans return back what part of the object was hit to help with state collisons
void CollideRectWithObject(const Rectangle mover, const Rectangle object, Vector2* motion, bool* hitSide, bool* hitTop, bool* hitBottom)
{
	if (!motion)
		return;

	// check the X axis
	float oldRight = mover.x + mover.width;
	float oldBottom = mover.y + mover.height;

	Rectangle newrect = mover;
	newrect.x += motion->x;
	newrect.y += motion->y;

	if (!CheckCollisionRecs(newrect, object))
		return;

	bool canHitX = true;
	if (newrect.y > object.y + object.height)	// our top is below wall bottom
		canHitX = false;
	else if (newrect.y + newrect.height < object.y) // our bottom is over the wall top
		canHitX = false;

	if (canHitX)
	{
		float newRight = newrect.x + newrect.width;
		float objectRight = object.x + object.width;

		// check the box moving to the right
		// if we were outside the left wall before, and are not now, we hit something
		if (motion->x > 0)
		{
			if (oldRight <= object.x)
			{
				if (newRight > object.x)
				{
					// we hit moving right, so set us back to where we hit the wall
					newrect.x = object.x - mover.width;
					if (hitSide)
						*hitSide = true;
				}
			}		
		}
		
		if (motion->x < 0)
		{
			// check the box moving to the left
			// if we were outside the right wall before, and are not now, we hit something
			if (mover.x >= objectRight)
			{
				if (newrect.x < objectRight)
				{
					// we hit moving left, so set us back to where we hit the wall
					newrect.x = objectRight;
					if (hitSide)
						*hitSide = true;
				}
			}
		}
	}

	// do the same for Y
	bool canHitY = true;
	if (newrect.x > object.x + object.width)		// our left is past wall right
		canHitY = false;
	else if (newrect.x + newrect.width < object.x)	// our right is past wall left
		canHitY = false;

	if (canHitY)
	{
		float newBottom = newrect.y + newrect.height;
		float objectBottom = object.y + object.height;

		// check the box moving to the down
		// if we were outside the top wall before, and are not now, we hit something
		if (motion->y >= 0)
		{
			if (oldBottom <= object.y)
			{
				if (newBottom > object.y)
				{
					// we hit moving down, so set us back to where we hit the wall
					newrect.y = object.y - mover.height;
					if (hitBottom)
						*hitBottom = true;
				}
				else if (newBottom == object.y)
				{
					if (hitBottom)
						*hitBottom = true;
				}
			}
		}
		else if (motion->y < 0)
		{
			// check the box moving up
			// if we were outside the bottom wall before, and are not now, we hit something
			if (mover.y >= objectBottom)
			{
				if (newrect.y < objectBottom)
				{
					// we hit moving up, so set us back to where we hit the wall
					newrect.y = objectBottom;
					if (hitTop)
						*hitTop = true;
				}
			}
 		}
	}

	motion->x = newrect.x - mover.x;
	motion->y = newrect.y - mover.y;
}

// function to return a fixed timestep when debugging
float GetDeltaTime()
{
#ifdef _DEBUG
	return 1 / 60.0f;
#else
	return GetFrameTime();
#endif
}


#define MAX_WALLS 7

// main entry point
int main(void)
{
	SetConfigFlags(FLAG_VSYNC_HINT);
	InitWindow(1280, 600, "Support Example");
	SetTargetFPS(60);

	// set up some walls and floors
	Rectangle walls[MAX_WALLS];

	walls[0] = (Rectangle){ 600,200,100,150 };
	walls[1] = (Rectangle){ 150,350,1000,50 };
	walls[2] = (Rectangle){ 0,575,1300,50 };
	walls[3] = (Rectangle){ 0,0,25,600 };
	walls[4] = (Rectangle){ 1255,0,25,600 };
	walls[5] = (Rectangle){ 0,200,400,25 };
	walls[6] = (Rectangle){ 880,200,400,25 };
	
	// set up a player
	Rectangle player = { 300,300, 20,50 };

	// state data for our player
	bool hitSide = false;
	bool hitBottom = false;
	bool hitTop = false;

	bool falling = false;

	Vector2 movement = { 0,0 };

	// game loop
	while (!WindowShouldClose())
	{
		// movement
		float speed = GetDeltaTime() * 300;
		float gravity = GetDeltaTime() * 16;
		float jump = GetDeltaTime() * -600;

		// if we are not falling we can move and jump
		if (!falling)
		{
			movement.x = 0;
			if (IsKeyDown(KEY_A))
				movement.x -= speed;
			if (IsKeyDown(KEY_D))
				movement.x += speed;

			if (IsKeyPressed(KEY_SPACE))
				movement.y += jump;
		}
		else
		{
			// if we are falling we can't jump but we can move a little bit to cheat ledge grabs
			speed *= 0.01f;
			if (IsKeyDown(KEY_A))
				movement.x -= speed;
			if (IsKeyDown(KEY_D))
				movement.x += speed;
		}

		// if we didn't land on something, we need to be pulled down
		if (!hitBottom)
			movement.y += gravity;

		// check collision
		hitSide = false;
		hitBottom = false;
		hitTop = false;
	
		// check all the walls, letting each one have a chance to modify the possible motion
		for (int i = 0; i < MAX_WALLS; i++)
			CollideRectWithObject(player, walls[i], &movement, &hitSide, &hitTop, &hitBottom);

		player.x += movement.x;
		player.y += movement.y;

		// if we are not on a thing and moving down, we are falling
		falling = !hitBottom && movement.y != 0;

		// draw the scene
		BeginDrawing();
		ClearBackground(LIGHTGRAY);
		
		// draw a simple grid
		for (int x = 0; x < 1300; x += 25)
		{
			DrawLine(x, 0, x, 1300, GRAY);
			DrawLine(0, x, 1300, x, GRAY);
		}

		// draw all the walls and floors
		for (int i = 0; i < MAX_WALLS; i++)
			DrawRectangleRec(walls[i], RED);

		// draw the player in a different color depending on what state they are in
		Color playerColor = BLUE;

		if (hitSide)
			playerColor = PURPLE;

		if (falling)
			playerColor = YELLOW;
		
		if (hitTop)
			playerColor = SKYBLUE;

		DrawRectangleRec(player, playerColor);

		EndDrawing();
	}

	CloseWindow();
	return 0;
}