// SPDX-License-Identifier: GPL-2.0-or-later
#include "raylib.h"

// Preserve raylib enum values before doomkeys.h defines same-named macros.
enum {
    RL_ENTER = KEY_ENTER, RL_ESCAPE = KEY_ESCAPE, RL_TAB = KEY_TAB,
    RL_BACKSPACE = KEY_BACKSPACE, RL_PAUSE = KEY_PAUSE,
    RL_F1 = KEY_F1, RL_F2 = KEY_F2, RL_F3 = KEY_F3, RL_F4 = KEY_F4,
    RL_F5 = KEY_F5, RL_F6 = KEY_F6, RL_F7 = KEY_F7, RL_F8 = KEY_F8,
    RL_F9 = KEY_F9, RL_F10 = KEY_F10, RL_F11 = KEY_F11, RL_F12 = KEY_F12
};
#include "doomgeneric.h"
#include "doomkeys.h"
#include "i_system.h"
#include <stdio.h>
#include <string.h>

static Texture2D screen;
static Color pixels[DOOMGENERIC_RESX * DOOMGENERIC_RESY];
static bool wanted[256], delivered[256];
typedef struct { int ray; unsigned char doom; } KeyBinding;
static const KeyBinding bindings[] = {
    { KEY_UP, KEY_UPARROW }, { KEY_W, KEY_UPARROW },
    { KEY_DOWN, KEY_DOWNARROW }, { KEY_S, KEY_DOWNARROW },
    { KEY_LEFT, KEY_LEFTARROW }, { KEY_RIGHT, KEY_RIGHTARROW },
    { KEY_A, KEY_STRAFE_L }, { KEY_D, KEY_STRAFE_R },
    { KEY_COMMA, KEY_STRAFE_L }, { KEY_PERIOD, KEY_STRAFE_R },
    { KEY_LEFT_CONTROL, KEY_FIRE }, { KEY_RIGHT_CONTROL, KEY_FIRE },
    { KEY_SPACE, KEY_USE }, { KEY_E, KEY_USE },
    { KEY_LEFT_SHIFT, KEY_RSHIFT }, { KEY_RIGHT_SHIFT, KEY_RSHIFT },
    { KEY_LEFT_ALT, KEY_RALT }, { KEY_RIGHT_ALT, KEY_RALT },
    { RL_ENTER, KEY_ENTER }, { RL_ESCAPE, KEY_ESCAPE }, { RL_TAB, KEY_TAB },
    { RL_BACKSPACE, KEY_BACKSPACE }, { RL_PAUSE, KEY_PAUSE },
    { KEY_EQUAL, KEY_EQUALS }, { KEY_MINUS, KEY_MINUS },
    { RL_F1, KEY_F1 }, { RL_F2, KEY_F2 }, { RL_F3, KEY_F3 }, { RL_F4, KEY_F4 },
    { RL_F5, KEY_F5 }, { RL_F6, KEY_F6 }, { RL_F7, KEY_F7 }, { RL_F8, KEY_F8 },
    { RL_F9, KEY_F9 }, { RL_F10, KEY_F10 }, { RL_F11, KEY_F11 }, { RL_F12, KEY_F12 }
};

static void cleanup(void)
{
    if (IsWindowReady()) { UnloadTexture(screen); CloseWindow(); }
    free(DG_ScreenBuffer);
    DG_ScreenBuffer = NULL;
}

static void poll_input(void)
{
    // Pump events even while the engine waits for its next 35 Hz tic.
    PollInputEvents();
    if (WindowShouldClose()) I_Quit();
    memset(wanted, 0, sizeof(wanted));
    if (!IsWindowFocused()) return; // Release held keys on focus loss.
    for (int key = KEY_A; key <= KEY_Z; ++key)
        wanted['a' + key - KEY_A] = IsKeyDown(key);
    for (int key = KEY_ZERO; key <= KEY_NINE; ++key)
        wanted['0' + key - KEY_ZERO] = IsKeyDown(key);
    for (size_t i = 0; i < sizeof(bindings) / sizeof(bindings[0]); ++i)
        wanted[bindings[i].doom] |= IsKeyDown(bindings[i].ray);
}

void DG_Init(void)
{
    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(960, 720, "ziggy-doom");
    if (!IsWindowReady()) { fputs("Could not open a raylib window.\n", stderr); exit(1); }
    atexit(cleanup);
    SetWindowMinSize(320, 240);
    SetExitKey(KEY_NULL); // Escape belongs to Doom's menu.
    Image image = { pixels, DOOMGENERIC_RESX, DOOMGENERIC_RESY, 1,
                    PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 };
    screen = LoadTextureFromImage(image);
    SetTextureFilter(screen, TEXTURE_FILTER_POINT);
}

void DG_DrawFrame(void)
{
    poll_input();
    // doomgeneric produces 0x00RRGGBB; raylib expects RGBA bytes.
    for (size_t i = 0; i < sizeof(pixels) / sizeof(pixels[0]); ++i) {
        uint32_t p = DG_ScreenBuffer[i];
        pixels[i] = (Color){ (p >> 16) & 255, (p >> 8) & 255, p & 255, 255 };
    }
    UpdateTexture(screen, pixels);
    // Correct original 320x200 pixels to a 4:3 display, with letterboxing.
    float width = (float)GetScreenWidth(), height = width * 3.0f / 4.0f;
    if (height > GetScreenHeight()) { height = (float)GetScreenHeight(); width = height * 4.0f / 3.0f; }
    Rectangle dest = { (GetScreenWidth() - width) / 2, (GetScreenHeight() - height) / 2, width, height };
    BeginDrawing();
    ClearBackground(BLACK);
    DrawTexturePro(screen, (Rectangle){0, 0, DOOMGENERIC_RESX, DOOMGENERIC_RESY}, dest, (Vector2){0, 0}, 0, WHITE);
    EndDrawing();
}

void DG_SleepMs(uint32_t ms) { poll_input(); WaitTime(ms / 1000.0); }
uint32_t DG_GetTicksMs(void) { return (uint32_t)((uint64_t)(GetTime() * 1000.0)); }

int DG_GetKey(int *pressed, unsigned char *key)
{
    // Merge aliases so releasing one Ctrl key cannot cancel the other.
    for (int i = 0; i < 256; ++i) {
        if (wanted[i] != delivered[i]) {
            *pressed = delivered[i] = wanted[i];
            *key = (unsigned char)i;
            return 1;
        }
    }
    return 0;
}

void DG_SetWindowTitle(const char *title) { SetWindowTitle(title); }

int main(int argc, char **argv)
{
    doomgeneric_Create(argc, argv);
    for (;;) { poll_input(); doomgeneric_Tick(); }
}
