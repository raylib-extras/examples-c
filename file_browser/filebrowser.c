/**********************************************************************************************
*
*   raylib-extras, examples-c * examples for Raylib in C
*
*   A small file explorer, with the ability to display different languages.
*
*   LICENSE: ZLib
*
*   Copyright (c) 2025 Gunko Vadim
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <raylib.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define FONT_SIZE 20
#define ITEM_HEIGHT 30
#define MAX_VISIBLE_ITEMS 15
#define MARGIN 10
#define SCROLL_SPEED 1

typedef struct {
    char **items;
    int count;
} FileList;

FileList files = {0};
int scroll_offset = 0;
int selected_item = 0;
char current_dir[1024] = {0};
char parent_dir[1024] = {0};
Rectangle file_list_area;
Font mlang_font;

// Function to load a font with multilingual support
Font LoadUnicodeFont() {
    int *codepoints = NULL;
    int count = 0;
    int capacity = 0;

    // Macro to add character ranges
    #define ADD_RANGE(start, end) \
        for (int i = start; i <= end; i++) { \
            if (count >= capacity) { \
                capacity = (capacity == 0) ? 256 : capacity * 2; \
                codepoints = realloc(codepoints, capacity * sizeof(int)); \
            } \
            codepoints[count++] = i; \
        }

    // Basic ASCII characters
    ADD_RANGE(32, 126);

    // Main European languages
    ADD_RANGE(0xC0, 0x17F);  // Latin-1 Supplement + Latin Extended-A
    ADD_RANGE(0x180, 0x24F); // Latin Extended-B
    ADD_RANGE(0x370, 0x3FF); // Greek and Coptic
    ADD_RANGE(0x400, 0x4FF); // Cyrillic
    ADD_RANGE(0x500, 0x52F); // Cyrillic Supplement
    ADD_RANGE(0x1E00, 0x1EFF); // Latin Extended Additional

    // Special characters for specific languages
    ADD_RANGE(0x1E00, 0x1EFF); // Latin Extended Additional (Fon, Ewe, etc.)
    ADD_RANGE(0x2C60, 0x2C7F); // Latin Extended-C (Aja, Basaa)

    // Asian languages (Cyrillic and Latin)
    ADD_RANGE(0xA640, 0xA69F); // Cyrillic Extended-B (Kazakh, Chuvash)

    // Additional characters
    ADD_RANGE(0x300, 0x36F);  // Combining Diacritical Marks
    ADD_RANGE(0x1DC0, 0x1DFF); // Combining Diacritical Marks Supplement

    #undef ADD_RANGE

    Font font = {0};

    // Try to load universal font

    font = LoadFontEx("resources/Yulong-Regular.otf", FONT_SIZE, codepoints, count);    


    free(codepoints);

    if (font.texture.id == 0) {
        font = GetFontDefault();
    }

    return font;
}

void HandleMouseWheel() {
    float wheel_move = GetMouseWheelMove();

    if (wheel_move != 0) {
        if (wheel_move > 0) {
            selected_item = (selected_item - SCROLL_SPEED > 0) ? selected_item - SCROLL_SPEED : 0;
        } else if (wheel_move < 0) {
            selected_item = (selected_item + SCROLL_SPEED < files.count - 1) ? selected_item + SCROLL_SPEED : files.count - 1;
        }
    }
}

const char* GetUserHomeDir() {
    const char *home = getenv("HOME");
    if (home == NULL) {
        home = getenv("USERPROFILE");
    }
    if (home == NULL) {
        home = ".";
    }
    return home;
}

void FreeFileList() {
    for (int i = 0; i < files.count; i++) {
        free(files.items[i]);
    }
    free(files.items);
    files.items = NULL;
    files.count = 0;
}

void LoadDirectory(const char *path) {
    FreeFileList();

    // Save current directory
    strncpy(current_dir, path, sizeof(current_dir) - 1);
    current_dir[sizeof(current_dir) - 1] = '\0';

    // Add parent directory if this is not root
    if (strcmp(path, "/") != 0 && strcmp(path, "C:\\") != 0) {
        files.items = malloc(sizeof(char*));
        files.items[0] = strdup("../");
        files.count = 1;
    }

    DIR *dir = opendir(path);
    if (dir != NULL) {
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            // Skip special directories
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            // Check if this is a directory
            char full_path[1024];
            snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

            struct stat statbuf;
            if (stat(full_path, &statbuf) == 0) {
                if (S_ISDIR(statbuf.st_mode)) {
                    // It's a directory
                    char *item = malloc(strlen(entry->d_name) + 2);
                    sprintf(item, "%s/", entry->d_name);
                    files.items = realloc(files.items, (files.count + 1) * sizeof(char*));
                    files.items[files.count++] = item;
                }
            }
        }
        closedir(dir);
    }

    // Now add files
    dir = opendir(path);
    if (dir != NULL) {
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            // Skip special directories
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            // Check if this is a file
            char full_path[1024];
            snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

            struct stat statbuf;
            if (stat(full_path, &statbuf) == 0) {
                if (!S_ISDIR(statbuf.st_mode)) {
                    // It's a file
                    files.items = realloc(files.items, (files.count + 1) * sizeof(char*));
                    files.items[files.count++] = strdup(entry->d_name);
                }
            }
        }
        closedir(dir);
    }

    scroll_offset = 0;
    selected_item = 0;
}

void DrawInterface() {
    // Header
    DrawTextEx(mlang_font, "File Browser", (Vector2){MARGIN, MARGIN}, FONT_SIZE, 1, DARKGRAY);

    // Current directory
    char dir_text[2048];
    snprintf(dir_text, sizeof(dir_text), "Directory: %s", current_dir);
    DrawTextEx(mlang_font, dir_text, (Vector2){MARGIN, MARGIN + 40}, FONT_SIZE, 1, GRAY);

    // File list area
    file_list_area = (Rectangle){MARGIN, MARGIN + 80, SCREEN_WIDTH - 2*MARGIN, SCREEN_HEIGHT - 160};
    DrawRectangleLinesEx(file_list_area, 1, GRAY);

    int max_visible_items = (int)(file_list_area.height / ITEM_HEIGHT);

    // Adjust scroll offset
    if (selected_item < scroll_offset) {
        scroll_offset = selected_item;
    }
    if (selected_item >= scroll_offset + max_visible_items) {
        scroll_offset = selected_item - max_visible_items + 1;
    }

    // Draw visible items
    for (int i = 0; i < max_visible_items && (scroll_offset + i) < files.count; i++) {
        int y_pos = (int)file_list_area.y + i * ITEM_HEIGHT;
        const char *filename = files.items[scroll_offset + i];

        // Alternate background colors
        Color bg_color = ((scroll_offset + i) % 2 == 0) ?
            Fade(LIGHTGRAY, 0.5f) : Fade(LIGHTGRAY, 0.3f);

        if ((scroll_offset + i) == selected_item) {
            // Selected item
            DrawRectangle((int)file_list_area.x, y_pos, (int)file_list_area.width, ITEM_HEIGHT, BLUE);

            char display_text[1024];
            if (filename[strlen(filename)-1] == '/') {
                if (strcmp(filename, "../") == 0) {
                    snprintf(display_text, sizeof(display_text), "[..]");
                } else {
                    snprintf(display_text, sizeof(display_text), "[%.*s]", (int)strlen(filename)-1, filename);
                }
                DrawTextEx(mlang_font, display_text, (Vector2){(int)file_list_area.x + 5, y_pos + 5}, FONT_SIZE, 1, WHITE);
            } else {
                DrawTextEx(mlang_font, filename, (Vector2){(int)file_list_area.x + 5, y_pos + 5}, FONT_SIZE, 1, WHITE);
            }
        } else {
            // Non-selected item
            DrawRectangle((int)file_list_area.x, y_pos, (int)file_list_area.width, ITEM_HEIGHT, bg_color);

            char display_text[1024];
            if (filename[strlen(filename)-1] == '/') {
                if (strcmp(filename, "../") == 0) {
                    snprintf(display_text, sizeof(display_text), "[..]");
                    DrawTextEx(mlang_font, display_text, (Vector2){(int)file_list_area.x + 5, y_pos + 5}, FONT_SIZE, 1, DARKBLUE);
                } else {
                    snprintf(display_text, sizeof(display_text), "[%.*s]", (int)strlen(filename)-1, filename);
                    DrawTextEx(mlang_font, display_text, (Vector2){(int)file_list_area.x + 5, y_pos + 5}, FONT_SIZE, 1, DARKGREEN);
                }
            } else {
                DrawTextEx(mlang_font, filename, (Vector2){(int)file_list_area.x + 5, y_pos + 5}, FONT_SIZE, 1, DARKGRAY);
            }
        }
    }

    // File information
    if (files.count > 0) {
        char file_info[64];
        snprintf(file_info, sizeof(file_info), "%d/%d", selected_item + 1, files.count);
        DrawTextEx(mlang_font, file_info, (Vector2){SCREEN_WIDTH - 50, SCREEN_HEIGHT - 30}, FONT_SIZE, 1, LIGHTGRAY);
    }

    // Help text
    DrawTextEx(mlang_font, "Arrows/PgUp-PgDn:Navigate  Enter:Open  Backspace:Back",
        (Vector2){MARGIN, SCREEN_HEIGHT - 30}, FONT_SIZE, 1, GRAY);
}

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "File Browser");
    SetTargetFPS(60);

    // Load font with Cyrillic support
    mlang_font = LoadUnicodeFont();
    SetTextureFilter(mlang_font.texture, TEXTURE_FILTER_TRILINEAR);

    // Load initial directory
    LoadDirectory(GetUserHomeDir());

    while (!WindowShouldClose()) {
        HandleMouseWheel();

        // Handle navigation keys
        if (IsKeyPressed(KEY_DOWN) && selected_item < files.count - 1) {
            selected_item++;
        }

        if (IsKeyPressed(KEY_UP) && selected_item > 0) {
            selected_item--;
        }

        if (IsKeyPressed(KEY_PAGE_DOWN)) {
            selected_item = (selected_item + MAX_VISIBLE_ITEMS < files.count - 1) ?
                selected_item + MAX_VISIBLE_ITEMS : files.count - 1;
        }

        if (IsKeyPressed(KEY_PAGE_UP)) {
            selected_item = (selected_item - MAX_VISIBLE_ITEMS > 0) ?
                selected_item - MAX_VISIBLE_ITEMS : 0;
        }

        // Handle Enter (open file/directory)
        if (IsKeyPressed(KEY_ENTER) && files.count > 0) {
            const char *selected = files.items[selected_item];
            if (selected[strlen(selected)-1] == '/') {
                if (strcmp(selected, "../") == 0) {
                    // Go to parent directory
                    char *last_slash = strrchr(current_dir, '/');
                    if (last_slash != NULL) {
                        *last_slash = '\0';
                        if (strlen(current_dir) == 0) strcpy(current_dir, "/");
                        LoadDirectory(current_dir);
                    }
                } else {
                    // Go to subdirectory
                    char new_path[2048];
                    snprintf(new_path, sizeof(new_path), "%s/%.*s", current_dir, (int)strlen(selected)-1, selected);
                    LoadDirectory(new_path);
                }
            } else {
                // File selected
                TraceLog(LOG_INFO, TextFormat("Selected: %s/%s", current_dir, selected));
            }
        }

        // Handle Backspace (go back)
        if (IsKeyPressed(KEY_BACKSPACE)) {
            char *last_slash = strrchr(current_dir, '/');
            if (last_slash != NULL) {
                *last_slash = '\0';
                if (strlen(current_dir) == 0) strcpy(current_dir, "/");
                LoadDirectory(current_dir);
            }
        }

        // Refresh with F5
        if (IsKeyPressed(KEY_F5)) {
            LoadDirectory(current_dir);
        }

        // Drawing
        BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawInterface();
        EndDrawing();
    }

    // Cleanup
    FreeFileList();
    UnloadFont(mlang_font);
    CloseWindow();

    return 0;
}
