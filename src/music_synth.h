// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef DG_MUSIC_SYNTH_H
#define DG_MUSIC_SYNTH_H
#include <stdbool.h>
#include <stddef.h>
bool ZS_Init(const unsigned char *data, size_t size);
void ZS_Shutdown(void);
void *ZS_Register(const unsigned char *data, size_t size);
void ZS_Unregister(void *song);
void ZS_Play(void *song, bool loop);
void ZS_Stop(void);
void ZS_Render(float *interleaved, size_t frames);
bool ZS_Ended(void);
void DG_PollMusic(void);
#endif
