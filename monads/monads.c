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
-When selecting an object, you can rename it by simply typing and using backspace.

-Key 'Delete' to delete a selected object and recursively delete all objects contained within that object (and so on) and their connections from and to.
If you are selecting a link it will delete that instead of an object.
-The Alt keys will clear the last action message.
The following commands need a control key held down to function:
-Key 'B' to delete all connections from and to a selected object.
-Key 'T' will rename the selected object to your clipboard contents.
-Key 'C' will copy the selected object and recursively for its sub-objects as text data into your clipboard.
-Key 'V' will paste the text data recursively as a new object contained by the selected object.
-Key 'A' will advance the selected link's end object to its neighboring one in its stead.
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
// 3. A Link cannot comprise of Monads of different depths.
// 4. Only one combination of a Link can exist in totality.
#define MAX_MONAD_NAME_SIZE 32
#define MONAD_LINK_MIDDLE_LERP 0.35f
typedef struct Monad
{
    char name[MAX_MONAD_NAME_SIZE];
    Vector2 avgCenter;
    struct Monad* rootSubMonads;
    struct Monad* prev;
    struct Monad* next;
    struct Link* rootSubLink;
    int depth;
    char deleteFrame;
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
    int resultDepth;
    char resultKey;
} ActiveResult;

#define SCREENMARGIN 50

bool IsVector2OnScreen(Vector2 pos)
{
    return pos.x >= SCREENMARGIN && pos.x <= GetScreenWidth() - SCREENMARGIN && pos.y >= SCREENMARGIN && pos.y <= GetScreenHeight() - SCREENMARGIN;
}

// Adds an object (subMonad) to ContainingMonadPtr. ContainingMonadPtr must not be null.
struct Monad* AddMonad(Vector2 canvasPosition, Monad* containingMonadPtr)
{
    //malloc and initialize new Monad. Always initialize variables that are not being overwritten.
    Monad* newMonadPtr = (Monad*)malloc(sizeof(Monad));
    memset(newMonadPtr, 0, sizeof(Monad));

    newMonadPtr->avgCenter = canvasPosition;
    newMonadPtr->rootSubMonads = NULL;
    newMonadPtr->rootSubLink = NULL;
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

    //move containing Monad
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

    //Return existing link if it already exists.
    if (rootPtr) //has entries.
    {
        Link* iterator = rootPtr;
        do
        {
            if ((iterator->startMonad == start) && (iterator->endMonad == end))
            {
                printf("Link already exists.\n");
                return iterator;
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

//Draws dual beziers, and returns the midpoint.
Vector2 DrawDualingBeziers(Vector2 startV2 , Vector2 endV2 , Color colorCode , Color colorCode2 , float thick1 , float thick2)
{
    Vector2 midPoint = Vector2Lerp(startV2, endV2, MONAD_LINK_MIDDLE_LERP);
    float zeroDistance = startV2.x - endV2.x;
    if (zeroDistance > 0.0 && zeroDistance <= 30.0f)
    {
        midPoint.x += 30.0f - zeroDistance;
    }
    else if (zeroDistance <= 0.0 && zeroDistance >= -30.0f)
    {
        midPoint.x -= 30.0f + zeroDistance;
    }
    zeroDistance = startV2.y - endV2.y;
    if (zeroDistance > 0.0 && zeroDistance <= 30.0f)
    {
        midPoint.y += 30.0f - zeroDistance;
    }
    else if (zeroDistance <= 0.0 && zeroDistance >= -30.0f)
    {
        midPoint.y -= 30.0f + zeroDistance;
    }
    DrawLineBezier(startV2, midPoint, thick1, colorCode);
    DrawLineBezier(midPoint, endV2, thick2, colorCode2);
    return midPoint;
}

#define OUTSCOPED functionDepth > selectedDepth + 1
#define SUBSCOPE functionDepth == selectedDepth + 1
#define INSCOPE functionDepth == selectedDepth
#define PRESCOPE functionDepth < selectedDepth

//Renders all Monads and Link. Returns activated Monad, it's container, if any and the depth. MonadPtr must not be null.
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
                Vector2 startV2 = iterator->startMonad->avgCenter;
                bool linkHit = false;
                if (iterator->startMonad == iterator->endMonad)
                {
                    linkHit = CheckCollisionPointCircle(GetMousePosition(), Vector2Add(startV2, (Vector2) { 15.0f, 15.0f }), 30.0f);
                    DrawRectangleV(startV2, (Vector2) { 10.0f, 10.0f }, (linkHit) ? RED : BLACK);
                }
                else
                {
                    float giantUpLerp = fmaxf(0.3f , fminf(800.0f , Vector2Distance(startV2 , GetMousePosition())) / 800.0f);
                    Vector2 midPoint = DrawDualingBeziers(startV2 , iterator->endMonad->avgCenter , BLUE , SameCategory(iterator->endMonad, iterator->startMonad) ? BLACK : RED , 2.0f/giantUpLerp , 1.0f/giantUpLerp);
                    linkHit = CheckCollisionPointCircle(GetMousePosition() , midPoint , 30.0f);
                    if (linkHit)
                    {
                        DrawLineBezier(startV2, midPoint, 2.2f, PURPLE);
                    }
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
    float domainRadius = 0.0f;
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
                {
                    DrawLineV(MonadPtr->avgCenter, iterator->avgCenter, RED); // something went wrong if still shows.
                }
                iterator = next;
                continue;
            }
            else if (INSCOPE)
            {
                DrawLineV(MonadPtr->avgCenter, iterator->avgCenter, VIOLET);
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

            float newdomainRadius = Vector2Distance(MonadPtr->avgCenter , iterator->avgCenter);
            if (newdomainRadius > domainRadius)
            {
                domainRadius = newdomainRadius;
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
        DrawPoly(MonadPtr->avgCenter, 3, 5.0f, 0, PURPLE);
        DrawText(MonadPtr->name, (int)MonadPtr->avgCenter.x + 10, (int)MonadPtr->avgCenter.y + 10, 24, Fade(PURPLE, 0.5f));
    }
    else if (PRESCOPE)
    {
        DrawCircleLinesV(MonadPtr->avgCenter, domainRadius , Fade(GRAY, (float)functionDepth / (float)selectedDepth));
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
    DISCARD_NONE, // Neither are malloc'd.
    DISCARD_FIRST, // Use when str2 is not a malloc'd char array.
    DISCARD_BOTH // Both are malloc'd.
};

char* AppendMallocDiscard(char* str1, char* str2, char discardLevel)
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

#define _FORBIDDEN "[]:;?>\0\r\n"

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

//Finds interlinks.
typedef struct DepthResult
{
    Monad* containerMonad;
    Monad* cousinMonad;
    Monad* sharedMonad; //highest point where both container and cousin can be both traced to.
    int depth;
    int sharedDepth;

} DepthResult;
DepthResult FindDepthOfObject(Monad* selectedMonad , Monad* findMonad , Monad* findCousinMonad , int Depth)
{
    if (selectedMonad == findMonad)
    {
        return (DepthResult){NULL , NULL , NULL , Depth , -1};
    }
    Monad* rootMonadPtr = selectedMonad->rootSubMonads;
    if (rootMonadPtr)
    {
        Monad* iterator = rootMonadPtr;
        do
        {
            DepthResult result = FindDepthOfObject(iterator , findMonad , findCousinMonad , Depth + 1);
            if (result.depth != -1)
            {
                if (!result.containerMonad)
                {
                    result.containerMonad = selectedMonad;
                }
                if (result.sharedDepth == -1 && findCousinMonad)
                {
                    Monad* iterator2 = rootMonadPtr;
                    do
                    {
                        DepthResult cousinResult = FindDepthOfObject(iterator2 , findCousinMonad , NULL , Depth + 1);
                        if (cousinResult.depth != -1) // If it's found, simply
                        {
                            result.sharedMonad = selectedMonad;
                            result.cousinMonad = cousinResult.containerMonad;
                            result.sharedDepth = Depth;
                            break;
                        }
                        iterator2 = iterator2->next;
                    } while (iterator2 != rootMonadPtr);
                }
                return result;
            }
            iterator = iterator->next;
        } while (iterator != rootMonadPtr);
    }
    return (DepthResult){NULL , NULL , NULL , -1 , -1};
}

char* ChainCarrotAfterJumpStringRecursiveMalloc(Monad* sharedMonad , Monad* endMonad)
{
    Monad* matchingIterator = sharedMonad->rootSubMonads;
    char* ret = AppendMallocDiscard("","",DISCARD_NONE); // must malloc.
    if (matchingIterator)
    {
        int index = 0;
        do
        {
            if (matchingIterator == endMonad)
            {
                ret = AppendMallocDiscard(ret , ">", DISCARD_FIRST);
                return AppendMallocDiscard(ret , GenerateIDMalloc(index) , DISCARD_BOTH);
            }
            char* test = AppendMallocDiscard(ChainCarrotAfterJumpStringRecursiveMalloc(matchingIterator , endMonad) , "" , DISCARD_FIRST);
            if (test[0] != '\0') //mom get the camera.
            {
                ret = AppendMallocDiscard(ret , ">" , DISCARD_FIRST);
                ret = AppendMallocDiscard(ret , GenerateIDMalloc(index) , DISCARD_BOTH);
                return AppendMallocDiscard(ret , test , DISCARD_BOTH);
            }
            free(test);
            index++;
            matchingIterator = matchingIterator->next;
        } while (matchingIterator != sharedMonad->rootSubMonads);
    }
    return ret;
}

void PrintMonadsRecursive(Monad* MonadPtr, Monad* OriginalMonad, char** outRef)
{
    char* out = *outRef;
    out = AppendMallocDiscard(out , "[" , DISCARD_FIRST);
    out = AppendMallocDiscard(out , PruneForbiddenCharactersMalloc(MonadPtr->name) , DISCARD_BOTH);
    out = AppendMallocDiscard(out , ":" , DISCARD_FIRST);

    *outRef = out; //reset this before the iteration.
    //iterate through the objects with this object treated as a category.
    Monad* rootMonadPtr = MonadPtr->rootSubMonads;
    if (rootMonadPtr)
    {
        Monad* iterator = rootMonadPtr;
        do
        {
            PrintMonadsRecursive(iterator , OriginalMonad , outRef);
            iterator = iterator->next;
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
            DepthResult depthResult = FindDepthOfObject(OriginalMonad , iterator->startMonad , iterator->endMonad , 0);
            if (depthResult.sharedMonad)
            {
                int jumpBy = depthResult.depth - depthResult.sharedDepth - 1;
                int subIndex = 0;
                printf("DR container:%p cousin:%p shared:%p depth: %i shared depth: %i\n", depthResult.containerMonad , depthResult.cousinMonad , depthResult.sharedMonad , depthResult.depth , depthResult.sharedDepth);    
                printf("%p->%p needs a jump by: %i\n" , iterator->startMonad , iterator->endMonad , jumpBy);
                Monad* matchingIterator = rootMonadPtr;
                do
                {
                    bool startFound = matchingIterator == iterator->startMonad;
                    if (startFound || (jumpBy && matchingIterator == iterator->endMonad))
                    {
                        out = AppendMallocDiscard(out , GenerateIDMalloc(subIndex) , DISCARD_BOTH); // Start monad index.
                        out = AppendMallocDiscard(out , ">" , DISCARD_FIRST);
                        out = AppendMallocDiscard(out , GenerateIDMalloc(jumpBy) , DISCARD_BOTH); //Must "jump up" by this amount.
                        out = AppendMallocDiscard(out , ChainCarrotAfterJumpStringRecursiveMalloc(depthResult.sharedMonad , startFound ? iterator->endMonad : iterator->startMonad), DISCARD_BOTH); // Make these turns.
                        if (!startFound)
                        {
                            out = AppendMallocDiscard(out , "?" , DISCARD_FIRST);
                        }
                        out = AppendMallocDiscard(out , ";" , DISCARD_FIRST);
                        break;
                    }
                    matchingIterator = matchingIterator->next;
                    subIndex++;
                } while (matchingIterator != rootMonadPtr);
            }
            iterator = iterator->next;
        } while (iterator != rootLinkPtr);
    }
    out = AppendMallocDiscard(out , "]" , DISCARD_FIRST);
    *outRef = out;
}

enum interpretStep
{
    NAME,
    SUB,
    LINK
};

char* InterpretAddMonadsRecursive(Monad* selectedMonad , const char* in)
{
    char* progress = (char*)in + 1; //adding 1 assuming it's coming right after a '['.
    char* payload = malloc(1);
    payload[0] = '\0';
    int subCount = 0;
    char step = NAME;
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
                progress = InterpretAddMonadsRecursive(AddMonad(newV2 , selectedMonad) , progress);
                subCount++;
            break;
            case ']':
                free(payload);
                return progress;
            case ':':
                if (step == NAME)
                {
                    strncpy(selectedMonad->name, payload, MAX_MONAD_NAME_SIZE);
                }
                free(payload);
                payload = malloc(1);
                payload[0] = '\0';
                step++;
            break;
            default:
                if (step != LINK)
                {
                    char addChar[2] = {*progress , '\0'};
                    payload = AppendMallocDiscard(payload , addChar , DISCARD_FIRST);
                }
        }
        progress++;
    }
    free(payload);
    printf("Monad - no end bracket: %s\n" , selectedMonad->name);
    return progress;
}

typedef struct ParentedMonad
{
    struct Monad* monad;
    struct ParentedMonad* parentChain;
} ParentedMonad;
char* InterpretLinksRecursive(Monad* selectedMonad , ParentedMonad parentInfo , const char* in)
{
    char* progress = (char*)in + 1; //adding 1 assuming it's coming right after a '['.
    char* payload = malloc(1);
    Monad* rootMonadPtr = selectedMonad->rootSubMonads;
    Monad* subIterator = rootMonadPtr;
    Monad* findStartIterator = NULL;
    Monad* findEnderIterator = NULL;
    payload[0] = '\0';
    char payloadIndex = 0;
    char step = NAME;
    bool reverseLink = false;
    while (*progress != '\0')
    {
        switch(*progress)
        {
            case '[':
            if (subIterator)
            {
                progress = InterpretLinksRecursive(subIterator , (ParentedMonad){selectedMonad , &parentInfo} , progress);
                subIterator = subIterator->next;
            }
            break;
            case ']':
                free(payload);
                return progress;
            case ':':
                free(payload);
                payload = malloc(1);
                payload[0] = '\0';
                payloadIndex = 0;
                step++;
            break;
            case '?':
                reverseLink = true;
            break;
            case ';':
                findEnderIterator = findEnderIterator->rootSubMonads;
                Monad* rootEnderIterator = findEnderIterator;
                int endIndex = 0;
                do
                {
                    if (!strcmp(GenerateIDMalloc(endIndex) , payload))
                    {
                        break;
                    }
                    findEnderIterator = findEnderIterator->next;
                    endIndex++;
                } while (findEnderIterator != rootEnderIterator);
                if (reverseLink)
                    AddLink(findEnderIterator , findStartIterator , selectedMonad);
                else
                    AddLink(findStartIterator , findEnderIterator , selectedMonad);
                free(payload);
                payload = malloc(1);
                payload[0] = '\0';
                payloadIndex = 0;
                reverseLink = false;
            break;
            case '>':
                switch (payloadIndex)
                {
                    case 0:
                        findStartIterator = rootMonadPtr;
                        int startIndex = 0;
                        do
                        {
                            if (!strcmp(GenerateIDMalloc(startIndex) , payload))
                            {
                                break;
                            }
                            findStartIterator = findStartIterator->next;
                            startIndex++;
                        } while (findStartIterator != rootMonadPtr);
                        payloadIndex++;
                    break;
                    case 1://jump
                        findEnderIterator = selectedMonad;
                        ParentedMonad* currentChain = &parentInfo;
                        int jumpIndex = 0;
                        while (currentChain && strcmp(GenerateIDMalloc(jumpIndex) , payload)) 
                        {
                            findEnderIterator = currentChain->monad;
                            currentChain = currentChain->parentChain;
                            jumpIndex++;
                        }
                        payloadIndex++;
                    break;
                    case 2:
                        findEnderIterator = findEnderIterator->rootSubMonads;
                        Monad* rootEnderIterator = findEnderIterator;
                        int endIndex = 0;
                        do
                        {
                            if (!strcmp(GenerateIDMalloc(endIndex) , payload))
                            {
                                break;
                            }
                            findEnderIterator = findEnderIterator->next;
                            endIndex++;
                        } while (findEnderIterator != rootEnderIterator);
                }
                free(payload);
                payload = malloc(1);
                payload[0] = '\0';
            break;
            default:
                if(step == LINK)
                {
                    char addChar[2] = {*progress , '\0'};
                    payload = AppendMallocDiscard(payload , addChar , DISCARD_FIRST);
                }
        }
        progress++;
    }
    return progress;
}

void ScreenResizeSyncRecursive(Monad* monad , float ratioX , float ratioY)
{
    monad->avgCenter.x *= ratioX;
    monad->avgCenter.y *= ratioY;
    Monad* rootMonad = monad->rootSubMonads;
    if (rootMonad)
    {
        Monad* iterator = rootMonad;
        do
        {
            ScreenResizeSyncRecursive(iterator , ratioX , ratioY);
            iterator = iterator->next;
        } while (iterator != rootMonad);
    }
}

void MonadsExample(Monad* GodMonad)
{
    AddLink( AddMonad((Vector2) { 600, 500 }, GodMonad) , AddMonad((Vector2) { 200, 400 }, GodMonad) , GodMonad);
    Monad* interLinkExample = AddMonad((Vector2) { 100, 100 }, AddMonad((Vector2) { 350, 200 }, GodMonad));
    Monad* example = AddMonad((Vector2) { 400, 400 }, GodMonad);
    Monad* interLinkExample2 = AddMonad((Vector2) { 440, 410 }, example);
    AddLink(AddMonad((Vector2) { 400, 450 }, example) , AddMonad((Vector2) { 500, 500 }, example) , example);
    AddLink(interLinkExample , interLinkExample2 , example);
}

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 800;
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "Monad");
    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Variables
    //--------------------------------------------------------------------------------------
    Monad* GodMonad = malloc(sizeof(Monad));
    memset(GodMonad, 0, sizeof(Monad));

    GodMonad->avgCenter.x = screenWidth / 2.0f;
    GodMonad->avgCenter.y = screenHeight / 2.0f;
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

    int backspaceDelay = 0;
    //--------------------------------------------------------------------------------------

    // Testing
    //--------------------------------------------------------------------------------------    
    MonadsExample(GodMonad);
    //--------------------------------------------------------------------------------------

    // Main loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        int newScreenWidth = GetScreenWidth();
        int newScreenHeight = GetScreenHeight();
        if (screenWidth != newScreenWidth || screenHeight != newScreenHeight)
        {
            ScreenResizeSyncRecursive(GodMonad , (float){newScreenWidth}/(float){screenWidth} , (float){newScreenHeight}/(float){screenHeight});
            screenWidth = newScreenWidth;
            screenHeight = newScreenHeight;
        }

        mouseV2 = GetMousePosition();
        if(IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT))
        {
            strcpy(monadLog , "");
        }
        else if (selectedMonad)
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
                    strcpy(monadLog, "Cannot delete [");
                    strcat(monadLog, selectedMonad->name);
                    strcat(monadLog, "]: Is root.");
                }
                else
                {
                    if (!selectedMonad->deleteFrame)
                    {
                        strcpy(monadLog, "Deleted object [");
                        strcat(monadLog, selectedMonad->name);
                        strcat(monadLog, "].");
                        selectedMonad->deleteFrame = DELETE_PRELINK;
                    }
                    selectedMonad = NULL;
                }
            }
            else if(IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL))
            {
                if (selectedLink && IsKeyPressed(KEY_A))
                {
                    strcpy(monadLog, "Link endpoint changed from [");
                    strcat(monadLog, selectedLink->endMonad->name);
                    strcat(monadLog, "] to [");
                    selectedLink->endMonad = selectedLink->endMonad->next;
                    strcat(monadLog, selectedLink->endMonad->name);
                    strcat(monadLog, "].");
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
                else if (!selectedMonad->deleteFrame && IsKeyPressed(KEY_B))
                {
                    strcpy(monadLog, "Broke all links from and to [");
                    strcat(monadLog, selectedMonad->name);
                    strcat(monadLog, "].");
                    selectedMonad->deleteFrame = DELETE_ONLYLINK;
                }
                else if (IsKeyPressed(KEY_C))
                {
                    BeginDrawing();
                    DrawText("COPYING", screenHeight/2 - 100, screenWidth/2 - 100, 48, ORANGE);
                    EndDrawing();
                    char* out = malloc(1);
                    out[0] = '\0';
                    PrintMonadsRecursive(selectedMonad , selectedMonad , &out);
                    SetClipboardText(out);
                    printf("%s\n",out);
                    free(out);
                    strcpy(monadLog, "Copied text data from [");
                    strcat(monadLog, selectedMonad->name);
                    strcat(monadLog, "] to clipboard.");   
                }
                else if (IsKeyPressed(KEY_V) && IsVector2OnScreen(mouseV2))
                {
                    BeginDrawing();
                    DrawText("PASTING", screenHeight/2 - 100, screenWidth/2 - 100, 48, ORANGE);
                    EndDrawing();
                    Monad* pastedOverMonad = AddMonad(mouseV2 , selectedMonad);
                    InterpretAddMonadsRecursive(pastedOverMonad , GetClipboardText());
                    InterpretLinksRecursive(pastedOverMonad , (ParentedMonad){NULL , NULL} , GetClipboardText());
                    selectedMonad = pastedOverMonad;
                    pastedOverMonad->avgCenter = mouseV2;
                    strcpy(monadLog, "Pasted text data in [");
                    strcat(monadLog, selectedMonad->name);
                    strcat(monadLog, "] from clipboard.");   
                }
            }
            else if(IsKeyDown(KEY_BACKSPACE))
            {
                if (backspaceDelay)
                {
                    backspaceDelay--;
                }
                else
                {
                    if (selectedMonad)
                    {
                        selectedMonad->name[strlen(selectedMonad->name) - 1] = '\0';
                    }
                    backspaceDelay = 5;
                }
            }
            else
            {
                backspaceDelay = 0;
                int key = GetKeyPressed();
                if (key && key != KEY_LEFT_SHIFT && key != KEY_RIGHT_SHIFT && key != KEY_LEFT_SUPER && key != KEY_RIGHT_SUPER)
                {
                    int nameLength = strlen(selectedMonad->name);
                    if (nameLength < MAX_MONAD_NAME_SIZE - 1)
                    {
                        bool shift = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
                        if (key >= KEY_A && key <= KEY_Z) 
                        {
                            char c = 'a' + (key - KEY_A);
                            if (shift) 
                            {
                                c -= 32;
                            }
                            key = c;
                        }
                        if (key >= KEY_ZERO && key <= KEY_NINE) 
                        {
                            if (shift) {
                                char shifted[] = {')', '!', '@', '#', '$', '%', '^', '&', '*', '('};
                                key = shifted[key - KEY_ZERO];
                            } else {
                                key = '0' + (key - KEY_ZERO);
                            }
                        }
                        if (key == KEY_SPACE) 
                        {
                            key = ' ';
                        }
                        selectedMonad->name[nameLength] = (char)key;
                        selectedMonad->name[nameLength + 1] = '\0';
                    }
                }
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
            DrawText(selectedMonad->name, (int)selectedMonad->avgCenter.x + 10, (int)selectedMonad->avgCenter.y + 10, selectedDepth < selectedMonad->depth ? 16 : 24, Fade(ORANGE, 0.5f));
        }
        else
        {
            DrawText("Null Selection", 32, 32, 20, ORANGE);
        }

        if (selectedLink)
        {
            DrawText("Edit Link", 32, 64, 20, PURPLE);
            Vector2 linkLocation;
            Vector2 endV2 = selectedLink->endMonad->avgCenter;
            Vector2 endNextV2 = selectedLink->endMonad->next->avgCenter;
            if (selectedLink->startMonad == selectedLink->endMonad)
            {
                linkLocation = selectedLink->startMonad->avgCenter;
            }
            else
            {
                linkLocation = DrawDualingBeziers(selectedLink->startMonad->avgCenter , selectedLink->endMonad->avgCenter , Fade(RED, 0.5f) , Fade(SameCategory(selectedLink->endMonad, selectedLink->startMonad) ? RED : PURPLE, 0.5f) , 3.5 , 1.5f);
            }
            linkLocation.x -= 12.0f;
            linkLocation.y -= 12.0f;
            DrawRectangleV(linkLocation , (Vector2){24.0f, 24.0f} , Fade(RED, 0.5f));
            DrawLineV(endV2, Vector2Add(endNextV2, Vector2Scale(Vector2Subtract(endV2, endNextV2), 0.9f)), ORANGE);
        }

        for (int m = 1, d = 1; m <= selectedDepth; m *= 10, d++)
        {
            char digit[2] = { '0' + (selectedDepth / m) % 10 ,  0 };
            DrawText(digit, screenWidth - 32 * d, 64, 20, SKYBLUE);
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
    CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}