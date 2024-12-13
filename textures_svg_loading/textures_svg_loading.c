/*******************************************************************************************
*
*   raylib [textures] example - SVG loading and texture creation
*
*   NOTE: Images are loaded in CPU memory (RAM); textures are loaded in GPU memory (VRAM)
*
*   Example originally created with raylib 4.2, last time updated with raylib 5.5
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2022-2024 Dennis Meinen (@bixxy#4258 on Discord) and Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#define NANOSVG_IMPLEMENTATION          // Expands implementation
#include "nanosvg.h"

#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvgrast.h"

// Load SVG image, rasteraizing it at desired width and height
// NOTE: If width/height are 0, using internal default width/height
static Image LoadImageSVG(const char *fileName, int width, int height);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [textures] example - svg loading");

    // NOTE: Textures MUST be loaded after Window initialization (OpenGL context is required)

    Image image = LoadImageSVG("resources/test.svg", 400, 350); // Loaded in CPU memory (RAM)
    Texture2D texture = LoadTextureFromImage(image); // Image converted to texture, GPU memory (VRAM)
    UnloadImage(image);   // Once image has been converted to texture and uploaded to VRAM, it can be unloaded from RAM

    SetTargetFPS(60);     // Set our game to run at 60 frames-per-second
    //---------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // TODO: Update your variables here
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawTexture(texture, screenWidth/2 - texture.width/2, screenHeight/2 - texture.height/2, WHITE);

            // Red border to illustrate how the SVG is centered within the specified dimensions
            DrawRectangleLines((screenWidth/2 - texture.width/2) - 1, (screenHeight/2 - texture.height/2) - 1, texture.width + 2, texture.height + 2, RED);

            DrawText("this IS a texture loaded from an SVG file!", 300, 410, 10, GRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(texture);       // Texture unloading

    CloseWindow();                // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

// Load SVG image, rasteraizing it at desired width and height
// NOTE: If width/height are 0, using internal default width/height
static Image LoadImageSVG(const char *fileName, int width, int height)
{
    Image image = { 0 };

    if ((strcmp(GetFileExtension(fileName), ".svg") == 0) ||
        (strcmp(GetFileExtension(fileName), ".SVG") == 0))
    {
        int dataSize = 0;
        unsigned char *fileData = NULL;

        fileData = LoadFileData(fileName, &dataSize);

        // Make sure the file data contains an EOL character: '\0'
        if ((dataSize > 0) && (fileData[dataSize - 1] != '\0'))
        {
            fileData = RL_REALLOC(fileData, dataSize + 1);
            fileData[dataSize] = '\0';
            dataSize += 1;
        }

        // Validate fileData as valid SVG string data
        //<svg xmlns="http://www.w3.org/2000/svg" width="2500" height="2484" viewBox="0 0 192.756 191.488">
        if ((fileData != NULL) &&
            (fileData[0] == '<') &&
            (fileData[1] == 's') &&
            (fileData[2] == 'v') &&
            (fileData[3] == 'g'))
        {
            struct NSVGimage *svgImage = nsvgParse(fileData, "px", 96.0f);
            unsigned char *imgData = RL_MALLOC(svgImage->width*svgImage->height*4);

            // NOTE: If required width or height is 0, using default SVG internal value
            if (width == 0) width = svgImage->width;
            if (height == 0) height = svgImage->height;

            // Calculate scales for both the width and the height
            float scaleWidth = width/svgImage->width;
            float scaleHeight = height/svgImage->height;

            // Set the largest of the 2 scales to be the scale to use
            float scale = (scaleHeight > scaleWidth)? scaleWidth : scaleHeight;

            int offsetX = 0;
            int offsetY = 0;

            if (scaleHeight > scaleWidth) offsetY = (height - svgImage->height*scale)/2;
            else offsetX = (width - svgImage->width*scale)/2;

            // Rasterize
            struct NSVGrasterizer *rast = nsvgCreateRasterizer();
            nsvgRasterize(rast, svgImage, offsetX, offsetY, scale, imgData, width, height, width*4);

            // Populate image struct with all data
            image.data = imgData;
            image.width = width;
            image.height = height;
            image.mipmaps = 1;
            image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

            nsvgDelete(svgImage);
            nsvgDeleteRasterizer(rast);
        }

        UnloadFileData(fileData);
    }

    return image;
}