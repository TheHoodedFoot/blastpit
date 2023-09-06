// Build file for a C project

const std = @import("std");

pub fn build(b: *std.build.Builder) void {

    // Target and release options
    const target_opt = b.standardTargetOptions(.{});
    const release_opt = b.standardReleaseOptions();

    // Add a conditional option
    const opt_a = b.option(bool, "opt_a", "Enable option a") orelse true;

    // First parameter is the name of the generated binary
    // Second parameter is null because we don't have a zig executable
    // If zig code is used, then the main() must be in the zig file
    const exe = b.addExecutable("example", null);
    exe.setTarget(target_opt);
    exe.setBuildMode(release_opt);

    // We can store common flags and apply to multiple files
    const cflags = [_][]const u8{
        "-Wall",
        "-Wextra",
    };

    // Second parameter is command line options
    // exe.addCSourceFile("main.c", &cflags);
    // We can turn off sanitizer per file
    // exe.addCSourceFile("buffer.c", &[_][]const u8{"-fno-sanitize=undefined"});
    // Or have per-file standards
    // exe.addCSourceFile("buffer.c", &[_][]const u8{"-std=c90"});

    // Compile multiple files
    const sources = [_][]const u8{
        "t_win32_common.cpp",
        "win32_common.cpp",
    };
    exe.addCSourceFiles(&sources, &cflags);

    // Conditional compilation
    if (opt_a) {
        // Defines a macro like using a -D compiler flag
        // Second argument is optional value of macro
        exe.defineCMacro("USE_OPTION_A", null);
    }

    // Per-target compilation
    // if (exe.target.isWindows()) {
    //     exe.addCSourceFile("windows.c", &[_][]const u8{});
    // } else {
    //     exe.addCSourceFile("linux.c", &[_][]const u8{});
    // }

    // Build will fail without linking libc
    // If building C++ then we would add .cpp files above and use
    // exe.linkLibCpp() in addition to exe.linkLibC()
    exe.linkLibC();
    exe.linkLibCpp();

    // Add extra libraries. This uses pkg-config
    // exe.linkSystemLibrary("libcurl");

    // This adds search paths for include and library directories
    // for when pkg-config cannot be used
    // exe.addIncludePath("/tmp");
    // exe.addLibraryPath("/tmp");
    // This specifies the library by name (-lcurl)
    // exe.linkSystemLibraryName("curl");

    // We can also link static libraries
    // exe.addObjectFile("libcurl.a");

    // If we need per-file include paths, we use this approach
    // exe.addCSourceFile("buffer.c", &[_][]const u8{"-I", "include_dir"});

    exe.install();
}
