/**********************************************************************************************
*
*   raylib-extras, examples-c * examples for Raylib in C
*
*   pathfinding in combination with signed distance fields
*
*   LICENSE: ZLib
*
*   Copyright (c) 2024 Eike Decker
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
**********************************************************************************************
*
*   This example demonstrates how to use signed distance fields (SDF) for improving
*   pathfinding.
*
*   1) Unit size: A path may have a requirement for a minimum width to allow passage.
*      By using the SDF values, we can block paths that are too narrow for the unit.
*
*   2) Path preferences: A unit may prefer to stay close to walls or avoid them.
*      The example demonstrates how to influence the pathfinding by using the SDF values.
*
*   3) Varying step distances: Using SDF values to adjust step distances during path 
*      finding, resulting in curved paths.
*
*  TODO: optimize the path after it was found: nodes could be removed to smooth the path
*        this could again be done using the SDF values to determine if a node can be removed
*        without clipping through walls
*
********************************************************************************************/

#include "raylib.h"
#include "raymath.h"

#include <stddef.h> // Required for: NULL
#include <math.h> // Required for: abs

typedef struct PathfindingNode
{
    int x, y;
    int fromX;
    int fromY;
    int score;
} PathfindingNode;

typedef struct NeighborOffset
{
    int x, y;
    int distance;
} NeighborOffset;

typedef struct Agent
{
    int startX, startY;
    int targetX, targetY;
    int wallFactor;
    int unitSize;
    PathfindingNode *path;
    int pathCount;
    PathfindingNode *map;
    Vector2* icon;
    Color color;
    int iconCount;
    float walkedPathDistance;
} Agent;

typedef struct AppState
{
    int visualizeMode;
    int randomizeBlocks;
    char paintMode;
    int updateSDF;
    int sdfFunction;
    int jumpingEnabled;
    int cellX, cellY;
    Agent rat;
    Agent cat;
} AppState;

// a simple cat face that can be drawn as a triangle fan
Vector2 catFace[] = {
    {0.0f, 1.0f},
    {0.7f, 1.0f},
    {1.0f, 0.7f},
    {1.0f, -1.0f},
    {0.5f, -0.6f},
    {-0.5f, -0.6f},
    {-1.0f, -1.0f},
    {-1.0f, 0.7f},
    {-0.7f, 1.0f},
};

// a simple rat face that can be drawn as a triangle fan
Vector2 ratFace[] = {
    {0.0f, 1.0f},
    {0.3f, 0.9f},
    {0.8f, -0.2f},
    {1.0f, -0.8f},
    {0.8f, -1.0f},
    {0.5f, -1.0f},
    {0.3f, -0.7f},
    {0.0f, -0.8f},
    {-0.3f, -0.7f},
    {-0.5f, -1.0f},
    {-0.8f, -1.0f},
    {-1.0f, -0.8f},
    {-0.8f, -0.2f},
    {-0.3f, 0.9f},
};


const Color gridColor = { 200, 200, 200, 40 };
const Color cellHighlightColor = { 200, 0, 0, 80 };
const float movementSpeed = 3.0f;

const int gridWidth = 80;
const int gridHeight = 45;
const int cellSize = 10;

char* blockedCells = NULL;
char* sdfCells = NULL;

// lookup table for cheap square root calculation
int isqrt[256] = {0};

// various offsets and distances for jumping nodes during pathfinding
NeighborOffset neighborOffsets[20*20] = {0};
int neighborOffsetCount = 0;

Agent Agent_init(int x, int y, int size, int targetX, int targetY, int wallFactor, Vector2* icon, int iconCount, Color color)
{
    Agent agent;
    agent.unitSize = size;
    agent.startX = x;
    agent.startY = y;
    agent.targetX = targetX;
    agent.targetY = targetY;
    agent.pathCount = 0;
    agent.wallFactor = wallFactor;
    agent.path = (PathfindingNode *)MemAlloc(gridWidth * gridHeight * sizeof(PathfindingNode) * 4);
    agent.map = (PathfindingNode *)MemAlloc(gridWidth * gridHeight * sizeof(PathfindingNode));
    agent.icon = icon;
    agent.color = color;
    agent.iconCount = iconCount;
    agent.walkedPathDistance = 0.0f;
    return agent;

}

void Agent_findPath(Agent *agent, int enableJumping)
{
    // the queue should in theory not be longer than the number of map cells
    PathfindingNode *queue = (PathfindingNode *)MemAlloc(gridWidth * gridHeight * sizeof(PathfindingNode));
    PathfindingNode *map = agent->map;
    PathfindingNode *path = agent->path;
    int unitSize = agent->unitSize;
    int sdfFactor = agent->wallFactor;
    // we swap the start and end points to get the path in the right order without reversing it
    // so it searches from the target to the start and not the other way round, but in this case,
    // this doesn't matter
    int toX = agent->startX;
    int toY = agent->startY;
    int startX = agent->targetX;
    int startY = agent->targetY;
    for (int i = 0; i < gridWidth * gridHeight; i++)
    {
        map[i].score = 0;
    }

    // initialize queue and map with start position data
    int queueLength = 1;
    queue[0].fromX = startX;
    queue[0].fromY = startY;
    queue[0].x = startX;
    queue[0].y = startY;
    queue[0].score = 1;
    map[startY * gridWidth + startX].fromX = -1;
    map[startY * gridWidth + startX].fromY = -1;
    map[startY * gridWidth + startX].x = startX;
    map[startY * gridWidth + startX].y = startY;
    map[startY * gridWidth + startX].score = 1;

    while (queueLength > 0)
    {
        // find and dequeue node with lowest score
        int lowestScoreIndex = 0;
        for (int i=1;i<queueLength;i++)
        {
            if (queue[i].score < queue[lowestScoreIndex].score)
            {
                lowestScoreIndex = i;
            }
        }
        PathfindingNode node = queue[lowestScoreIndex];
        for (int i=lowestScoreIndex+1;i<queueLength;i++)
        {
            queue[i-1] = queue[i];
        }
        queueLength--;

        // we can determine how far we can safely jump away from this cell by 
        // taking the SDF value of the current cell. If our unit size is 2 and 
        // the SDF value is 5, we can safely jump 3 cells away from this cell, knowing
        // that we can't clip through walls at this distance.
        int cellSdf = sdfCells[node.y * gridWidth + node.x];
        int maxDistance = cellSdf - agent->unitSize;
        if (maxDistance < 1)
        {
            maxDistance = 1;
        }

        // The neighbor offsets are used to check various directions of different distances
        for (int i=0; i<neighborOffsetCount; i++)
        {
            // step distance is the distance the offset is away from the current cell
            // if it exceeds the max distance, we skip this offset
            int stepDistance = neighborOffsets[i].distance;
            if (stepDistance > maxDistance || (!enableJumping && stepDistance > 1))
            {
                continue;
            }

            // rejecting first cells that are outside the map
            int x = node.x + neighborOffsets[i].x;
            int y = node.y + neighborOffsets[i].y;
            if (x < 0 || x >= gridWidth || y < 0 || y >= gridHeight)
            {
                continue;
            }

            // nextSdf is the SDF value of the next cell where we would land
            int nextSdf = sdfCells[y * gridWidth + x];

            // skip if the next cell is closer to a wall than the unit size (wall clipping)
            if (nextSdf < unitSize)
            {
                continue;
            }

            // calculate the score of the next cell
            int score = node.score + stepDistance;
            int sdfValue = sdfCells[y * gridWidth + x];
            // assuming a linear interpolation between the SDF values of the current and next cell,
            // we can estimate the integral of the SDF values between the two cells - this is
            // only a rough approximation and since it's integers, we cheat a bit to favor longer jumps
            int integratedSdfValue = (sdfValue + cellSdf) * (stepDistance + 1) / 2;
            score = score + integratedSdfValue * sdfFactor / 6;

            // if the cell is not yet visited or the score is lower than the previous score, 
            // we update the cell and queue the cell for evaluation (one optimization would be to
            // not queue the cell if it is already queued, but this complexity is omitted here)
            if (map[y * gridWidth + x].score == 0 || score < map[y * gridWidth + x].score)
            {
                map[y * gridWidth + x] = (PathfindingNode){
                    .fromX = node.x,
                    .fromY = node.y,
                    .x = x,
                    .y = y,
                    .score = score
                };
                // queue the cell for evaluation
                queue[queueLength] = map[y * gridWidth + x];
                queueLength++;

                // prevent queue overflow - should not happen with the chosen queue lengths, but
                // could (maybe) still happen for extreme worst case scenarios
                if (queueLength >= gridWidth * gridHeight)
                {
                    TraceLog(LOG_ERROR, "queue overflow\n");
                    queueLength = gridWidth * gridHeight - 1;
                }
            }
        }
    }

    if (map[toY * gridWidth + toX].score > 0)
    {
        // path found
        int x = toX;
        int y = toY;
        int length = 0;
        // reconstruct path by following the from pointers to previous cells - the list is reversed
        // but we handle this with swapping the start / end points
        while (map[y * gridWidth + x].score > 0 && (x != startX || y != startY) && length < gridWidth * gridHeight)
        {
            path[length] = map[y * gridWidth + x];
            x = path[length].fromX;
            y = path[length].fromY;
            length++;
        }
        path[length++] = map[startY * gridWidth + startX];
        
        agent->pathCount = length;
    }
    else
    {
        // no path found
        agent->pathCount = 0;
    }

    MemFree(queue);
}

void Agent_drawPath(Agent *agent)
{
    int rectSize = agent->unitSize * 2;
    int rectOffset = (cellSize - rectSize) / 2;
    for (int i = 0; i < agent->pathCount; i++)
    {
        DrawRectangle(
            agent->path[i].x*cellSize + rectOffset, 
            agent->path[i].y * cellSize + rectOffset, 
            rectSize, rectSize, agent->color);
    }
    for (int i = 1; i < agent->pathCount; i++)
    {
        DrawLine(agent->path[i-1].x*cellSize + cellSize / 2, agent->path[i-1].y * cellSize + cellSize / 2, agent->path[i].x*cellSize + cellSize / 2, agent->path[i].y * cellSize + cellSize / 2, agent->color);
    }
}

void Agent_drawPathMovement(Agent *agent)
{
    int pathCount = agent->pathCount;
    if (pathCount == 0)
    {
        return;
    }

    PathfindingNode* path = agent->path;
    int pointCount = agent->iconCount;
    Vector2* points = agent->icon;
    float radius = agent->unitSize * cellSize / 2.0f;
    Color color = agent->color;

    float dt = GetFrameTime();
    agent->walkedPathDistance += dt * movementSpeed;
    float pointDistance = 0.0f;
    for (int i=1;i<pathCount;i++)
    {
        PathfindingNode p1 = path[i-1];
        PathfindingNode p2 = path[i];
        float dx = p2.x - p1.x;
        float dy = p2.y - p1.y;
        float d = sqrtf(dx * dx + dy * dy);
        if (pointDistance + d >= agent->walkedPathDistance)
        {
            float t = (agent->walkedPathDistance - pointDistance) / d;
            float x = p1.x + dx * t + 0.5f;
            float y = p1.y + dy * t + 0.5f;
            if (points != NULL && pointCount > 0)
            {
                Vector2 fan[32];
                for (int j=0;j<pointCount;j++)
                {
                    float px = points[j].x * radius + x * cellSize;
                    float py = points[j].y * radius + y * cellSize;
                    fan[j] = (Vector2){ px, py };
                }
                DrawTriangleFan(fan, pointCount, color);
            }
            else
                DrawCircle(x * cellSize, y * cellSize, radius, color);
            
            return;
        }
        pointDistance += d;
    }

    agent->walkedPathDistance = 0.0f;
} 

float CalcPathLength(PathfindingNode* path, int pathCount)
{
    float length = 0.0f;
    for (int i=1;i<pathCount;i++)
    {
        PathfindingNode p1 = path[i-1];
        PathfindingNode p2 = path[i];
        float dx = p2.x - p1.x;
        float dy = p2.y - p1.y;
        length += sqrtf(dx * dx + dy * dy);
    }
    return length;
}

void AppState_handleInput(AppState *appState)
{
    Vector2 mousePos = GetMousePosition();
    int cellX = mousePos.x / cellSize;
    int cellY = mousePos.y / cellSize;
    appState->cellX = cellX;
    appState->cellY = cellY;

    //----------------------------------------------------------------------------------
    // mouse input handling
    //----------------------------------------------------------------------------------
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        appState->paintMode = blockedCells[cellY * gridWidth + cellX] == 1 ? 0 : 1;
    }
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
    {
        blockedCells[cellY * gridWidth + cellX] = appState->paintMode;
        appState->updateSDF = 1;
    }

    //----------------------------------------------------------------------------------
    // keyboard input handling
    //----------------------------------------------------------------------------------
    if (IsKeyDown(KEY_C)) 
    {
        for (int i = 0; i < gridWidth * gridHeight; i++)
        {
            blockedCells[i] = 0;
        }
        appState->updateSDF = 1;
    }

    if (IsKeyPressed(KEY_V))
    {
        appState->visualizeMode++;
    }

    if (IsKeyPressed(KEY_Q))
    {
        appState->rat.wallFactor = (appState->rat.wallFactor + 1) % 8;
        appState->updateSDF = 1;
    }

    if (IsKeyPressed(KEY_R))
    {
        appState->randomizeBlocks = 1;
    }

    if (IsKeyPressed(KEY_S))
    {
        appState->sdfFunction = (appState->sdfFunction + 1) % 3;
        appState->updateSDF = 1;
    }

    if (IsKeyPressed(KEY_J))
    {
        appState->jumpingEnabled = !appState->jumpingEnabled;
        appState->updateSDF = 1;
    }
}

void AppState_randomizeBlocks(AppState *appState)
{
    appState->updateSDF = 1;
    for (int i = 0; i < gridWidth * gridHeight; i++)
    {
        blockedCells[i] = 0;
    }
    for (int i=0;i<40;i++)
    {
        int x = GetRandomValue(15, gridWidth-15);
        int y = GetRandomValue(15, gridHeight-15);
        int size = GetRandomValue(1, 2);
        int blockValue = GetRandomValue(0, 1);
        for (int j=-size;j<=size;j++)
        {
            for (int k=-size;k<=size;k++)
            {
                blockedCells[(y+j)*gridWidth + x+k] = blockValue;
            }
        }
    }
}

int clamp(int value, int min, int max)
{
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

void AppState_updateSDF(AppState *appState)
{
    // calculate sdf values
    // update sdf values to max distance we want to consider
    for (int i = 0; i < gridWidth * gridHeight; i++)
    {
        sdfCells[i] = 10;
    }
    
    int sdfFunction = appState->sdfFunction;

    for (int y = 0; y < gridHeight; y++)
    {
        for (int x = 0; x < gridWidth; x++)
        {
            // for each cell, we update the surrounding cells with the distance to this wall
            // doing this brutally simple, for big maps this is inefficient
            if (blockedCells[y * gridWidth + x] == 1)
            {
                sdfCells[y * gridWidth + x] = 0;
                int minX = clamp(x - 10, 0, gridWidth - 1);
                int minY = clamp(y - 10, 0, gridHeight - 1);
                int maxX = clamp(x + 10, 0, gridWidth - 1);
                int maxY = clamp(y + 10, 0, gridHeight - 1);
                // update surrounding cells up to max distance we want to consider
                for (int j = minY; j <= maxY; j++)
                {
                    for (int i = minX; i <= maxX; i++)
                    {
                        int dx = x - i;
                        int dy = y - j;
                        int d = 0;
                        if (sdfFunction == 0)
                        {
                            // euclidean distance
                            d = isqrt[dx * dx + dy * dy];
                        }
                        else if (sdfFunction == 1)
                        {
                            // chebyshev distance
                            d = (abs(dx) < abs(dy)) ? abs(dy) : abs(dx);
                        }
                        else if (sdfFunction == 2)
                        {
                            // manhattan distance
                            d = abs(dx) + abs(dy);
                        }

                        if (d < sdfCells[j*gridWidth + i] && d < 10)
                        {
                            sdfCells[j*gridWidth + i] = d;
                        }
                    }
                }
            }
        }
    }

    // trigger path finding for both agents
    Agent_findPath(&appState->rat, appState->jumpingEnabled);
    Agent_findPath(&appState->cat, appState->jumpingEnabled);
}

void DrawMapContent()
{
    for (int y = 0; y < gridHeight; y++)
    {
        for (int x = 0; x < gridWidth; x++)
        {
            // blocked cells black
            if (blockedCells[y * gridWidth + x] == 1) 
            {
                DrawRectangle(x * cellSize, y * cellSize, cellSize, cellSize, BLACK);
            }

            // sdf values as transparent blue (the further away from wals, the brighter)
            int sdf = sdfCells[y * gridWidth + x];
            DrawRectangle(x*cellSize, y * cellSize, cellSize, cellSize, (Color){ 32, 32, 32, 230-sdf * 20});
        }
    }
}

void DrawPathMapVisualization(PathfindingNode* pathToDraw)
{
    int scoreMax = 0;
    for (int i = 0; i<gridWidth*gridHeight; i++)
    {
        if (pathToDraw[i].score > scoreMax)
        {
            scoreMax = pathToDraw[i].score;
        }
    }
    for (int y = 0; y < gridHeight; y++)
    {
        for (int x = 0; x < gridWidth; x++)
        {
            if (pathToDraw[y * gridWidth + x].score > 0)
            {
                int score = pathToDraw[y * gridWidth + x].score;
                int c = score % 64 * 4;
                DrawRectangle(x*cellSize, y * cellSize, cellSize, cellSize, (Color){ c, c, 0, 128 });
            }
        }
    }
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    SetTraceLogLevel(LOG_ALL);
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [sdf pathfinding] example");

    SetTargetFPS(60);

    // initialize square root lookup table for cheap square root calculation
    for (int i=0;i<256;i++)
    {
        isqrt[i] = (int)ceilf(sqrtf(i));
    }

    for (int x = -10; x <= 10; x++)
    {
        for (int y = -10; y <= 10; y++)
        {
            int d = isqrt[x * x + y * y];
            if (d <= 10 && d > 0)
            {
                neighborOffsets[neighborOffsetCount] = (NeighborOffset){ x, y, d };
                neighborOffsetCount++;
            }
        }
    }

    blockedCells = (char *)MemAlloc(gridWidth * gridHeight * sizeof(char));
    sdfCells = (char *)MemAlloc(gridWidth * gridHeight * sizeof(char));

    AppState appState = {
        .visualizeMode = 0,
        .randomizeBlocks = 1,
        .paintMode = 0,
        .updateSDF = 1,
        .sdfFunction = 0,
        .jumpingEnabled = 1,
        .rat = Agent_init(5, 25, 1, 75, 25, 2, ratFace, sizeof(ratFace) / sizeof(ratFace[0]), RED),
        .cat = Agent_init(5, 25, 2, 75, 25, 0, catFace, sizeof(catFace) / sizeof(catFace[0]), BLUE),
    };
    
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        //----------------------------------------------------------------------------------
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground((Color) {170,200,150,255});

            AppState_handleInput(&appState);

            //----------------------------------------------------------------------------------
            // initialize map with random blocks
            //----------------------------------------------------------------------------------
            if (appState.randomizeBlocks)
            {
                appState.randomizeBlocks = 0;
                AppState_randomizeBlocks(&appState);
            }

            //----------------------------------------------------------------------------------
            // update sdf values and execute pathfinding
            //----------------------------------------------------------------------------------
            if (appState.updateSDF)
            {
                appState.updateSDF = 0;
                AppState_updateSDF(&appState);
            }

            //----------------------------------------------------------------------------------
            // draw cell content of walls and sdf values
            //----------------------------------------------------------------------------------
            DrawMapContent();

            //----------------------------------------------------------------------------------
            // draw rat pathfinding score data for visualization
            //----------------------------------------------------------------------------------
            PathfindingNode* pathToDraw = NULL;
            switch (appState.visualizeMode % 3)
            {
                case 1: // visualize rat map
                    pathToDraw = appState.rat.map;
                    break;
                case 2: // visualize cat map
                    pathToDraw = appState.cat.map;
                    break;
            }

            if (pathToDraw != NULL)
            {
                DrawPathMapVisualization(pathToDraw);
            }

            //----------------------------------------------------------------------------------
            // draw grid lines
            //----------------------------------------------------------------------------------
            for (int y = 0; y < gridHeight; y++)
            {
                DrawRectangle(0, y * cellSize, gridWidth*cellSize, 1, gridColor);
            }
            for (int x = 0; x < gridWidth; x++)
            {
                DrawRectangle(x*cellSize, 0, 1, gridHeight*cellSize, gridColor);
            }

            // highlight current cell the mouse is over
            DrawRectangle(appState.cellX * cellSize, appState.cellY * cellSize, cellSize, cellSize, cellHighlightColor);


            //----------------------------------------------------------------------------------
            // draw paths of cat and rat
            //----------------------------------------------------------------------------------
            Agent_drawPath(&appState.rat);
            Agent_drawPath(&appState.cat);

            //----------------------------------------------------------------------------------
            // draw animated movement of rat and cat
            //----------------------------------------------------------------------------------
            Agent_drawPathMovement(&appState.rat);
            Agent_drawPathMovement(&appState.cat);
            
            //----------------------------------------------------------------------------------
            // description and status
            //----------------------------------------------------------------------------------
            DrawText("Left click to toggle blocked cells, C: clear, Left mouse: toggle cell", 10, 10, 20, BLACK);
            DrawText("The red rat is small and likes to run close to walls", 10, 30, 20, RED);
            DrawText("The blue cat is big and can't fit through narrow paths and\nprefers the short path", 10, 50, 20, BLUE);
            DrawText(TextFormat("Rat path length: %.2f, Cat path length: %.2f", 
                CalcPathLength(appState.rat.path, appState.rat.pathCount), 
                CalcPathLength(appState.cat.path, appState.rat.pathCount)), 
                10, GetScreenHeight() - 100, 20, BLACK);
            DrawText(TextFormat("R: randomize blocks, J: jumping enabled (current: %s)", appState.jumpingEnabled ? "yes" : "no"), 10, GetScreenHeight() - 80, 20, BLACK);
            DrawText(TextFormat("S: switch SDF function (current: %s)", appState.sdfFunction == 0 ? "euclidean" : (appState.sdfFunction == 1 ? "chebyshev" : "manhattan")), 10, GetScreenHeight() - 60, 20, BLACK);
            DrawText(TextFormat("Q: Rat wall factor (how much the rat wants to stay close to walls): %d", appState.rat.wallFactor), 10, GetScreenHeight() - 40, 20, BLACK);
            DrawText(TextFormat("V: switch visualization mode (current: %s)", appState.visualizeMode % 3 == 0 ? "none" : (appState.visualizeMode % 3 == 1 ? "map rat" : "map cat")), 10, GetScreenHeight() - 20, 20, BLACK);
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();
    //--------------------------------------------------------------------------------------

    return 0;
}

