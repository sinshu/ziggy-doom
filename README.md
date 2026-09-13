# ziggy-doom

A Doom port built with Zig, using doomgeneric, raylib, and ZiggySynth for graphics, input, sound effects, and MIDI music.

## Requirements

- Zig 0.16.0 (tested on Windows x86_64).
- A Doom IWAD: `doom1.wad`, `doom.wad`, or `doom2.wad`.
- Optional: `TimGM6mb.sf2` in the project root for music.

Game data and SoundFonts are not included. The first build downloads raylib.

## Run

```sh
zig build run -- -iwad doom2.wad
```

Use `-soundfont path.sf2` for another SoundFont, or `-nomusic` to disable music. Build without running with `zig build`.

## Controls

W/S: move · A/D: strafe · Left/Right: turn · Ctrl: fire · E/Space: use · Shift: run · 1–7: weapons · Esc: menu · Tab: map.

## License

GPL-2.0-or-later; see [LICENSE](LICENSE). raylib uses the zlib license; ZiggySynth uses MIT. Third-party notices are in `vendor/`.
