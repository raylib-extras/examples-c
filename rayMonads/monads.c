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
-If you hold the left mouse button down, you can drag the currently selected object around.

-Key 'B' to delete all connections from and to a selected object.
-Key 'Delete' to delete a selected object and recursively delete all objects contained within that object (and so on) and their connections from and to.
If you are selecting a link it will delete that instead of an object.
-Key 'T' will rename the selected object to your clipboard contents.
-Key 'C' will copy the selected object and recursively for its sub-objects as text data into your clipboard.
-Key 'V' will paste the clipboard contents, assuming it's good text data, recursively into named sub-objects of the selected object. This renames the selected object as well.
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
    Vector2 avgCenter, defaultCenter;
    struct Monad* rootSubMonads;
    struct Monad* prev;
    struct Monad* next;
    struct Link* rootSubLink;
    float radius;
    int depth;
    int deleteFrame;
}  Monad;

typedef struct Link
{
    struct Monad* startMonad;
    struct Monad* endMonad;
    struct Link* prev;
    struct Link* next;
} Link;

enum Response
{
    RESULT_NONE,
    RESULT_CLICK,
    RESULT_RCLICK
};

// After returning recursively up the chain, certain results can override other results depending on the situation.
typedef struct ActiveResult
{
    struct Monad* resultMonad;
    struct Monad* resultContainerMonad;
    struct Link* resultLink;
    int resultKey, resultDepth;
} ActiveResult;

#define SCREENMARGIN 50

bool IsVector2OnScreen(Vector2 pos)
{
    return pos.x >= SCREENMARGIN && pos.x <= GetScreenWidth() - SCREENMARGIN &&
           pos.y >= SCREENMARGIN && pos.y <= GetScreenHeight() - SCREENMARGIN;
}

// Adds an object (subMonad) to ContainingMonadPtr. ContainingMonadPtr must not be null.
struct Monad* AddMonad(Vector2 canvasPosition, Monad* containingMonadPtr)
{
    //malloc and initialize new Monad. Always initialize variables that are not being overwritten.
    Monad* newMonadPtr = (Monad*)malloc(sizeof(Monad));
    memset(newMonadPtr, 0, sizeof(Monad));

    newMonadPtr->defaultCenter = canvasPosition;
    newMonadPtr->avgCenter = canvasPosition;
    newMonadPtr->rootSubMonads = NULL;
    newMonadPtr->rootSubLink = NULL;
    newMonadPtr->radius = 10.0f;
    newMonadPtr->depth = containingMonadPtr->depth + 1;
    newMonadPtr->deleteFrame = DELETE_OFF;

    newMonadPtr->name[0] = (containingMonadPtr->rootSubMonads) ? containingMonadPtr->rootSubMonads->prev->name[0] + 1 : 'A';
    newMonadPtr->name[1] = 0;

    //insert new Monad in list entry.
    Monad* rootPtr = containingMonadPtr->rootSubMonads;
    if (rootPtr) //has entries.
    {
        Monad* rootNextPtrUnchanged = rootPtr->next;
        Monad* rootPrevPtrUnchanged = rootPtr->prev;
        if ((rootPtr == rootNextPtrUnchanged) || (rootPtr == rootPrevPtrUnchanged)) //after one entry
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
    float prospectDistance = Vector2Distance(containingMonadPtr->avgCenter, canvasPosition) * 1.5f;
    if (prospectDistance > containingMonadPtr->radius)
    {
        containingMonadPtr->radius = prospectDistance;
    }
    containingMonadPtr->avgCenter = Vector2Scale(Vector2Add(containingMonadPtr->avgCenter, canvasPosition), 0.5f);

    return newMonadPtr;
}

// Recursively frees the object and its links after calling the function for its sub-objects.
void  RemoveSubMonadsRecursive(Monad* MonadPtr)
{
    Monad* rootMonad = MonadPtr->rootSubMonads;
    if (rootMonad)
    {
        Monad* iterator = rootMonad;
        do
        {
            Monad* nextMonad = iterator->next;
            RemoveSubMonadsRecursive(iterator);
            iterator = nextMonad;
        } while (iterator != rootMonad);
    }

    Link* rootLink = MonadPtr->rootSubLink;
    if (rootLink)
    {
        Link* iterator = rootLink;
        do
        {
            Link* nextLink = iterator->next;
            free(iterator);
            iterator = nextLink;
        } while (iterator != rootLink);
    }

    free(MonadPtr);
}

// Remove an object (subMonad) from containingMonadPtr. containingMonadPtr must not be null.
bool RemoveMonad(Monad* MonadPtr, Monad* containingMonadPtr)
{
    Monad* rootMonad = containingMonadPtr->rootSubMonads;
    if (rootMonad)
    {
        Monad* iterator = rootMonad;
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
bool SameCategory(Monad* MonadPtr, Monad* MonadMatePtr)
{
    Monad* iterator = MonadMatePtr;
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
struct Link* AddLink(Monad* start, Monad* end, Monad* containingMonadPtr)
{
    Link* rootPtr = containingMonadPtr->rootSubLink;

    //Return nothing if it already exists
    if (rootPtr) //has entries.
    {
        Link* iterator = rootPtr;
        do
        {
            if ((iterator->startMonad == start) && (iterator->endMonad == end))
            {
                printf("Link already exists.\n");
                return NULL;
            }
            iterator = iterator->next;
        } while (iterator != rootPtr);
    }
    //malloc and initialize new Link. Always initialize variables that are not being overwritten.
    Link* newLinkPtr = (Link*)malloc(sizeof(Link));
    newLinkPtr->startMonad = start;
    newLinkPtr->endMonad = end;

    //insert new Link in list entry.
    if (rootPtr) //has entries.
    {
        Link* rootNextPtrUnchanged = rootPtr->next;
        Link* rootPrevPtrUnchanged = rootPtr->prev;
        if ((rootPtr == rootNextPtrUnchanged) || (rootPtr == rootPrevPtrUnchanged)) //after one entry
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
bool RemoveLink(Link* linkPtr, Monad* containingMonadPtr)
{
    Link* rootLink = containingMonadPtr->rootSubLink;
    if (rootLink)
    {
        Link* iterator = rootLink;
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
struct ActiveResult RecursiveDraw(Monad* MonadPtr, int functionDepth, int selectedDepth)
{
    //check collision with mouse, generate first part of activeResult.
    ActiveResult activeResult = (ActiveResult){ 0 };
    activeResult.resultKey = (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) ? RESULT_CLICK : ((IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) ? RESULT_RCLICK : RESULT_NONE);
    activeResult.resultDepth = functionDepth;
    if ((functionDepth >= selectedDepth) && CheckCollisionPointCircle(GetMousePosition(), MonadPtr->avgCenter, 30.0f))
    {
        activeResult.resultMonad = MonadPtr;
    }

    //iterate through the functors in the category.
    Link* rootLinkPtr = MonadPtr->rootSubLink;
    if (rootLinkPtr)
    {
        Link* iterator = rootLinkPtr;
        do
        {
            if (INSCOPE)
            {
                bool linkHit = false;
                if (iterator->startMonad == iterator->endMonad)
                {
                    linkHit = CheckCollisionPointCircle(GetMousePosition(), Vector2Add(iterator->startMonad->avgCenter, (Vector2) { 15.0f, 15.0f }), 30.0f);
                    DrawRectangleV(iterator->startMonad->avgCenter, (Vector2) { 10.0f, 10.0f }, (linkHit) ? RED : BLACK);
                }
                else
                {
                    Vector2 midPoint = Vector2Lerp(iterator->startMonad->avgCenter, iterator->endMonad->avgCenter, MONAD_LINK_MIDDLE_LERP);
                    linkHit = CheckCollisionPointCircle(GetMousePosition(), midPoint, 30.0f);
                    DrawLineBezier(iterator->startMonad->avgCenter, midPoint, 2.0f, (linkHit) ? PURPLE : BLUE);
                    DrawLineBezier(midPoint, iterator->endMonad->avgCenter, 1.0f, (SameCategory(iterator->endMonad, iterator->startMonad)) ? BLACK : RED);
                }
                if (linkHit)
                {
                    activeResult.resultLink = iterator;
                    activeResult.resultMonad = MonadPtr;
                }
            }

            Link* nextSaved = iterator->next;
            if ((iterator->startMonad->deleteFrame >= DELETE_POSTONLYLINK) || (iterator->endMonad->deleteFrame >= DELETE_POSTONLYLINK))
            {
                if (RemoveLink(iterator, MonadPtr) && !(rootLinkPtr = MonadPtr->rootSubLink))
                {
                    break;
                }
            }
            iterator = nextSaved;

        } while (iterator != rootLinkPtr);
    }

    //iterate through the objects with this object treated as a category.
    Monad* rootMonadPtr = MonadPtr->rootSubMonads;
    if (rootMonadPtr)
    {
        Monad* iterator = rootMonadPtr;
        do
        {
            Monad* next = iterator->next;

            if (iterator->deleteFrame >= DELETE_FINAL)
            {
                if (RemoveMonad(iterator, MonadPtr) && !(rootMonadPtr = MonadPtr->rootSubMonads))
                {
                    break;
                }
                else
                    DrawLineV(MonadPtr->avgCenter, iterator->avgCenter, RED); // something went wrong if still shows.
                iterator = next;
                continue;
            }
            else if (INSCOPE)
            {
                DrawLineV(MonadPtr->avgCenter, iterator->avgCenter, (iterator == rootMonadPtr) ? VIOLET : GREEN);
                DrawLineV(next->avgCenter, iterator->avgCenter, Fade((iterator == rootMonadPtr->prev) ? ORANGE : YELLOW, 0.5f));
            }

            //--------------------------------
            ActiveResult activeOverride = RecursiveDraw(iterator, functionDepth + 1, selectedDepth);
            //--------------------------------

            if (activeOverride.resultMonad && !activeResult.resultLink)
            {
                activeResult = activeOverride;
            }
            else if (activeOverride.resultLink)
            {
                activeResult.resultLink = activeOverride.resultLink;
                activeResult.resultMonad = MonadPtr;
            }

            iterator = next;
        } while (iterator != rootMonadPtr);
    }

    //mark for deletion progression
    if (MonadPtr->deleteFrame >= DELETE_PRELINK)
    {
        MonadPtr->deleteFrame++;
        return (ActiveResult) { 0 };
    }
    else if (MonadPtr->deleteFrame >= DELETE_POSTONLYLINK)
    {
        MonadPtr->deleteFrame--;
    }

    //cancel any more drawing.
    if (OUTSCOPED)
        return (ActiveResult) { 0 };

    //we have returned back to the container, since this is null, we know that this is the container.
    if (!activeResult.resultContainerMonad && (activeResult.resultMonad != MonadPtr))
        activeResult.resultContainerMonad = MonadPtr;

    if (INSCOPE)
    {
        //DrawCircleLinesV(MonadPtr->avgCenter , MonadPtr->radius , GREEN);
        DrawPoly(MonadPtr->avgCenter, 3, 5.0f, 0, PURPLE);
        DrawText(MonadPtr->name, (int)MonadPtr->avgCenter.x + 10, (int)MonadPtr->avgCenter.y + 10, 24, Fade(PURPLE, 0.5f));
    }
    else if (PRESCOPE)
    {
        DrawCircleLinesV(MonadPtr->avgCenter, MonadPtr->radius, Fade(GRAY, (float)functionDepth / (float)selectedDepth));
    }
    else if (SUBSCOPE)
    {
        DrawCircleV(MonadPtr->avgCenter, 5.0f, BLUE);
        DrawText(MonadPtr->name, (int)MonadPtr->avgCenter.x + 10, (int)MonadPtr->avgCenter.y + 10, 16, Fade(SKYBLUE, 0.5f));
    }

    if (activeResult.resultMonad == MonadPtr)
        DrawCircleLinesV(MonadPtr->avgCenter, 20.0f, ORANGE);

    return activeResult;
}

enum discardAppend
{
    DISCARD_NONE,
    DISCARD_FIRST,// use when str2 is not a malloc'd char array.
    DISCARD_BOTH
};

char* AppendMallocDiscard(char* str1, char* str2, int discardLevel)
{
    char* new_str = NULL;
    if ((new_str = malloc(strlen(str1)+strlen(str2)+1)))
    {
        new_str[0] = '\0';
        strcat(new_str,str1);
        strcat(new_str,str2);
    }

    if (discardLevel >= DISCARD_FIRST)
    {
        if(str1)
        {
            free(str1);
        }
        if(discardLevel >= DISCARD_BOTH && str2)
        {
            free(str2);
        }
    }

    return new_str;
}

#define _FORBIDDEN "[]:;>\0\r\n"

char* GenerateIDMalloc(int index) //sub monads limited by the highest int, really high.
{
    index++;//so it isn't 0
    char* forbiddenChars = _FORBIDDEN;
    char* ret = malloc(1);
    ret[0] = '\0';
    #define _HIGHESTCHAR 255
    for (; index; index /= _HIGHESTCHAR)
    {
        char character[2] = {(char)(index % _HIGHESTCHAR) , '\0'};
        char* iteratorForbidden = forbiddenChars;
        while (iteratorForbidden[0] != '\0')
        {
            if (character[0] == iteratorForbidden[0])
            {
                character[0]++;
            }
            else
            {
                iteratorForbidden++;
            }
        }
        ret = AppendMallocDiscard(ret , character , DISCARD_FIRST);
    }
    return ret;
}

char* PruneForbiddenCharactersMalloc(char* name)
{
    char* newName = malloc(strlen(name) + 1);
    char* forbiddenChars = _FORBIDDEN;
    int index = 0;
    while (name[index] != '\0')
    {
        char* editedCharacter = &newName[index];
        char* iteratorForbidden = forbiddenChars;
        *editedCharacter = name[index];
        while (iteratorForbidden[0] != '\0')
        {
            if (*editedCharacter == iteratorForbidden[0])
            {
                *editedCharacter = '_';
                break;
            }
            iteratorForbidden++;
        }
        index++;
    }
    newName[index] = '\0';
    return newName;
}

void PrintMonadsRecursive(Monad* MonadPtr, int index, char** outRef) //outref remains the same value through the entire recursion, is that okay?
{
    char* out = *outRef;
    out = AppendMallocDiscard(out , "[" , DISCARD_FIRST);
    out = AppendMallocDiscard(out , GenerateIDMalloc(index) , DISCARD_BOTH);
    out = AppendMallocDiscard(out , ":" , DISCARD_FIRST);
    out = AppendMallocDiscard(out , PruneForbiddenCharactersMalloc(MonadPtr->name) , DISCARD_BOTH);
    out = AppendMallocDiscard(out , ":" , DISCARD_FIRST);

    *outRef = out; //reset this before the iteration.
    //iterate through the objects with this object treated as a category.
    Monad* rootMonadPtr = MonadPtr->rootSubMonads;
    if (rootMonadPtr)
    {
        int subIndex = 0;
        Monad* iterator = rootMonadPtr;
        do
        {
            PrintMonadsRecursive(iterator, subIndex, outRef);
            iterator = iterator->next;
            subIndex++;
        } while (iterator != rootMonadPtr);
        out = *outRef; // Old reference is most certainly freed in recursive calls. Update.
    }

    out = AppendMallocDiscard(out , ":" , DISCARD_FIRST);

    //iterate through the functors in the category.
    Link* rootLinkPtr = MonadPtr->rootSubLink;
    if (rootLinkPtr && rootMonadPtr)
    {
        Link* iterator = rootLinkPtr;
        do
        {
            int subIndex = 0;
            Monad* matchingIterator = rootMonadPtr;
            do
            {
                if (matchingIterator == iterator->startMonad)
                {
                    int subIndex2 = 0;
                    Monad* matchingIterator2 = rootMonadPtr;
                    do
                    {
                        if (matchingIterator2 == iterator->endMonad)
                        {
                            out = AppendMallocDiscard(out , GenerateIDMalloc(subIndex) , DISCARD_BOTH);
                            out = AppendMallocDiscard(out , ">" , DISCARD_FIRST);
                            out = AppendMallocDiscard(out , GenerateIDMalloc(subIndex2) , DISCARD_BOTH);
                            out = AppendMallocDiscard(out , ";" , DISCARD_FIRST);
                            break;
                        }
                        matchingIterator2 = matchingIterator2->next;
                        subIndex2++;
                    } while (matchingIterator2 != rootMonadPtr);
                    break;
                }
                matchingIterator = matchingIterator->next;
                subIndex++;
            } while (matchingIterator != rootMonadPtr);
            iterator = iterator->next;
        } while (iterator != rootLinkPtr);
    }

    out = AppendMallocDiscard(out , "]" , DISCARD_FIRST);
    *outRef = out;
}

enum interpretStep
{
    ID,
    NAME,
    SUB,
    LINK
};

char* InterpretAddMonadsAndLinksRecursive(Monad* selectedMonad , const char* in)
{
    char* progress = (char*)in + 1; //adding 1 assuming it's coming right after a '['.
    char* payload = malloc(1);
    char* payload2 = malloc(1);
    int payloadIndex = 0;
    payload[0] = '\0';
    payload2[0] = '\0';
    int step = ID;
    int subCount = 0;
    Monad* rootMonadPtr = selectedMonad->rootSubMonads;
    Monad* firstNewMonad = NULL;
    Monad* lastNewMonad = NULL;
    if (rootMonadPtr)
    {
        Monad* iterator = rootMonadPtr;
        do
        {
            subCount++;
        } while (iterator != rootMonadPtr);
    }
    while (*progress != '\0')
    {
        switch(*progress)
        {
            case '[':
                Vector2 oriV2 = selectedMonad->avgCenter;
                Vector2 newV2 = (Vector2){oriV2.x + subCount*(oriV2.x < GetScreenWidth()/2 ? 60.1f : -60.1f) + 60.0f , oriV2.y + subCount*(oriV2.y < GetScreenHeight()/2 ? 60.0f : -60.0f)};
                if(!IsVector2OnScreen(newV2))
                {
                    newV2 = (Vector2){GetScreenWidth() - 70.0f , GetScreenHeight() - 70.0f};
                }
                Monad* newMonadPtr = AddMonad(newV2 , selectedMonad);
                if (!firstNewMonad)
                {
                    firstNewMonad = newMonadPtr;
                }
                progress = InterpretAddMonadsAndLinksRecursive(newMonadPtr , progress);
                subCount++;
            break;
            case ']':
                free(payload);
                free(payload2);
                return progress;
            case ':':
                switch (step)
                {
                case NAME:
                    strncpy(selectedMonad->name, payload, MAX_MONAD_NAME_SIZE);
                break;
                case SUB:
                    lastNewMonad = selectedMonad->prev;
                }
                free(payload);
                free(payload2);
                payload = malloc(1);
                payload2 = malloc(1);
                payload[0] = '\0';
                payload2[0] = '\0';
                payloadIndex = 0;
                step++;
            break;
            case ';':
                if (firstNewMonad && lastNewMonad)
                {
                    Monad* iterator = firstNewMonad;
                    int index = 0;
                    do
                    {
                        char* left = GenerateIDMalloc(index);
                        if (!strcmp(left , payload))
                        {
                            Monad* iterator2 = firstNewMonad;
                            int index2 = 0;
                            do
                            {
                                char* right = GenerateIDMalloc(index2);
                                if (!strcmp(right , payload2) && AddLink(iterator , iterator2 , selectedMonad))
                                {
                                    free(right);
                                    break;
                                }
                                free(right);
                                index2++;
                                iterator2 = iterator2->next;
                            } while (iterator2 != lastNewMonad);
                            free(left);
                            break;
                        }
                        free(left);
                        index++;
                        iterator = iterator->next;
                    } while (iterator != lastNewMonad);
                }
                free(payload);
                free(payload2);
                payload = malloc(1);
                payload2 = malloc(1);
                payload[0] = '\0';
                payload2[0] = '\0';
                payloadIndex = 0;
            break;
            case '>':
                payloadIndex = 1;
            break;
            default:
                char addChar[2] = {*progress , '\0'};
                if(payloadIndex == 1)
                {
                    payload2 = AppendMallocDiscard(payload2 , addChar , DISCARD_FIRST);
                }
                else
                {
                    payload = AppendMallocDiscard(payload , addChar , DISCARD_FIRST);
                }
        }
        progress++;
    }
    free(payload);
    free(payload2);
    printf("Monad - no end bracket: %s\n" , selectedMonad->name);
    return progress;
}

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 800;
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "Monad");
    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Variables
    //--------------------------------------------------------------------------------------
    Monad* GodMonad = malloc(sizeof(Monad));
    memset(GodMonad, 0, sizeof(Monad));

    GodMonad->avgCenter.x = screenWidth / 2.0f;
    GodMonad->avgCenter.y = screenHeight / 2.0f;
    GodMonad->defaultCenter = GodMonad->avgCenter;
    GodMonad->prev = GodMonad;
    GodMonad->next = GodMonad;
    strcpy(GodMonad->name, "Monad 0");

    Vector2 mouseV2;
    char monadLog[MAX_MONAD_NAME_SIZE * 3] = "Session started.";
    Monad* selectedMonad = NULL;
    Link* selectedLink = NULL;
    int selectedDepth = 0;
    ActiveResult mainResult = (ActiveResult){ 0 };
    bool selectDrag = false;
    //--------------------------------------------------------------------------------------

    // Testing
    //--------------------------------------------------------------------------------------    
    AddMonad((Vector2) { 600, 500 }, GodMonad);
    AddMonad((Vector2) { 200, 400 }, GodMonad);
    AddMonad((Vector2) { 100, 100 }, AddMonad((Vector2) { 350, 200 }, GodMonad));

    Monad* example = AddMonad((Vector2) { 400, 400 }, GodMonad);
    AddMonad((Vector2) { 440, 410 }, example);
    AddLink(AddMonad((Vector2) { 400, 450 }, example) , AddMonad((Vector2) { 500, 500 }, example) , example);
    //--------------------------------------------------------------------------------------

    // Main loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        mouseV2 = GetMousePosition();
        if (selectedMonad)
        {
            if (selectedLink && IsKeyPressed(KEY_DELETE))
            {
                strcpy(monadLog, "Link [");
                strcat(monadLog, selectedLink->startMonad->name);
                strcat(monadLog, "] to [");
                strcat(monadLog, selectedLink->endMonad->name);
                if (RemoveLink(selectedLink, selectedMonad))
                {
                    selectedLink = NULL;
                    strcat(monadLog, "] deleted.");
                }
                else
                    strcat(monadLog, "] failed to delete.");
            }
            else if (IsKeyPressed(KEY_DELETE))
            {
                if (selectedMonad == GodMonad)
                {
                    strcpy(monadLog, "Cannot delete Monad 0.");
                }
                else
                {
                    strcpy(monadLog, "Deleted object [");
                    strcat(monadLog, selectedMonad->name);
                    strcat(monadLog, "].");
                    selectedMonad->deleteFrame = DELETE_PRELINK;
                    selectedMonad = NULL;
                }
            }
            else if (IsKeyPressed(KEY_B))
            {
                strcpy(monadLog, "Broke all links from and to [");
                strcat(monadLog, selectedMonad->name);
                strcat(monadLog, "].");
                selectedMonad->deleteFrame = DELETE_ONLYLINK;
            }
            else if (IsKeyPressed(KEY_T))
            {
                strcpy(monadLog, "Renamed [");
                strcat(monadLog, selectedMonad->name);
                strcat(monadLog, "] to [");
                strncpy(selectedMonad->name, GetClipboardText(), MAX_MONAD_NAME_SIZE);
                selectedMonad->name[MAX_MONAD_NAME_SIZE - 1] = '\0'; //ensures NULL termination.
                strcat(monadLog, selectedMonad->name);
                strcat(monadLog, "].");
            }
            else if (IsKeyPressed(KEY_C))
            {
                char* out = malloc(1);
                out[0] = '\0';
                PrintMonadsRecursive(selectedMonad , 0 , &out);
                printf("%s\n" , out);
                SetClipboardText(out);
                free(out);
                strcpy(monadLog, "Copied text data from ");
                strcat(monadLog, selectedMonad->name);
                strcat(monadLog, "to clipboard.");   
            }
            else if (IsKeyPressed(KEY_V))
            {
                InterpretAddMonadsAndLinksRecursive(selectedMonad , GetClipboardText());
                strcpy(monadLog, "Pasted text data in ");
                strcat(monadLog, selectedMonad->name);
                strcat(monadLog, " from clipboard.");   
            }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        mainResult = RecursiveDraw(GodMonad, 0, selectedDepth);

        DrawText(monadLog, 48, 8, 20, GRAY);

        if (selectedMonad)
        {
            int determineMode = selectedMonad->depth - selectedDepth;
            DrawText((!determineMode) ? "Adding" : (determineMode == 1) ? "Linking" : "Edit Only", 32, 32, 20, SKYBLUE);
            DrawPoly(selectedMonad->avgCenter, 3, 10.0f, 0, Fade(RED, 0.5f));
        }
        else
        {
            DrawText("Null Selection", 32, 32, 20, ORANGE);
        }

        if (selectedLink)
        {
            DrawText("Edit Link", 32, 64, 20, PURPLE);
            Vector2 midPoint = Vector2Lerp(selectedLink->startMonad->avgCenter, selectedLink->endMonad->avgCenter, MONAD_LINK_MIDDLE_LERP);
            DrawLineBezier(selectedLink->startMonad->avgCenter, midPoint, 4.0f, Fade(RED, 0.5f));
            DrawLineBezier(midPoint, selectedLink->endMonad->avgCenter, 2.0f, Fade((SameCategory(selectedLink->endMonad, selectedLink->startMonad)) ? RED : PURPLE, 0.5f));
            DrawRectangleV(midPoint, (Vector2) { 25.0f, 25.0f }, Fade(RED, 0.5f));
        }

        for (int m = 1, d = 1; m <= selectedDepth; m *= 10, d++)
        {
            char digit[2] = { '0' + (selectedDepth / m) % 10 ,  0 };
            DrawText(digit, GetScreenWidth() - 32 * d, 64, 20, SKYBLUE);
        }
        EndDrawing();

        switch (mainResult.resultKey)
        {
        case RESULT_NONE:
            break;
        case RESULT_CLICK:
            selectedMonad = mainResult.resultMonad;
            selectedLink = mainResult.resultLink;
            printf("Object %p, Link %p\n", selectedMonad, selectedLink);
            break;
        case RESULT_RCLICK:
            if (selectedMonad)
            {
                if (mainResult.resultMonad && mainResult.resultContainerMonad && (mainResult.resultDepth == selectedMonad->depth))
                {
                    if (SameCategory(selectedMonad, mainResult.resultMonad))
                        selectedLink = AddLink(selectedMonad, mainResult.resultMonad, mainResult.resultContainerMonad);
                    else
                        selectedLink = AddLink(mainResult.resultMonad, selectedMonad, mainResult.resultContainerMonad);
                    if (selectedLink)
                    {
                        strcpy(monadLog, "Added link [");
                        strcat(monadLog, selectedLink->startMonad->name);
                        strcat(monadLog, "] to [");
                        strcat(monadLog, selectedLink->endMonad->name);
                        strcat(monadLog, "].");
                    }
                    selectedMonad = mainResult.resultMonad;
                    selectedLink = NULL;
                }
                else if (selectedDepth == selectedMonad->depth)
                {
                    if (!mainResult.resultMonad && Vector2Distance(selectedMonad->avgCenter, mouseV2) >= 30.0f /*deny if too close to container.*/)
                    {
                        strcpy(monadLog, "Added object [");
                        strcat(monadLog, AddMonad(mouseV2, selectedMonad)->name);
                        strcat(monadLog, "].");
                    }
                    else if (selectedLink && (selectedLink->startMonad->depth == mainResult.resultMonad->depth))
                    {
                        strcpy(monadLog, "Changed link end object to [");
                        strcat(monadLog, (selectedLink->endMonad = mainResult.resultMonad)->name);
                        strcat(monadLog, "].");
                    }
                }
            }
            break;
        }

        if (selectedMonad && IsMouseButtonDown(MOUSE_BUTTON_LEFT) && (selectDrag || Vector2Distance(selectedMonad->avgCenter, mouseV2) <= 30.0f))
        {
            if (IsVector2OnScreen(mouseV2))
            {
                selectedMonad->avgCenter = mouseV2;
            }
            selectDrag = true;
        }
        else
        {
            selectDrag = false;
        }

        float mouseMove = GetMouseWheelMove();
        if (mouseMove != 0)
        {
            selectedDepth += (mouseMove > 0) ? 1 : -1;
            if (selectedDepth < 0)
                selectedDepth = 0;
        }
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------

    RemoveSubMonadsRecursive(GodMonad); // Free every object and link from memory.

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
