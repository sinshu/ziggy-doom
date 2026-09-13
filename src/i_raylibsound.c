// SPDX-License-Identifier: GPL-2.0-or-later
#include "raylib.h"
#include "dmx_sound.h"
#include "i_sound.h"
#include "w_wad.h"
#include "z_zone.h"
#include <stdlib.h>
#include <stdio.h>

extern int snd_channels;

typedef struct CachedSound {
    int lump;
    Sound sound;
    struct CachedSound *next;
} CachedSound;

typedef struct {
    Sound sound;
    bool active;
} SoundChannel;

static CachedSound *cache;
static SoundChannel *channels;
static int channel_count;
static bool initialized, sfx_prefix;

static bool ValidChannel(int channel)
{
    return initialized && channel >= 0 && channel < channel_count;
}

static void StopChannel(int channel)
{
    if (!ValidChannel(channel) || !channels[channel].active) return;
    StopSound(channels[channel].sound);
    UnloadSoundAlias(channels[channel].sound);
    channels[channel] = (SoundChannel){0};
}

static void Shutdown(void)
{
    if (!initialized) return;
    for (int i = 0; i < channel_count; ++i) StopChannel(i);
    while (cache) {
        CachedSound *next = cache->next;
        UnloadSound(cache->sound);
        free(cache);
        cache = next;
    }
    free(channels);
    channels = NULL;
    channel_count = 0;
    CloseAudioDevice();
    initialized = false;
}

static boolean Init(boolean use_prefix)
{
    if (initialized) return true;
    if (snd_channels <= 0) return false;
    channels = calloc((size_t)snd_channels, sizeof(*channels));
    if (!channels) return false;
    InitAudioDevice();
    if (!IsAudioDeviceReady()) {
        free(channels);
        channels = NULL;
        fputs("Audio device unavailable; continuing without sound effects.\n", stderr);
        return false;
    }
    channel_count = snd_channels;
    sfx_prefix = use_prefix;
    initialized = true;
    // Also cover exits before S_Init registers the engine shutdown callback.
    atexit(Shutdown);
    return true;
}

static int GetSfxLumpNum(sfxinfo_t *sfx)
{
    if (!sfx) return -1;
    if (sfx->link) sfx = sfx->link;
    char name[9];
    snprintf(name, sizeof(name), "%s%.*s", sfx_prefix ? "ds" : "",
             sfx_prefix ? 6 : 8, sfx->name);
    return W_CheckNumForName(name);
}

static CachedSound *CacheSound(int lump)
{
    if (lump < 0 || (unsigned int)lump >= numlumps) return NULL;
    for (CachedSound *entry = cache; entry; entry = entry->next)
        if (entry->lump == lump) return entry;

    int size = W_LumpLength((unsigned int)lump);
    if (size < 8) return NULL;
    const uint8_t *data = W_CacheLumpNum(lump, PU_STATIC);
    DmxSound pcm;
    Sound sound = {0};
    if (DecodeDmxSound(data, (size_t)size, &pcm)) {
        Wave wave = { .frameCount = pcm.count, .sampleRate = pcm.rate,
                      .sampleSize = 8, .channels = 1, .data = (void *)pcm.samples };
        sound = LoadSoundFromWave(wave);
    }
    // LoadSoundFromWave copies PCM. The borrowed Wave must not be unloaded.
    W_ReleaseLumpNum(lump);
    if (!IsSoundValid(sound)) {
        UnloadSound(sound);
        return NULL;
    }
    CachedSound *entry = malloc(sizeof(*entry));
    if (!entry) {
        UnloadSound(sound);
        return NULL;
    }
    *entry = (CachedSound){ lump, sound, cache };
    cache = entry;
    return entry;
}

static void UpdateSoundParams(int channel, int vol, int sep)
{
    if (!ValidChannel(channel) || !channels[channel].active) return;
    if (vol < 0) vol = 0;
    if (vol > 127) vol = 127;
    if (sep < 0) sep = 0;
    if (sep > 254) sep = 254;
    SetSoundVolume(channels[channel].sound, vol / 127.0f);
    SetSoundPan(channels[channel].sound, sep / 127.0f - 1.0f);
}

static int StartSound(sfxinfo_t *sfx, int channel, int vol, int sep)
{
    if (!ValidChannel(channel)) return -1;
    StopChannel(channel);
    CachedSound *entry = CacheSound(GetSfxLumpNum(sfx));
    if (!entry) return -1;
    Sound alias = LoadSoundAlias(entry->sound);
    if (!IsSoundValid(alias)) {
        UnloadSoundAlias(alias);
        return -1;
    }
    channels[channel] = (SoundChannel){ alias, true };
    UpdateSoundParams(channel, vol, sep);
    PlaySound(alias);
    return channel;
}

static boolean SoundIsPlaying(int channel)
{
    return ValidChannel(channel) && channels[channel].active
        && IsSoundPlaying(channels[channel].sound);
}

static void Update(void)
{
    for (int i = 0; i < channel_count; ++i)
        if (channels[i].active && !SoundIsPlaying(i)) StopChannel(i);
}

static void CacheSounds(sfxinfo_t *sounds, int count)
{
    if (!initialized) return;
    for (int i = 1; i < count; ++i) CacheSound(GetSfxLumpNum(&sounds[i]));
}

static snddevice_t devices[] = {
    SNDDEVICE_SB, SNDDEVICE_PAS, SNDDEVICE_GUS,
    SNDDEVICE_WAVEBLASTER, SNDDEVICE_SOUNDCANVAS, SNDDEVICE_AWE32
};

sound_module_t DG_sound_module = {
    devices, sizeof(devices) / sizeof(devices[0]),
    Init, Shutdown, GetSfxLumpNum, Update, UpdateSoundParams,
    StartSound, StopChannel, SoundIsPlaying, CacheSounds
};
