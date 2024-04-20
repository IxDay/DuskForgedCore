const Builder = @import("std").build.Builder;

pub fn build(b: *Builder) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const bzip2 = b.addStaticLibrary(.{
        .name = "bzip2",
        .target = target,
        .optimize = optimize,
    });
    bzip2.addIncludePath(.{ .path = "deps/libmpq" });
    // mpq.addIncludePath(.{ .path = "deps/libmpq/libmpq" });
    bzip2.addCSourceFiles(&.{
        "deps/bzip2/huffman.c",
        "deps/bzip2/randtable.c",
        "deps/bzip2/blocksort.c",
        "deps/bzip2/bzlib.c",
        "deps/bzip2/crctable.c",
        "deps/bzip2/decompress.c",
        "deps/bzip2/compress.c",
    }, &.{});
    bzip2.linkLibC();

    const mpq = b.addStaticLibrary(.{
        .name = "mpq",
        .target = target,
        .optimize = optimize,
    });
    mpq.addIncludePath(.{ .path = "deps/libmpq" });
    // mpq.addIncludePath(.{ .path = "deps/libmpq/libmpq" });
    mpq.addCSourceFiles(&.{
        "deps/libmpq/libmpq/mpq.c",
        "deps/libmpq/libmpq/extract.c",
        "deps/libmpq/libmpq/wave.c",
        "deps/libmpq/libmpq/huffman.c",
        "deps/libmpq/libmpq/explode.c",
        "deps/libmpq/libmpq/common.c",
    }, &.{});
    mpq.linkLibrary(bzip2);
    mpq.linkLibC();

    const z = b.addStaticLibrary(.{
        .name = "z",
        .target = target,
        .optimize = optimize,
    });
    z.linkLibC();
    z.addCSourceFiles(&.{
        "deps/zlib/deflate.c",
        "deps/zlib/gzwrite.c",
        "deps/zlib/adler32.c",
        "deps/zlib/crc32.c",
        "deps/zlib/trees.c",
        "deps/zlib/uncompr.c",
        "deps/zlib/gzlib.c",
        "deps/zlib/gzclose.c",
        "deps/zlib/inffast.c",
        "deps/zlib/zutil.c",
        "deps/zlib/inftrees.c",
        "deps/zlib/infback.c",
        "deps/zlib/inflate.c",
        "deps/zlib/compress.c",
        "deps/zlib/gzread.c",
    }, &.{"-std=c89"});

    // const common = b.addStaticLibrary(.{
    //     .name = "common",
    //     .target = target,
    //     .optimize = optimize,
    // });
    // common.linkLibCpp();

    const map_extractor = b.addExecutable(.{
        .name = "map_extractor",
        .target = target,
        .optimize = optimize,
    });

    const fmt = b.addStaticLibrary(.{
        .name = "fmt",
        .target = target,
        .optimize = optimize,
    });
    fmt.addIncludePath(.{ .path = "deps/fmt/include/fmt" });
    fmt.addCSourceFiles(&.{
        "deps/fmt/src/format.cc",
        "deps/fmt/src/os.cc",
        "deps/fmt/src/fmt.cc",
    }, &.{"-std=c++20"});
    fmt.linkLibCpp();

    map_extractor.linkLibrary(mpq);
    map_extractor.linkLibrary(z);
    map_extractor.linkLibrary(fmt);
    map_extractor.addIncludePath(.{ .path = "src/common" });
    map_extractor.addIncludePath(.{ .path = "src/common/Utilities" });
    map_extractor.linkLibCpp();
    map_extractor.addIncludePath(.{ .path = "src/tools/map_extractor/loadlib" });
    map_extractor.addIncludePath(.{ .path = "deps/libmpq/libmpq" });
    map_extractor.addCSourceFiles(&.{
        "src/tools/map_extractor/adt.cpp",
        "src/tools/map_extractor/dbcfile.cpp",
        "src/tools/map_extractor/System.cpp",
        "src/tools/map_extractor/mpq_libmpq.cpp",
        "src/tools/map_extractor/wdt.cpp",
        "src/tools/map_extractor/loadlib.cpp",
    }, &.{});

    b.installArtifact(map_extractor);
}
