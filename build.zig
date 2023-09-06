// Build file for a C project

const std = @import("std");

pub fn build(b: *std.build.Builder) void {

    // Target and release options
    const target_opt = b.standardTargetOptions(.{});
    const release_opt = b.standardReleaseOptions();

    // Add a conditional option
    const tracy = b.option(bool, "tracy", "Enable Tracy profiler") orelse false;

    // First parameter is the name of the generated binary
    // Second parameter is null because we don't have a zig executable
    // If zig code is used, then the main() must be in the zig file
    const exe = b.addExecutable("ig_template", null);
    exe.setTarget(target_opt);
    exe.setBuildMode(release_opt);

    // common flags
    // our source tree flags
    // submodule flags
    // debug flags
    // release flags
    // profile flags
    // sanitizer flags
    // cross flags

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
        "src/inkscape/ig_template.c",
    };
    exe.addCSourceFiles(&sources, &cflags);

    // Conditional compilation
    if (tracy) {
        // Defines a macro like using a -D compiler flag
        // Second argument is optional value of macro
        exe.defineCMacro("TRACY_ENABLE", null);
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
    // exe.linkLibCpp();

    // Add extra libraries. This uses pkg-config
    // exe.linkSystemLibrary("libcurl");

    // This adds search paths for include and library directories
    // for when pkg-config cannot be used
    exe.addIncludePath("src/include");
    exe.addIncludePath("src/inkscape");
    exe.addIncludePath("src/imgui");
    exe.addIncludePath("src/submodules/cimgui");
    exe.addIncludePath("src/submodules/cimgui/generator/output");
    exe.addIncludePath("src/submodules/cimgui/imgui/examples/libs/glfw/include");
    exe.addIncludePath("src/submodules/mxml");
    exe.addIncludePath("src/submodules/sds");
    exe.addIncludePath("src/submodules/cimnodes");
    exe.addIncludePath("src/submodules/cimplot");
    // exe.addLibraryPath("/tmp");
    // This specifies the library by name (-lcurl)
    // exe.linkSystemLibraryName("curl");

    // We can also link static libraries
    // exe.addObjectFile("libcurl.a");

    // If we need per-file include paths, we use this approach
    // exe.addCSourceFile("buffer.c", &[_][]const u8{"-I", "include_dir"});

    exe.install();

    // Replace Makefile
    const make_step = b.step("make", "Build the project using make");
    make_step.makeFn = runMake;

    // If this is set as the default_step it will be run with every 'zig build'
    // b.default_step = make_step;

    // A custom step (ls -alh /tmp)
    const ls_step = b.step("ls", "List the contents of the /tmp directory");
    ls_step.makeFn = ls;
}

// ░█▀▀░▀█▀░█▀▀░█▀█░█▀▀
// ░▀▀█░░█░░█▀▀░█▀▀░▀▀█
// ░▀▀▀░░▀░░▀▀▀░▀░░░▀▀▀

// **ls** List the contents of the /tmp directory.
fn ls(self: *std.build.Step) !void {

    // self is unused
    _ = self;

    // Zig and its bizarre memory allocation shenanigans
    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    var allocator: std.mem.Allocator = arena.allocator();
    defer arena.deinit();

    // The arguments must be split first (you can use std.mem.split if
    // you have all of your arguments in one string)
    var command = std.ChildProcess.init(&[_][]const u8{ "/bin/ls", "-alh" }, allocator);
    // defer command.deinit();

    // By default, standard input is inherited, but to give the child program
    // a specific input, you need to request it to open a pipe with parent program.
    command.stdin_behavior = .Pipe;

    // Spawn the process and write to its standard input.
    try command.spawn();
    try command.stdin.?.writer().print("plot sin(1/x)\n", .{});

    // Now wait for the program to end.
    _ = try command.wait();
}

///**Builds the Project** using make.
fn runMake(self: *std.build.Step) !void {

    // self is unused
    _ = self;

    // Zig and its bizarre memory allocation shenanigans
    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    var allocator: std.mem.Allocator = arena.allocator();
    defer arena.deinit();

    // The arguments must be split first (you can use std.mem.split if
    // you have all of your arguments in one string)
    var command = std.ChildProcess.init(&[_][]const u8{"make"}, allocator);
    // defer command.deinit();

    // By default, standard input is inherited, but to give the child program
    // a specific input, you need to request it to open a pipe with parent program.
    command.stdin_behavior = .Pipe;

    // Spawn the process and write to its standard input.
    try command.spawn();
    try command.stdin.?.writer().print("plot sin(1/x)\n", .{});

    // Now wait for the program to end.
    _ = try command.wait();
}

// ░█░█░█▀█░▀█▀░▀█▀░░░▀█▀░█▀▀░█▀▀░▀█▀░█▀▀
// ░█░█░█░█░░█░░░█░░░░░█░░█▀▀░▀▀█░░█░░▀▀█
// ░▀▀▀░▀░▀░▀▀▀░░▀░░░░░▀░░▀▀▀░▀▀▀░░▀░░▀▀▀

// const c = @cImport({
//     @cInclude("src/libblastpit");
// });

// test "AutoGenerateId" {
//     var bp: c.t_Blastpit = c.blastpitNew();
//     try std.testing.expect(c.AutoGenerateId(bp) == 1);
// }

test "expect addOne adds one to 41" {

    // The Standard Library contains useful functions to help create tests.
    // `expect` is a function that verifies its argument is true.
    // It will return an error if its argument is false to indicate a failure.
    // `try` is used to return an error to the test runner to notify it that the test failed.
    // var c.t_Blastpit *bp = c.blastpitNew();
    try std.testing.expect(addOne(41) == 42);
}

/// The function `addOne` adds one to the number given as its argument.
fn addOne(number: i32) i32 {
    return number + 1;
}
