// SPDX-License-Identifier: GPL-2.0-or-later
#include "raylib.h"
#include "audio_device.h"
#include "music_synth.h"
#include "i_sound.h"
#include "m_argv.h"
#include "../vendor/chocolate-doom/mus2mid.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MUSIC_FRAMES 2048
typedef struct Song { void *midi; struct Song *next; } Song;
static Song *songs, *current;
static AudioStream stream;
static bool initialized, playing, paused;
static float volume = 1;

static void Stop(void)
{
    if (!initialized) return;
    StopAudioStream(stream);
    ZS_Stop();
    current = NULL;
    playing = paused = false;
}
static void Unregister(void *handle)
{
    Song **entry = &songs;
    while (*entry && *entry != handle) entry = &(*entry)->next;
    if (!*entry) return;
    Song *song = *entry;
    if (current == song) Stop();
    *entry = song->next;
    ZS_Unregister(song->midi);
    free(song);
}
static void Shutdown(void)
{
    if (!initialized) return;
    Stop();
    while (songs) Unregister(songs);
    UnloadAudioStream(stream);
    ZS_Shutdown();
    DG_ReleaseAudio();
    initialized = false;
}
static boolean Init(void)
{
    if (initialized) return true;
    int arg = M_CheckParmWithArgs("-soundfont", 1);
    const char *path = arg ? myargv[arg + 1] : "TimGM6mb.sf2";
    int size = 0;
    unsigned char *data = LoadFileData(path, &size);
    bool loaded = data && size > 0 && ZS_Init(data, (size_t)size);
    UnloadFileData(data);
    if (!loaded) {
        fprintf(stderr, "Could not load SoundFont '%s'; BGM disabled (use -soundfont path.sf2).\n", path);
        return false;
    }
    if (!DG_AcquireAudio()) { ZS_Shutdown(); return false; }
    SetAudioStreamBufferSizeDefault(MUSIC_FRAMES);
    stream = LoadAudioStream(44100, 32, 2);
    if (!IsAudioStreamValid(stream)) {
        ZS_Shutdown(); DG_ReleaseAudio(); return false;
    }
    initialized = true;
    SetAudioStreamVolume(stream, volume);
    atexit(Shutdown);
    return true;
}
static void *Register(void *data, int length)
{
    if (!initialized || !data || length < 4) return NULL;
    void *midi = NULL;
    if (!memcmp(data, "MUS\x1a", 4)) {
        MEMFILE *input = mem_fopen_read(data, (size_t)length);
        MEMFILE *output = mem_fopen_write();
        if (!mus2mid(input, output)) {
            void *bytes; size_t size;
            mem_get_buf(output, &bytes, &size);
            midi = ZS_Register(bytes, size);
        }
        mem_fclose(input); mem_fclose(output);
    } else if (!memcmp(data, "MThd", 4)) {
        midi = ZS_Register(data, (size_t)length);
    }
    if (!midi) { fputs("Could not decode MUS/MIDI song.\n", stderr); return NULL; }
    Song *song = malloc(sizeof(*song));
    if (!song) { ZS_Unregister(midi); return NULL; }
    *song = (Song){ midi, songs };
    songs = song;
    return song;
}
void DG_PollMusic(void)
{
    if (!initialized || !playing || paused) return;
    // Main-thread synthesis: no WAD, allocator or sequencer access on audio thread.
    for (int i = 0; i < 2 && IsAudioStreamProcessed(stream); ++i) {
        if (ZS_Ended()) { Stop(); return; }
        float pcm[MUSIC_FRAMES * 2];
        ZS_Render(pcm, MUSIC_FRAMES);
        UpdateAudioStream(stream, pcm, MUSIC_FRAMES);
    }
}
static void Play(void *handle, boolean loop)
{
    Stop();
    if (!initialized || !handle) return;
    Song *song = songs;
    while (song && song != handle) song = song->next;
    if (!song) return;
    current = song;
    ZS_Play(song->midi, loop != 0);
    playing = true;
    DG_PollMusic();
    PlayAudioStream(stream);
}
static void SetVolume(int value)
{
    if (value < 0) value = 0;
    if (value > 127) value = 127;
    volume = value / 127.0f;
    if (initialized) SetAudioStreamVolume(stream, volume);
}
static void Pause(void)
{
    if (initialized && playing && !paused) { PauseAudioStream(stream); paused = true; }
}
static void Resume(void)
{
    if (initialized && playing && paused) { ResumeAudioStream(stream); paused = false; }
}
static boolean IsPlaying(void) { return initialized && playing; }
music_module_t DG_music_module = {
    .Init = Init, .Shutdown = Shutdown, .SetMusicVolume = SetVolume,
    .PauseMusic = Pause, .ResumeMusic = Resume, .RegisterSong = Register,
    .UnRegisterSong = Unregister, .PlaySong = Play, .StopSong = Stop,
    .MusicIsPlaying = IsPlaying, .Poll = DG_PollMusic
};
