// SPDX-License-Identifier: GPL-2.0-or-later
// Keep this include order: it previously made MinGW's CRT wrappers recursive.
#include "doomtype.h"
#include <string.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    const char *name = argc > 1 ? argv[1] : "SW1BRCOM";
    const char lump[8] = {'S', 'W', '1', 'B', 'R', 'C', 'O', 'M'};
    if (strncasecmp(name, "sw1brcom", 8) != 0
        || strcasecmp(name, "sw1brcom") != 0
        || strncasecmp(lump, "sw1brcom", sizeof(lump)) != 0
        || strncasecmp(name, "SW2BRCOM", 8) >= 0
        || strcasecmp(name, "SW0BRCOM") <= 0
        || strncasecmp(name, "", 0) != 0)
        return 1;
    puts("STRING COMPARE PASS");
    return 0;
}
