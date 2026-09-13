// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef ZIGGY_DMX_SOUND_H
#define ZIGGY_DMX_SOUND_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    const uint8_t *samples;
    uint32_t count;
    unsigned int rate;
} DmxSound;

// DMX format 3: little-endian header followed by unsigned 8-bit mono PCM.
static bool DecodeDmxSound(const uint8_t *data, size_t size, DmxSound *out)
{
    if (!data || size < 8 || data[0] != 3 || data[1] != 0) return false;
    unsigned int rate = (unsigned int)data[2] | ((unsigned int)data[3] << 8);
    uint32_t count = (uint32_t)data[4] | ((uint32_t)data[5] << 8)
                   | ((uint32_t)data[6] << 16) | ((uint32_t)data[7] << 24);
    if (!rate || count <= 48 || count > size - 8) return false;
    // Match doomgeneric's SDL backend: DMX skips 16 samples at each end.
    *out = (DmxSound){ data + 24, count - 32, rate };
    return true;
}

#endif
