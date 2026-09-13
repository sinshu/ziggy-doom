# Chocolate Doom MUS to MIDI converter

Source: https://github.com/chocolate-doom/chocolate-doom
Commit: 895f581c5d91497bdda0516612da803fe5843e28
Copied src/mus2mid.c and src/mus2mid.h. License: GPL-2.0-or-later.
COPYING contains the GPL v2 text, copied from this project's LICENSE.

Local compatibility change: define PACKED_STRUCT using doomgeneric's PACKEDATTR.
Reset queued time and channel velocities for each conversion so prior songs or
failed conversions cannot affect the next song.
The converter uses the existing doomgeneric memio implementation.
