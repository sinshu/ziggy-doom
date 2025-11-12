const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});
    
    const mod = b.addModule("ziggy-doom", .{
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });
    
    mod.addCSourceFiles(.{
        .files = &.{
            "src/main.c",
        },
    });
    
    const exe = b.addExecutable(.{
        .name = "ziggy-doom",
        .root_module = mod,
    });
    
    const raylib_dep = b.dependency("raylib", .{
        .target = target,
        .optimize = optimize,
    });
    const raylib = raylib_dep.artifact("raylib");
    exe.linkLibrary(raylib);
    
    b.installArtifact(exe);
}
