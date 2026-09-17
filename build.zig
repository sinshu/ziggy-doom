const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});
    const smoke = b.option(bool, "smoke", "Build the IWAD/render/input integration smoke test") orelse false;
    const mod = b.createModule(.{
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });
    const synth_mod = b.createModule(.{
        .root_source_file = b.path("src/music_synth.zig"),
        .target = target,
        .optimize = .ReleaseSafe,
        .link_libc = true,
    });
    synth_mod.addImport("ziggysynth", b.createModule(.{
        .root_source_file = b.path("vendor/ziggysynth/ziggysynth.zig"),
        .target = target,
        .optimize = .ReleaseSafe,
    }));
    const synth_lib = b.addLibrary(.{ .name = "music-synth", .linkage = .static, .root_module = synth_mod });
    mod.linkLibrary(synth_lib);
    mod.addIncludePath(b.path("vendor/doomgeneric"));
    mod.addCMacro("DOOMGENERIC_RESX", "320");
    mod.addCMacro("DOOMGENERIC_RESY", "200");
    mod.addCMacro("FEATURE_SOUND", "1");
    mod.addCMacro("DG_RAYLIB_SOUND", "1");
    mod.addCSourceFiles(.{
        .files = if (smoke) &.{"tests/smoke.c"} else &.{"src/main.c"},
        .flags = &.{ "-std=c99", "-Wall", "-Wextra" },
    });
    mod.addCSourceFiles(.{
        .files = &.{ "src/i_raylibsound.c", "src/i_raylibmusic.c", "src/audio_device.c", "vendor/chocolate-doom/mus2mid.c" },
        .flags = &.{ "-std=c99", "-Wall", "-Wextra" },
    });
    mod.addCSourceFiles(.{
        .files = &.{
            "vendor/doomgeneric/dummy.c",
            "vendor/doomgeneric/am_map.c",
            "vendor/doomgeneric/doomdef.c",
            "vendor/doomgeneric/doomstat.c",
            "vendor/doomgeneric/dstrings.c",
            "vendor/doomgeneric/d_event.c",
            "vendor/doomgeneric/d_items.c",
            "vendor/doomgeneric/d_iwad.c",
            "vendor/doomgeneric/d_loop.c",
            "vendor/doomgeneric/d_main.c",
            "vendor/doomgeneric/d_mode.c",
            "vendor/doomgeneric/d_net.c",
            "vendor/doomgeneric/f_finale.c",
            "vendor/doomgeneric/f_wipe.c",
            "vendor/doomgeneric/g_game.c",
            "vendor/doomgeneric/hu_lib.c",
            "vendor/doomgeneric/hu_stuff.c",
            "vendor/doomgeneric/info.c",
            "vendor/doomgeneric/i_cdmus.c",
            "vendor/doomgeneric/i_endoom.c",
            "vendor/doomgeneric/i_joystick.c",
            "vendor/doomgeneric/i_scale.c",
            "vendor/doomgeneric/i_sound.c",
            "vendor/doomgeneric/i_system.c",
            "vendor/doomgeneric/i_timer.c",
            "vendor/doomgeneric/memio.c",
            "vendor/doomgeneric/m_argv.c",
            "vendor/doomgeneric/m_bbox.c",
            "vendor/doomgeneric/m_cheat.c",
            "vendor/doomgeneric/m_config.c",
            "vendor/doomgeneric/m_controls.c",
            "vendor/doomgeneric/m_fixed.c",
            "vendor/doomgeneric/m_menu.c",
            "vendor/doomgeneric/m_misc.c",
            "vendor/doomgeneric/m_random.c",
            "vendor/doomgeneric/p_ceilng.c",
            "vendor/doomgeneric/p_doors.c",
            "vendor/doomgeneric/p_enemy.c",
            "vendor/doomgeneric/p_floor.c",
            "vendor/doomgeneric/p_inter.c",
            "vendor/doomgeneric/p_lights.c",
            "vendor/doomgeneric/p_map.c",
            "vendor/doomgeneric/p_maputl.c",
            "vendor/doomgeneric/p_mobj.c",
            "vendor/doomgeneric/p_plats.c",
            "vendor/doomgeneric/p_pspr.c",
            "vendor/doomgeneric/p_saveg.c",
            "vendor/doomgeneric/p_setup.c",
            "vendor/doomgeneric/p_sight.c",
            "vendor/doomgeneric/p_spec.c",
            "vendor/doomgeneric/p_switch.c",
            "vendor/doomgeneric/p_telept.c",
            "vendor/doomgeneric/p_tick.c",
            "vendor/doomgeneric/p_user.c",
            "vendor/doomgeneric/r_bsp.c",
            "vendor/doomgeneric/r_data.c",
            "vendor/doomgeneric/r_draw.c",
            "vendor/doomgeneric/r_main.c",
            "vendor/doomgeneric/r_plane.c",
            "vendor/doomgeneric/r_segs.c",
            "vendor/doomgeneric/r_sky.c",
            "vendor/doomgeneric/r_things.c",
            "vendor/doomgeneric/sha1.c",
            "vendor/doomgeneric/sounds.c",
            "vendor/doomgeneric/statdump.c",
            "vendor/doomgeneric/st_lib.c",
            "vendor/doomgeneric/st_stuff.c",
            "vendor/doomgeneric/s_sound.c",
            "vendor/doomgeneric/tables.c",
            "vendor/doomgeneric/v_video.c",
            "vendor/doomgeneric/wi_stuff.c",
            "vendor/doomgeneric/w_checksum.c",
            "vendor/doomgeneric/w_file.c",
            "vendor/doomgeneric/w_main.c",
            "vendor/doomgeneric/w_wad.c",
            "vendor/doomgeneric/z_zone.c",
            "vendor/doomgeneric/w_file_stdc.c",
            "vendor/doomgeneric/i_input.c",
            "vendor/doomgeneric/i_video.c",
            "vendor/doomgeneric/doomgeneric.c",
        },
        // The original engine relies on wrapping arithmetic and legacy C.
        .flags = &.{ "-std=gnu99", "-fwrapv", "-fno-sanitize=undefined", "-Wno-deprecated-non-prototype", "-Wno-pointer-sign" },
    });
    const raylib = b.dependency("raylib", .{
        .target = target,
        .optimize = optimize,
        .raudio = true,
        .rmodels = false,
        // Doom renders the complete frame; raylib only displays the texture.
        .rtext = false,
        .rshapes = false,
        // Sound effects and ZiggySynth supply decoded PCM, so no audio codecs
        // are needed. Keep PNG export for the integration smoke screenshot.
        .config = "-DSUPPORT_FILEFORMAT_WAV=0 -DSUPPORT_FILEFORMAT_OGG=0 " ++
            "-DSUPPORT_FILEFORMAT_MP3=0 -DSUPPORT_FILEFORMAT_QOA=0 " ++
            "-DSUPPORT_FILEFORMAT_XM=0 -DSUPPORT_FILEFORMAT_MOD=0 " ++
            "-DSUPPORT_GESTURES_SYSTEM=0 -DSUPPORT_MOUSE_GESTURES=0 " ++
            "-DSUPPORT_AUTOMATION_EVENTS=0 -DSUPPORT_CLIPBOARD_IMAGE=0 " ++
            "-DSUPPORT_SCREEN_CAPTURE=0 -DSUPPORT_COMPRESSION_API=0 " ++
            "-DSUPPORT_IMAGE_GENERATION=0 -DSUPPORT_FILEFORMAT_BMP=0 " ++
            "-DSUPPORT_FILEFORMAT_GIF=0 -DSUPPORT_FILEFORMAT_QOI=0 " ++
            "-DSUPPORT_FILEFORMAT_DDS=0",
    });
    mod.linkLibrary(raylib.artifact("raylib"));
    const sound_tests_mod = b.createModule(.{ .target = target, .optimize = .Debug, .link_libc = true });
    sound_tests_mod.addIncludePath(b.path("vendor/doomgeneric"));
    sound_tests_mod.addIncludePath(raylib.path("src"));
    sound_tests_mod.addCSourceFiles(.{
        .files = &.{"tests/sound.c"},
        .flags = &.{ "-std=c99", "-Wall", "-Wextra" },
    });
    const sound_tests = b.addExecutable(.{ .name = "sound-tests", .root_module = sound_tests_mod });
    const run_sound_tests = b.addRunArtifact(sound_tests);
    const test_step = b.step("test", "Test string comparisons, DMX decoding and sound channel lifecycle");
    test_step.dependOn(&run_sound_tests.step);
    const string_tests_mod = b.createModule(.{ .target = target, .optimize = optimize, .link_libc = true });
    string_tests_mod.addIncludePath(b.path("vendor/doomgeneric"));
    string_tests_mod.addCSourceFiles(.{
        .files = &.{"tests/string_compare.c"},
        .flags = &.{ "-std=c99", "-Wall", "-Wextra" },
    });
    const string_tests = b.addExecutable(.{ .name = "string-tests", .root_module = string_tests_mod });
    const run_string_tests = b.addRunArtifact(string_tests);
    run_string_tests.addArg("SW1BRCOM");
    test_step.dependOn(&run_string_tests.step);
    const exe = b.addExecutable(.{ .name = if (smoke) "ziggy-doom-smoke" else "ziggy-doom", .root_module = mod });
    b.installArtifact(exe);
    const run = b.addRunArtifact(exe);
    run.setCwd(b.path("."));
    if (b.args) |args| run.addArgs(args);
    b.step("run", "Play Doom (pass engine options after --)").dependOn(&run.step);
}
