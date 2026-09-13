// SPDX-License-Identifier: GPL-2.0-or-later
const std = @import("std");
const zs = @import("ziggysynth");
const allocator = std.heap.c_allocator;
var font: zs.SoundFont = undefined;
var synth: zs.Synthesizer = undefined;
var sequencer: zs.MidiFileSequencer = undefined;
var ready = false;

export fn ZS_Init(data: [*]const u8, size: usize) bool {
    if (ready) return true;
    var reader = std.Io.Reader.fixed(data[0..size]);
    font = zs.SoundFont.init(allocator, &reader) catch return false;
    const settings = zs.SynthesizerSettings.init(44100);
    synth = zs.Synthesizer.init(allocator, &font, &settings) catch {
        font.deinit();
        return false;
    };
    sequencer = zs.MidiFileSequencer.init(&synth);
    ready = true;
    return true;
}
export fn ZS_Shutdown() void {
    if (!ready) return;
    synth.deinit();
    font.deinit();
    ready = false;
}
export fn ZS_Register(data: [*]const u8, size: usize) ?*zs.MidiFile {
    if (!ready) return null;
    const midi = allocator.create(zs.MidiFile) catch return null;
    var reader = std.Io.Reader.fixed(data[0..size]);
    midi.* = zs.MidiFile.init(allocator, &reader) catch {
        allocator.destroy(midi);
        return null;
    };
    if (midi.messages.len == 0) {
        midi.deinit();
        allocator.destroy(midi);
        return null;
    }
    return midi;
}
export fn ZS_Unregister(song: ?*zs.MidiFile) void {
    const midi = song orelse return;
    if (ready and sequencer.midi_file == midi) sequencer.stop();
    midi.deinit();
    allocator.destroy(midi);
}
export fn ZS_Play(song: ?*zs.MidiFile, loop: bool) void {
    if (ready) sequencer.play(song orelse return, loop);
}
export fn ZS_Stop() void {
    if (ready) sequencer.stop();
}
export fn ZS_Ended() bool {
    if (!ready) return true;
    const midi = sequencer.midi_file orelse return true;
    // Allow note releases/reverb to decay after the last MIDI event.
    return !sequencer.play_loop and sequencer.current_time >= midi.getLength() + 2.0;
}
export fn ZS_Render(output: [*]f32, frames: usize) void {
    if (!ready) {
        @memset(output[0 .. frames * 2], 0);
        return;
    }
    var left: [1024]f32 = undefined;
    var right: [1024]f32 = undefined;
    var pos: usize = 0;
    while (pos < frames) {
        const count = @min(frames - pos, left.len);
        sequencer.render(left[0..count], right[0..count]);
        for (0..count) |i| {
            output[2 * (pos + i)] = left[i];
            output[2 * (pos + i) + 1] = right[i];
        }
        pos += count;
    }
}
