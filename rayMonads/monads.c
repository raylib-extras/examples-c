/*******************************************************************************************
*
*   raylib [core] example - Monads
*
*   Example originally created with raylib 5.5, last time updated with raylib 5.5
*
*   Example contributed by James R. (@<H-Art-Src>)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025 James R, (@<H-Art-Src>)
*
********************************************************************************************/
/*
Infinite depth of monads (objects) with variable connections (functors) between them at any depth.

Use the mouse wheel to change the depth.

Click any object/connection to select it.

-If you're selecting an object currently hovering over the current depth, right clicking will add objects to it.
-If you're selecting an object currently at the current depth, right clicking another object at the same depth will create a one-way connection travelling to the right-clicked object.
-It will instead create a one way connection from the right-clicked object if it is in a different category (container object).
-You can change a link's travelling to object if you are selecting it and its containing object (usually will also be selected by clicking the link) by right clicking an object at the same depth.

-Key 'B' to delete all connections from and to a selected object.
-Key 'Delete' to delete a selected object and recursively delete all objects contained within that object (and so on) and their connections from and to.
If you are selecting a link it will delete that instead of an object.
-Key 'V' will rename the selected object to your clipboard contents.
*/

#include "raylib.h"
#include "raymath.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// This enum acts as a countdown to make sure links of an object are deleted in exactly two frames for object deletion and link breaking.
enum
{
    DELETE_OFF,
    DELETE_POSTONLYLINK,
    DELETE_ONLYLINK,
    DELETE_PRELINK,
    DELETE_FINAL
};

// 1. A Monad cannot have multiple container Monads.
// 2. rootSubLink can only have starting Monads that exist within rootSubMonads.
// 3. Link cannot comprise of Monads of different depths.
// 4. Only one combination of a link can exist in totality.
#define MAX_MONAD_NAME_SIZE 32
#define MONAD_LINK_MIDDLE_LERP 0.35f
typedef struct Monad
{
    char name[MAX_MONAD_NAME_SIZE];
    Vector2 avgCenter , defaultCenter;
    struct Monad *rootSubMonads;
    struct Monad *prev;
    struct Monad *next;
    struct Link *rootSubLink;
    float radius;
    int depth;
    int deleteFrame;
}  Monad;

typedef struct Link
{
    struct Monad *startMonad;
    struct Monad *endMonad;
    struct Link *prev;
    struct Link *next;
} Link;

enum Response
{
    RESULT_NONE,
    RESULT_CLICK,
    RESULT_RCLICK
};

// After returning recursively up the chain, certain results can overide other results depending on the situation.
typedef struct ActiveResult
{
   struct Monad *resultMonad;
   struct Monad *resultContainerMonad;
   struct Link *resultLink;
   int resultKey , resultDepth;
} ActiveResult;

// Adds an object (subMonad) to ContainingMonadPtr. ContainingMonadPtr must not be null.
struct Monad *AddMonad(Vector2 canvasPosition , Monad *containingMonadPtr)
{
    if (Vector2Distance(canvasPosition , containingMonadPtr->avgCenter) <= 30.0f) //deny if too close to container.
        return NULL;
     
    //malloc and initialize new Monad. Always initialize variables that are not being overwritten.
    Monad *newMonadPtr = (Monad*) malloc(sizeof(Monad));
    newMonadPtr->defaultCenter = canvasPosition;
    newMonadPtr->avgCenter = canvasPosition;
    newMonadPtr->rootSubMonads = NULL;
    newMonadPtr->rootSubLink = NULL;
    newMonadPtr->radius = 10.0f;
    newMonadPtr->depth = containingMonadPtr->depth + 1;
    newMonadPtr->deleteFrame = DELETE_OFF;
    
    newMonadPtr->name[0] = (containingMonadPtr->rootSubMonads)? containingMonadPtr->rootSubMonads->prev->name[0] + 1 : 'A';
    newMonadPtr->name[1] = 0;
    
    //insert new Monad in list entry.
    Monad *rootPtr = containingMonadPtr->rootSubMonads;
    if (rootPtr) //has entries.
    {
        Monad *rootNextPtrUnchanged = rootPtr->next;
        Monad *rootPrevPtrUnchanged = rootPtr->prev;
        if ( (rootPtr == rootNextPtrUnchanged) || (rootPtr == rootPrevPtrUnchanged) ) //after one entry
        {
            newMonadPtr->next = rootPtr;
            newMonadPtr->prev = rootPtr;
            rootPtr->next = newMonadPtr;
            rootPtr->prev = newMonadPtr;
        }
        else //after two or more entries.
        {
            newMonadPtr->next = rootPtr;
            newMonadPtr->prev = rootPrevPtrUnchanged;
            rootPtr->prev = newMonadPtr;
            rootPrevPtrUnchanged->next = newMonadPtr;
        }
    }
    else //after zero entries
    {
        containingMonadPtr->rootSubMonads = rootPtr = newMonadPtr;
        rootPtr->next = newMonadPtr;
        rootPtr->prev = newMonadPtr;
    }
    
    //containing Monad data
    float prospectDistance = Vector2Distance(containingMonadPtr->avgCenter , canvasPosition)*1.5f;
    if (prospectDistance > containingMonadPtr->radius)
    {
        containingMonadPtr->radius = prospectDistance;
    }
    containingMonadPtr->avgCenter = Vector2Scale(Vector2Add(containingMonadPtr->avgCenter , canvasPosition) , 0.5f);
    
    return newMonadPtr;
}

// Recursively frees the object and its links after calling the function for its sub-objects.
void  RemoveSubMonadsRecursive(Monad *MonadPtr)
{
    Monad *rootMonad = MonadPtr->rootSubMonads;
    if (rootMonad)
    {
        Monad *iterator = rootMonad;
        do
        {
            Monad *nextMonad = iterator->next;
            RemoveSubMonadsRecursive(iterator);
            iterator = nextMonad;
        } while (iterator != rootMonad);
    }
    
    Link *rootLink = MonadPtr->rootSubLink;
    if (rootLink)
    {
        Link *iterator = rootLink;
        do
        {
            Link *nextLink = iterator->next;
            free(iterator);
            iterator = nextLink;
        } while (iterator != rootLink);
    }

    free(MonadPtr);
}

// Remove an object (subMonad) from containingMonadPtr. containingMonadPtr must not be null.
bool RemoveMonad(Monad *MonadPtr , Monad *containingMonadPtr)
{
    Monad *rootMonad = containingMonadPtr->rootSubMonads;
    if (rootMonad)
    {
        Monad *iterator = rootMonad;
        do
        {
            if (iterator == MonadPtr)
            {
                if (rootMonad == rootMonad->next) //is root and sole sub Monad.
                    containingMonadPtr->rootSubMonads = NULL;
                else if (rootMonad == iterator) //is root and NOT sole sub Monad.
                    containingMonadPtr->rootSubMonads = rootMonad->next;
                iterator->next->prev = iterator->prev;
                iterator->prev->next = iterator->next;
                RemoveSubMonadsRecursive(iterator);
                return true;
            }
            iterator = iterator->next;
        } while (iterator != rootMonad);
    }
    return false;
}

// Checks if two Monads are of the same category.
bool SameCategory(Monad *MonadPtr , Monad *MonadMatePtr)
{
    Monad *iterator = MonadMatePtr;
    if (iterator)
    {
        do
        {
            if (iterator == MonadPtr)
                return true;
            iterator = iterator->next;
        } while (iterator != MonadMatePtr);
    }
    return false;
}

// Add a link to containingMonadPtr. start must be an object contained in the containingMonadPtr. All parameters must not be null.
struct Link *AddLink(Monad *start ,  Monad *end , Monad *containingMonadPtr)
{
    Link *rootPtr = containingMonadPtr->rootSubLink;

    //Return nothing if it already exists
    if (rootPtr) //has entries.
    {
        Link *iterator = rootPtr;
        do
        {
            if ( (iterator->startMonad == start) && (iterator->endMonad == end) )
            {
                printf("Link already exists.\n");
                return NULL;
            }
            iterator = iterator->next;
        } while (iterator != rootPtr);
    }
    //malloc and initialize new Link. Always initialize variables that are not being overwritten.
    Link *newLinkPtr = (Link*) malloc(sizeof(Link));
    newLinkPtr->startMonad = start;
    newLinkPtr->endMonad = end;
        
    //insert new Link in list entry.
    if (rootPtr) //has entries.
    {
        Link *rootNextPtrUnchanged = rootPtr->next;
        Link *rootPrevPtrUnchanged = rootPtr->prev;
        if ( (rootPtr == rootNextPtrUnchanged) || (rootPtr == rootPrevPtrUnchanged) ) //after one entry
        {
            newLinkPtr->next = rootPtr;
            newLinkPtr->prev = rootPtr;
            rootPtr->next = newLinkPtr;
            rootPtr->prev = newLinkPtr;
        }
        else //after two or more entries.
        {
            newLinkPtr->next = rootPtr;
            newLinkPtr->prev = rootPrevPtrUnchanged;
            rootPtr->prev = newLinkPtr;
            rootPrevPtrUnchanged->next = newLinkPtr;
        }
    }
    else //after zero entries
    {
        containingMonadPtr->rootSubLink = rootPtr = newLinkPtr;
        rootPtr->next = newLinkPtr;
        rootPtr->prev = newLinkPtr;
    }    
    return newLinkPtr;
}

// Remove a link from containingMonadPtr. containingMonadPtr must not be null.
bool RemoveLink(Link *linkPtr , Monad *containingMonadPtr)
{
    Link *rootLink = containingMonadPtr->rootSubLink;
    if (rootLink)
    {
        Link *iterator = rootLink;
        do
        {
            if (iterator == linkPtr)
            {
                if (rootLink == rootLink->next) //is root and sole sub Link.
                    containingMonadPtr->rootSubLink = NULL;
                else if (rootLink == iterator) //is root and NOT sole sub Link.
                    containingMonadPtr->rootSubLink = rootLink->next;
                iterator->next->prev = iterator->prev;
                iterator->prev->next = iterator->next;
                free(iterator);
                return true;
            }
            iterator = iterator->next;
        } while (iterator != rootLink);
    }
    return false;
}

#define OUTSCOPED functionDepth > selectedDepth + 1
#define SUBSCOPE functionDepth == selectedDepth + 1
#define INSCOPE functionDepth == selectedDepth
#define PRESCOPE functionDepth < selectedDepth
// Renders all Monads and Link. Returns activated Monad, it's container, if any and the depth. MonadPtr must not be null.
//TODO: Every recursive call adds all of the instructions of the function to RAM again. Which parts of this function can be separated into its own function so they don't get loaded in every time?
//Or maybe... the compiler catches it already.
struct ActiveResult RecursiveDraw(Monad *MonadPtr , int functionDepth , int selectedDepth)
{
    //check collision with mouse, generate first part of activeResult.
    ActiveResult activeResult = (ActiveResult){0};
    activeResult.resultKey = (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))? RESULT_CLICK : ((IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))? RESULT_RCLICK : RESULT_NONE);
    activeResult.resultDepth = functionDepth;
    if ( (functionDepth >= selectedDepth) && CheckCollisionPointCircle( GetMousePosition() , MonadPtr->avgCenter , 30.0f))
    {
        activeResult.resultMonad = MonadPtr;
    }

     //iterate through the functors in the category.
    Link *rootLinkPtr = MonadPtr->rootSubLink;
    if (rootLinkPtr)
    {
        Link *iterator = rootLinkPtr;
        do
        {
            if (INSCOPE)
            {
                bool linkHit = false;
                if (iterator->startMonad == iterator->endMonad)
                {
                    linkHit = CheckCollisionPointCircle( GetMousePosition() , Vector2Add(iterator->startMonad->avgCenter , (Vector2){15.0f,15.0f}) , 30.0f);
                    DrawRectangleV(iterator->startMonad->avgCenter , (Vector2){10.0f,10.0f}, (linkHit)? RED : BLACK);
                }
                else
                {
                    Vector2 midPoint = Vector2Lerp(iterator->startMonad->avgCenter , iterator->endMonad->avgCenter , MONAD_LINK_MIDDLE_LERP);
                    linkHit = CheckCollisionPointCircle( GetMousePosition() , midPoint , 30.0f );
                    DrawLineBezier(iterator->startMonad->avgCenter , midPoint ,  2.0f , (linkHit)? PURPLE : BLUE);
                    DrawLineBezier(midPoint , iterator->endMonad->avgCenter ,  1.0f ,  (SameCategory( iterator->endMonad , iterator->startMonad ))? BLACK : RED);
                }
                if (linkHit)
                {
                    activeResult.resultLink = iterator;
                    activeResult.resultMonad = MonadPtr;
                }
            }
            
            Link *nextSaved = iterator->next;
            if ( (iterator->startMonad->deleteFrame >= DELETE_POSTONLYLINK) || (iterator->endMonad->deleteFrame >= DELETE_POSTONLYLINK) )
            {
                if ( RemoveLink(iterator , MonadPtr) && !(rootLinkPtr = MonadPtr->rootSubLink) )
                {
                    break;
                }
            }
            iterator = nextSaved;

        } while (iterator != rootLinkPtr);
    }
 
    //iterate through the objects with this object treated as a category.
    Monad *rootMonadPtr = MonadPtr->rootSubMonads;
    if (rootMonadPtr)
    {
        Monad *iterator = rootMonadPtr;
        do
        {
            Monad *next = iterator->next;
            
            if (iterator->deleteFrame >= DELETE_FINAL)
            {
                if ( RemoveMonad(iterator ,  MonadPtr) && !(rootMonadPtr = MonadPtr->rootSubMonads) )
                {    
                    break;
                }
                else
                    DrawLineV(MonadPtr->avgCenter , iterator->avgCenter , RED); // something went wrong if still shows.
                iterator = next;
                continue;
            }
            else if (INSCOPE)
            {
                DrawLineV(MonadPtr->avgCenter , iterator->avgCenter , (iterator == rootMonadPtr)? VIOLET : GREEN);
                DrawLineV(next->avgCenter , iterator->avgCenter , Fade((iterator == rootMonadPtr->prev)? ORANGE : YELLOW , 0.5f));
            }

            //--------------------------------
            ActiveResult activeOveride = RecursiveDraw(iterator , functionDepth + 1 , selectedDepth);
            //--------------------------------

            if (activeOveride.resultMonad && !activeResult.resultLink)
            {
                activeResult = activeOveride;
            }
            else if (activeOveride.resultLink)
            {
                activeResult.resultLink = activeOveride.resultLink;
                activeResult.resultMonad = MonadPtr;
            }

            iterator = next;
        } while (iterator != rootMonadPtr);
    }
  
    //mark for deletion progression
    if (MonadPtr->deleteFrame >= DELETE_PRELINK)
    {
            MonadPtr->deleteFrame++;
            return (ActiveResult){0};
    }
    else if (MonadPtr->deleteFrame >= DELETE_POSTONLYLINK)
    {
        MonadPtr->deleteFrame--;
    }

   //cancel any more drawing.
    if (OUTSCOPED)
        return (ActiveResult){0};

    //we have returned back to the container, since this is null, we know that this is the container.
    if ( !activeResult.resultContainerMonad && (activeResult.resultMonad != MonadPtr) )
        activeResult.resultContainerMonad = MonadPtr;

    if (INSCOPE)
    {
        //DrawCircleLinesV(MonadPtr->avgCenter , MonadPtr->radius , GREEN);
        DrawPoly(MonadPtr->avgCenter, 3 ,  5.0f ,  0 , PURPLE);
        DrawText(MonadPtr->name , MonadPtr->avgCenter.x + 10 , MonadPtr->avgCenter.y + 10 , 24 , Fade(PURPLE , 0.5f));
    }
    else if (PRESCOPE)
    {
        DrawCircleLinesV(MonadPtr->avgCenter , MonadPtr->radius , Fade(GRAY , (float){functionDepth}/(float){selectedDepth}) );
    }
    else if (SUBSCOPE)
    {     
        DrawCircleV(MonadPtr->avgCenter , 5.0f , BLUE);
        DrawText(MonadPtr->name , MonadPtr->avgCenter.x + 10 , MonadPtr->avgCenter.y + 10 , 16 , Fade(SKYBLUE , 0.5f));
    }
    if (activeResult.resultMonad == MonadPtr)
        DrawCircleLinesV(MonadPtr->avgCenter , 20.0f , ORANGE); 

    return activeResult;
}

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 800;
    InitWindow(screenWidth, screenHeight, "Monad");
    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Variables
    //--------------------------------------------------------------------------------------
    Monad GodMonad = (Monad){0};
    GodMonad.avgCenter.x = (float){screenWidth}/2.0f;
    GodMonad.avgCenter.y = (float){screenHeight}/2.0f;
    GodMonad.defaultCenter = GodMonad.avgCenter;
    GodMonad.prev = &GodMonad;
    GodMonad.next = &GodMonad;
    strcpy(GodMonad.name , "Monad 0");
    
    char monadLog[MAX_MONAD_NAME_SIZE*3] = "Session started.";
    Monad *selectedMonad = NULL;
    Link *selectedLink = NULL;
    int selectedDepth = 0;
    ActiveResult mainResult =  (ActiveResult){0};
    //--------------------------------------------------------------------------------------

    // Testing
    //--------------------------------------------------------------------------------------    
    AddMonad((Vector2){600,500}, &GodMonad);
    AddMonad((Vector2){200,400}, &GodMonad);
    AddMonad((Vector2){100,100} , AddMonad((Vector2){350,200}, &GodMonad));

    Monad *example =  AddMonad((Vector2){400,400}, &GodMonad);
    AddMonad((Vector2){440,410}, example);
    AddMonad((Vector2){400,450}, example);
    AddMonad((Vector2){500,500}, example);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        if (selectedMonad)
        { 
            if (selectedLink && IsKeyPressed(KEY_DELETE))
            {
                strcpy(monadLog , "Link [");
                strcat(monadLog , selectedLink->startMonad->name);
                strcat(monadLog , "] to [");
                strcat(monadLog , selectedLink->endMonad->name);
                if (RemoveLink(selectedLink , selectedMonad))
                    {
                        selectedLink = NULL;
                        strcat(monadLog , "] deleted.");
                    }
                else
                    strcat(monadLog , "] failed to delete.");
            }
            else if (IsKeyPressed(KEY_DELETE))
            {
                if (selectedMonad == &GodMonad)
                {
                    strcpy(monadLog , "Cannot delete Monad 0.");
                }
                else
                {
                    strcpy(monadLog , "Deleted object [");
                    strcat(monadLog , selectedMonad->name);
                    strcat(monadLog , "].");
                    selectedMonad->deleteFrame = DELETE_PRELINK;
                    selectedMonad = NULL;
                }
            }
           else if (IsKeyPressed(KEY_B))
            {
                strcpy(monadLog , "Broke all links from and to [");
                strcat(monadLog , selectedMonad->name);
                strcat(monadLog , "].");
                selectedMonad->deleteFrame = DELETE_ONLYLINK;
            }
           else if (IsKeyPressed(KEY_V))
            {
                strcpy(monadLog , "Renamed [");
                strcat(monadLog , selectedMonad->name);
                strcat(monadLog , "] to [");
                strncpy(selectedMonad->name , GetClipboardText() , MAX_MONAD_NAME_SIZE);
                selectedMonad->name[MAX_MONAD_NAME_SIZE - 1] = '\0'; //ensures NULL termination.
                strcat(monadLog , selectedMonad->name);
                strcat(monadLog , "].");
            }
        }
        
        BeginDrawing();
            ClearBackground(RAYWHITE);

            mainResult = RecursiveDraw(&GodMonad  , 0 , selectedDepth);

            DrawText( monadLog , 48 , 8 , 20 , GRAY);

            if (selectedMonad)
            {
                int determineMode = selectedMonad->depth - selectedDepth;
                DrawText(( !determineMode)? "Adding" : (determineMode == 1)? "Linking" : "Edit Only" , 32, 32, 20, SKYBLUE);
                DrawPoly(selectedMonad->avgCenter, 3 ,  10.0f ,   0 ,  Fade(RED , 0.5f));
            }
            else
            {
                DrawText( "Null Selection" , 32, 32, 20, ORANGE);
            }

            if (selectedLink)
            {
                DrawText("Edit Link" , 32, 64, 20, PURPLE);
                Vector2 midPoint = Vector2Lerp(selectedLink->startMonad->avgCenter , selectedLink->endMonad->avgCenter , MONAD_LINK_MIDDLE_LERP);
                DrawLineBezier(selectedLink->startMonad->avgCenter , midPoint ,  4.0f , Fade(RED,0.5f));
                DrawLineBezier(midPoint , selectedLink->endMonad->avgCenter ,  2.0f ,  Fade((SameCategory( selectedLink->endMonad , selectedLink->startMonad ))? RED : PURPLE , 0.5f) );
                DrawRectangleV(midPoint , (Vector2){25.0f,25.0f}, Fade(RED , 0.5f));
            }

            for (int m = 1, d = 1 ; m <= selectedDepth ; m *= 10 , d++)
            {
                char digit[2] = {'0' + (selectedDepth/m)%10 ,  0};
                DrawText(digit, GetScreenWidth() - 32*d , 64 , 20 , SKYBLUE);
            }
        EndDrawing();

        switch (mainResult.resultKey)
        {
            case RESULT_NONE:
            break;
            case RESULT_CLICK:
                selectedMonad = mainResult.resultMonad;
                selectedLink = mainResult.resultLink;
                printf("Object %p, Link %p\n" , selectedMonad , selectedLink);
            break;
            case RESULT_RCLICK:
                if (selectedMonad)
                {
                   if ( mainResult.resultMonad && mainResult.resultContainerMonad && (mainResult.resultDepth == selectedMonad->depth) )
                   {
                       if (SameCategory(selectedMonad , mainResult.resultMonad))
                            selectedLink = AddLink(selectedMonad , mainResult.resultMonad , mainResult.resultContainerMonad);
                        else
                            selectedLink = AddLink(mainResult.resultMonad , selectedMonad , mainResult.resultContainerMonad);
                        if (selectedLink)
                        {
                            strcpy(monadLog , "Added link [");
                            strcat(monadLog , selectedLink->startMonad->name);
                            strcat(monadLog , "] to [");
                            strcat(monadLog , selectedLink->endMonad->name);
                            strcat(monadLog , "].");
                        }
                        selectedMonad = mainResult.resultMonad;
                        selectedLink = NULL;
                   }
                   else if (selectedDepth == selectedMonad->depth)
                   {
                        if (!mainResult.resultMonad)
                        {
                            strcpy(monadLog , "Added object [");
                            strcat(monadLog , AddMonad(GetMousePosition() ,  selectedMonad)->name);
                            strcat(monadLog , "].");
                        }
                        else if ( selectedLink && (selectedLink->startMonad->depth == mainResult.resultMonad->depth) )
                        {
                            strcpy(monadLog , "Changed link end object to [");
                            strcat(monadLog , (selectedLink->endMonad = mainResult.resultMonad)->name);
                            strcat(monadLog , "].");
                        }
                   }
                }
            break;
        }
        
        float mouseMove = GetMouseWheelMove();
        if (mouseMove != 0)
        {
            selectedDepth += (mouseMove > 0)? 1 : -1;
            if (selectedDepth < 0)
                selectedDepth = 0;
        }
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------

    RemoveSubMonadsRecursive(&GodMonad); // Free every object and link from memory.

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
