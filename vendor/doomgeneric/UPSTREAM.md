# Vendored doomgeneric

Source: https://github.com/ozkl/doomgeneric
Commit: dcb7a8dbc7a16ce3dda29382ac9aae9d77d21284

Copied the top-level .c and .h files from doomgeneric/, plus the upstream LICENSE.
The build uses the engine source list from upstream doomgeneric/Makefile,
replacing doomgeneric_xlib.c with ../../src/main.c. Other platform backends
are retained for reference but are not compiled.

Local source changes:
- i_system.c: always exit after I_Quit runs shutdown callbacks. Upstream only
  called exit with ORIGCODE enabled, so menu Quit returned to the game loop.
Build definitions: DOOMGENERIC_RESX=320, DOOMGENERIC_RESY=200.
FEATURE_SOUND and DG_RAYLIB_SOUND are defined. src/i_raylibsound.c supplies
DG_sound_module; i_sound.c excludes the SDL header, libsamplerate bindings,
and music module registration for this backend. Music remains disabled.
The engine is compiled as GNU C99 with wrapping signed arithmetic and without
undefined-behavior sanitization to accommodate legacy Doom arithmetic.
