const std = @import("std");

const root = @import("../../build.zig");

pub fn buildDep(b: *std.Build, options: root.Options, comptime path: []const u8) *std.Build.Step.Compile {
    const mpq = b.addStaticLibrary(.{
        .name = "mpq",
        .target = options.target,
        .optimize = options.optimize,
    });
    const prefix = root.rel(path, root.dir(@src()));

    // std.debug.print("{s}\n", .{foo});
    mpq.addIncludePath(.{ .path = prefix });
    mpq.addCSourceFiles(.{
        .files = &.{
            prefix ++ "/libmpq/mpq.c",
            prefix ++ "/libmpq/extract.c",
            prefix ++ "/libmpq/wave.c",
            prefix ++ "/libmpq/huffman.c",
            prefix ++ "/libmpq/explode.c",
            prefix ++ "/libmpq/common.c",
        },
        .flags = &.{},
    });
    mpq.linkLibC();
    return mpq;
}

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});
    const mpq = buildDep(b, .{
        .optimize = optimize,
        .target = target,
    }, ".");

    b.installArtifact(mpq);
}
