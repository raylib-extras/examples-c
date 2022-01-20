/*******************************************************************************************
*
*   raylib [Vector Maths] example - Circle In View Cone
*
*   Welcome to raylib!
*
*   You can find all basic examples on C:\raylib\raylib\examples folder or
*   raylib official webpage: www.raylib.com
*
*   Enjoy using raylib. :)
*
*   This example has been created using raylib 4.0 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 20122 Jeffery Myers
*
********************************************************************************************/

#include "raylib.h"
#include "raymath.h"


// Transforms a Vector2 by a given Matrix
// Use this for older raylib
// Vector2 Vector2Transform(Vector2 v, Matrix mat)
// {
// 	Vector2 result = { 0 };
// 
// 	float x = v.x;
// 	float y = v.y;
// 	float z = 0;
// 
// 	result.x = mat.m0 * x + mat.m4 * y + mat.m8 * z + mat.m12;
// 	result.y = mat.m1 * x + mat.m5 * y + mat.m9 * z + mat.m13;
// 
// 	return result;
// }

typedef struct
{
	Vector2 Direction;
	Vector2 Normal;
}ViewRay;

typedef struct 
{
	ViewRay Left;
	ViewRay Right;
	
	Vector2 Forward;
	Vector2 Position;
}View;

// setup a view 
void InitView(View* view, Vector2 position, float startingAngle, float fov)
{
	Matrix mat = MatrixRotateZ(startingAngle * DEG2RAD);
	view->Forward = Vector2Transform((Vector2) { 0, -1 }, mat);
	view->Position = position;

	mat = MatrixRotateZ(startingAngle+fov/2.0f * DEG2RAD);
	view->Left.Direction = Vector2Transform((Vector2) { 0, -1 }, mat);
	view->Left.Normal = Vector2Transform((Vector2) { 1, 0 }, mat);

	mat = MatrixRotateZ(startingAngle - fov / 2.0f * DEG2RAD);
	view->Right.Direction = Vector2Transform((Vector2) { 0, -1 }, mat);
	view->Right.Normal = Vector2Transform((Vector2) { -1, 0 }, mat);
}

// rotate all the vectors in the view
void RotateView(View* view, float angle)
{
	Matrix mat = MatrixRotateZ(angle * DEG2RAD);

	view->Forward = Vector2Transform(view->Forward, mat);
	view->Left.Direction = Vector2Transform(view->Left.Direction, mat);
	view->Left.Normal = Vector2Transform(view->Left.Normal, mat);
	view->Right.Direction = Vector2Transform(view->Right.Direction, mat);
	view->Right.Normal = Vector2Transform(view->Right.Normal, mat);
}

void DrawView(View* view)
{
	DrawCircle(view->Position.x, view->Position.y, 5, LIGHTGRAY);

	//left view
	Vector2 viewExtension = Vector2Add(view->Position, Vector2Scale(view->Left.Direction, 500));
	DrawLineV(view->Position, viewExtension, PURPLE);
	Vector2 normalPoint = Vector2Add(view->Position, Vector2Scale(view->Left.Direction, 200));
	Vector2 normalExtension = Vector2Add(normalPoint, Vector2Scale(view->Left.Normal, 20));
	DrawLineV(normalPoint, normalExtension, PURPLE);

	//right view
	viewExtension = Vector2Add(view->Position, Vector2Scale(view->Right.Direction, 500));
	DrawLineV(view->Position, viewExtension, VIOLET);
	normalPoint = Vector2Add(view->Position, Vector2Scale(view->Right.Direction, 200));
	normalExtension = Vector2Add(normalPoint, Vector2Scale(view->Right.Normal, 20));
	DrawLineV(normalPoint, normalExtension, VIOLET);
}

typedef enum 
{
	Outside = 0,
	Contained = 1,
	Intersecting = 2,
}ViewInteresectionType;

// see if a circle is on the normal side of a ray, intersecting it, or outside it
ViewInteresectionType SphereNearViewRay(Vector2 origin, ViewRay* ray, Vector2 center, float radius)
{
	// get the vector from the origin to the point to test
	Vector2 vecToPoint = Vector2Subtract(center, origin);

	// get the dot product between the ray and the vector to the point
	float dotForPoint = Vector2DotProduct(ray->Direction, vecToPoint);

	// the nearest point on the ray is the dot product projection along the ray vector
	Vector2 nearestPoint = { origin.x + ray->Direction.x * dotForPoint, origin.y + ray->Direction.y * dotForPoint };

	// distance from the center to the nearest point, squared
	float nearestDistSq = Vector2LengthSqr(Vector2Subtract(center, nearestPoint));

	// dot product between the normal and the vector to the point tells us what side of the ray the point is on
	float normDot = Vector2DotProduct(ray->Normal, vecToPoint);

	// the dot product for the point is the distance along the ray to our nearest point
	// if it is less than the negative radius, the point is behind the ray origin and can't be intersecting or inside
	if (dotForPoint > -radius)
	{
		// if the distance from the nearest point to the center is less than a radius, it must be intersecting
		if (nearestDistSq <= radius * radius)
		{
			return Intersecting;
		}
		else
		{
			// if the point is on the inside of the ray, it is fully contained (because we know it's not intersecting)
			if (normDot >= 0)
				return Contained;
		}
	}

	return Outside;
}

// test the circle against both sides of the view
ViewInteresectionType CircleInView(View* view, Vector2 center, float radius)
{
	// get intersection for left and right
	ViewInteresectionType left = SphereNearViewRay(view->Position, &view->Left, center, radius);
	ViewInteresectionType right = SphereNearViewRay(view->Position, &view->Right, center, radius);

	// if it it is outside any edge it is outside the view
	if (left == Outside || right == Outside)
		return Outside;

	// inside both is inside the view
	if (left == Contained && right == Contained)
		return Contained;

	// anything else must be intersecting
	return Intersecting;
}

int main(void)
{
	const int screenWidth = 800;
	const int screenHeight = 600;

	InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");

	SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

	float radius = 50;
	View view;
	InitView(&view, (Vector2){ 400,425 }, 0, 60);

	// Main game loop
	while (!WindowShouldClose())    // Detect window close button or ESC key
	{
		float angle = 0;

		// rotate the view
		if (IsKeyDown(KEY_LEFT))
			angle += 90 * GetFrameTime();
		if (IsKeyDown(KEY_RIGHT))
			angle -= 90 * GetFrameTime();

		// change the radius
		if (IsKeyDown(KEY_UP))
			radius += 50 * GetFrameTime();
		if (IsKeyDown(KEY_DOWN))
			radius -= 25 * GetFrameTime();

		RotateView(&view, angle);

		// get a point to test
		Vector2 point = GetMousePosition();

		BeginDrawing();

		ClearBackground(BLACK);

		DrawView(&view);

		DrawCircle(point.x, point.y, 5, BLUE);

		Color c = RED;

		ViewInteresectionType t = CircleInView(&view, point, radius);

		if (t == Intersecting)
			c = YELLOW;
		else if (t == Contained)
			c = GREEN;

		DrawCircleLines(point.x, point.y, radius, c);

		EndDrawing();
	}
	CloseWindow();
	return 0;
}