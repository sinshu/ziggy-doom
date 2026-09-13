// SPDX-License-Identifier: GPL-2.0-or-later
#include "s_sound.h"
#include "w_wad.h"
#include "z_zone.h"
#include <math.h>

static void CheckMusic(void)
{
    // Standard MIDI: piano C4 for half a second, then end of track.
    unsigned char midi[] = {
        'M','T','h','d',0,0,0,6,0,0,0,1,0,96,
        'M','T','r','k',0,0,0,15,
        0,0xc0,0, 0,0x90,60,100, 96,0x80,60,0, 0,0xff,0x2f,0
    };
    S_StopMusic();
    assert(I_RegisterSong("bad!", 4) == NULL);
    assert(I_RegisterSong("MUS\x1a", 4) == NULL);
    assert(I_RegisterSong("MThd", 4) == NULL);
    void *short_song = I_RegisterSong(midi, sizeof(midi));
    assert(short_song);
    I_PlaySong(short_song, false);
    assert(I_MusicIsPlaying());
    float pcm[2048];
    double energy = 0;
    for (int block = 0; block < 140; ++block) {
        ZS_Render(pcm, 1024);
        for (int i = 0; i < 2048; ++i) {
            assert(isfinite(pcm[i]));
            energy += fabs(pcm[i]);
        }
    }
    assert(energy > 1 && ZS_Ended());
    I_StopSong();
    assert(!I_MusicIsPlaying());
    I_PlaySong(short_song, true);
    for (int block = 0; block < 140; ++block) ZS_Render(pcm, 1024);
    assert(!ZS_Ended() && I_MusicIsPlaying());
    I_PauseSong(); DG_PollMusic(); I_ResumeSong();
    I_SetMusicVolume(0); I_SetMusicVolume(127);
    I_UnRegisterSong(short_song); // Unregistering the active song must stop it.
    assert(!I_MusicIsPlaying());

    int lump = W_GetNumForName("D_RUNNIN");
    void *data = W_CacheLumpNum(lump, PU_STATIC);
    int length = W_LumpLength(lump);
    void *first = I_RegisterSong(data, length);
    void *second = I_RegisterSong(data, length);
    assert(first && second);
    W_ReleaseLumpNum(lump); // Handles own parsed MIDI; no borrowed WAD data.
    I_PlaySong(first, true);
    float reference[2048];
    ZS_Render(reference, 1024);
    I_PlaySong(second, true);
    ZS_Render(pcm, 1024);
    assert(memcmp(reference, pcm, sizeof(pcm)) == 0);
    energy = 0;
    for (int block = 0; block < 86; ++block) {
        ZS_Render(pcm, 1024);
        for (int i = 0; i < 2048; ++i) {
            assert(isfinite(pcm[i]));
            energy += fabs(pcm[i]);
        }
    }
    assert(energy > 1);
    I_UnRegisterSong(first); I_UnRegisterSong(second);
    S_ChangeMusic(mus_runnin, true);
    assert(I_MusicIsPlaying());
    puts("SMOKE MUSIC PASS: SoundFont synthesis, MUS/MIDI, nonzero PCM, repeat conversion, loop/end, pause/resume, volume, song switch");
}
