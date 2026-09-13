// SPDX-License-Identifier: GPL-2.0-or-later
#include "raylib.h"
#include "audio_device.h"
static unsigned int users;
bool DG_AcquireAudio(void)
{
    if (!users) {
        InitAudioDevice();
        if (!IsAudioDeviceReady()) return false;
    }
    ++users;
    return true;
}
void DG_ReleaseAudio(void)
{
    if (users && --users == 0) CloseAudioDevice();
}
