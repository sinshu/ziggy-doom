// SPDX-License-Identifier: GPL-2.0-or-later
// Integration smoke test: the real engine, IWAD, raylib texture and input contract.
#define main backend_main
#define DG_DrawFrame backend_draw_frame
#define DG_GetKey backend_get_key
#include "../src/main.c"
#undef main
#undef DG_DrawFrame
#undef DG_GetKey
#include "doomstat.h"
#include "doomdef.h"
#include "i_sound.h"
#include "sounds.h"
#include "m_argv.h"
#include <assert.h>
#include "music_checks.h"

static int frames, start_tic;
static bool ready;
static fixed_t start_x, start_y;

int DG_GetKey(int *pressed, unsigned char *key)
{
    memset(wanted, 0, sizeof(wanted));
    wanted[KEY_UPARROW] = ready && gametic - start_tic >= 10 && gametic - start_tic < 35;
    wanted[KEY_FIRE] = ready && gametic - start_tic >= 40 && gametic - start_tic < 65;
    return backend_get_key(pressed, key);
}

void DG_DrawFrame(void)
{
    backend_draw_frame();
    ++frames;
    if (!ready) return;
    if (gametic - start_tic >= 90) {
        assert(players[consoleplayer].mo->x != start_x || players[consoleplayer].mo->y != start_y);
        assert(players[consoleplayer].ammo[am_clip] < 50);
        if (!DirectoryExists(".tmp")) MakeDirectory(".tmp");
        Image frame = LoadImageFromTexture(screen);
        assert(ExportImage(frame, ".tmp/doom-smoke.png"));
        UnloadImage(frame);
        printf("SMOKE PASS: 90 game tics, rendered frames, movement and shooting; screenshot .tmp/doom-smoke.png\n");
        I_Quit();
    }
}

int main(int argc, char **argv)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    puts("SMOKE: starting engine");
    doomgeneric_Create(argc, argv);
    puts("SMOKE: engine initialized");
    if (M_CheckParm("-musiccheck")) CheckMusic();
    if (M_CheckParm("-nomusic") || M_CheckParm("-nosound") || M_CheckParm("-expectnomusic"))
        assert(!I_MusicIsPlaying());
    if (M_CheckParm("-nosound") || (M_CheckParm("-nosfx") && M_CheckParm("-nomusic")))
        assert(!IsAudioDeviceReady());
    if (M_CheckParm("-soundcheck")) {
        assert(IsAudioDeviceReady());
        assert(I_StartSound(&S_sfx[sfx_sawful], 0, 127, 0) == 0);
        assert(I_StartSound(&S_sfx[sfx_sawful], 1, 127, 254) == 1);
        assert(I_SoundIsPlaying(0) && I_SoundIsPlaying(1));
        I_UpdateSoundParams(0, 32, 254);
        assert(I_SoundIsPlaying(0) && I_SoundIsPlaying(1));
        I_StopSound(0);
        assert(!I_SoundIsPlaying(0) && I_SoundIsPlaying(1));
        I_StopSound(1);
        puts("SMOKE AUDIO PASS: real WAD, audio device, simultaneous aliases, live update and independent stop");
    }
    start_tic = gametic; ready = true;
    start_x = players[consoleplayer].mo->x; start_y = players[consoleplayer].mo->y;
    for (;;) { poll_input(); doomgeneric_Tick(); }
}

