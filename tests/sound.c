// SPDX-License-Identifier: GPL-2.0-or-later
// Exercise the real backend with a deterministic audio device and WAD substitute.
#include "../src/i_raylibsound.c"
#include <assert.h>
#include <string.h>

int snd_channels = 20;
unsigned int numlumps = 1;
static uint8_t lump_data[72];
static bool device_available = true, device_ready;
static int releases, sources, aliases;

typedef struct {
    bool playing, alias;
    float volume, pan;
    const void *shared;
} FakeBuffer;

static FakeBuffer *Buffer(Sound sound) { return (FakeBuffer *)sound.stream.buffer; }
void InitAudioDevice(void) { device_ready = device_available; }
bool IsAudioDeviceReady(void) { return device_ready; }
void CloseAudioDevice(void) { assert(!sources && !aliases); device_ready = false; }
Sound LoadSoundFromWave(Wave wave)
{
    assert(wave.sampleSize == 8 && wave.channels == 1 && wave.sampleRate == 11025);
    assert(wave.frameCount == 32 && wave.data == lump_data + 24);
    FakeBuffer *buffer = calloc(1, sizeof(*buffer));
    assert(buffer);
    buffer->shared = buffer;
    ++sources;
    Sound sound = {0};
    sound.frameCount = wave.frameCount;
    sound.stream.buffer = (void *)buffer;
    return sound;
}
bool IsSoundValid(Sound sound) { return sound.stream.buffer != NULL; }
Sound LoadSoundAlias(Sound source)
{
    FakeBuffer *buffer = calloc(1, sizeof(*buffer));
    assert(buffer);
    buffer->alias = true;
    buffer->shared = Buffer(source)->shared;
    ++aliases;
    source.stream.buffer = (void *)buffer;
    return source;
}
void UnloadSound(Sound sound)
{
    if (!Buffer(sound)) return;
    assert(!Buffer(sound)->alias && aliases == 0);
    --sources;
    free(Buffer(sound));
}
void UnloadSoundAlias(Sound sound)
{
    if (!Buffer(sound)) return;
    assert(Buffer(sound)->alias && !Buffer(sound)->playing);
    --aliases;
    free(Buffer(sound));
}
void PlaySound(Sound sound) { Buffer(sound)->playing = true; }
void StopSound(Sound sound) { Buffer(sound)->playing = false; }
bool IsSoundPlaying(Sound sound) { return Buffer(sound)->playing; }
void SetSoundVolume(Sound sound, float volume) { Buffer(sound)->volume = volume; }
void SetSoundPan(Sound sound, float pan) { Buffer(sound)->pan = pan; }
int W_CheckNumForName(char *name) { return strcmp(name, "dstest") == 0 ? 0 : -1; }
int W_LumpLength(unsigned int lump) { assert(lump == 0); return sizeof(lump_data); }
void *W_CacheLumpNum(int lump, int tag) { assert(lump == 0 && tag == PU_STATIC); return lump_data; }
void W_ReleaseLumpNum(int lump) { assert(lump == 0); ++releases; }

int main(void)
{
    lump_data[0] = 3;
    lump_data[2] = 0x11; lump_data[3] = 0x2b; // 11025 Hz
    lump_data[4] = 64;
    DmxSound pcm;
    assert(DecodeDmxSound(lump_data, sizeof(lump_data), &pcm));
    assert(pcm.samples == lump_data + 24 && pcm.count == 32 && pcm.rate == 11025);
    assert(!DecodeDmxSound(NULL, 0, &pcm));
    assert(!DecodeDmxSound(lump_data, 7, &pcm));
    assert(!DecodeDmxSound(lump_data, 71, &pcm));
    lump_data[0] = 0; assert(!DecodeDmxSound(lump_data, 72, &pcm)); lump_data[0] = 3;
    lump_data[4] = 48; assert(!DecodeDmxSound(lump_data, 72, &pcm));
    lump_data[4] = 49; assert(DecodeDmxSound(lump_data, 72, &pcm)); lump_data[4] = 64;
    lump_data[7] = 0x80; assert(!DecodeDmxSound(lump_data, 72, &pcm)); lump_data[7] = 0;
    lump_data[2] = lump_data[3] = 0; assert(!DecodeDmxSound(lump_data, 72, &pcm));
    lump_data[2] = 0x11; lump_data[3] = 0x2b;

    device_available = false;
    assert(!Init(true));
    Shutdown();
    assert(StartSound(NULL, 0, 127, 127) == -1);
    device_available = true;
    assert(Init(true));
    sfxinfo_t sfx = { .name = "test" }, linked = { .link = &sfx };
    assert(StartSound(&sfx, -1, 127, 127) == -1);
    assert(StartSound(&sfx, 20, 127, 127) == -1);
    assert(StartSound(&sfx, 0, 127, 0) == 0);
    assert(StartSound(&linked, 19, 127, 254) == 19);
    assert(sources == 1 && aliases == 2 && releases == 1);
    FakeBuffer *first = Buffer(channels[0].sound), *second = Buffer(channels[19].sound);
    assert(first != second && first->shared == second->shared);
    assert(first->pan == -1 && second->pan == 1);
    UpdateSoundParams(0, 32, 127);
    assert(first->playing && first->volume == 32 / 127.0f && first->pan == 0);
    assert(second->playing && second->volume == 1 && second->pan == 1);
    UpdateSoundParams(0, -10, 999);
    assert(first->volume == 0 && first->pan == 1);
    assert(StartSound(&sfx, 0, 127, 127) == 0 && aliases == 2);
    StopChannel(0);
    assert(!SoundIsPlaying(0) && SoundIsPlaying(19) && aliases == 1);
    second->playing = false;
    Update();
    assert(!channels[19].active && aliases == 0);
    assert(StartSound(&sfx, 0, 127, 127) == 0);
    assert(StartSound(&sfx, 19, 127, 127) == 19);
    Shutdown(); Shutdown();
    assert(!sources && !aliases && !device_ready);

    assert(Init(true));
    lump_data[0] = 0;
    assert(StartSound(&sfx, 0, 127, 127) == -1);
    assert(releases == 2 && !sources && !aliases);
    Shutdown();
    puts("SOUND PASS: DMX bounds, shared PCM, independent live pan/volume, channel replacement, cleanup, device failure");
    return 0;
}
