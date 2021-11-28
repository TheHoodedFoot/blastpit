const std = @import("std");

pub fn build(b: *std.build.Builder) void {

    // Standard target options allows the person running `zig build` to choose
    // what target to build for. Here we do not override the defaults, which
    // means any target is allowed, and the default is native. Other options
    // for restricting supported target set are available.
    const target = b.standardTargetOptions(.{});

    // Standard release options allow the person running `zig build` to select
    // between Debug, ReleaseSafe, ReleaseFast, and ReleaseSmall.
    const mode = b.standardReleaseOptions();

    // // Executable
    // const exe = b.addExecutable("wirer232", "src/wirer232.zig");
    // exe.setTarget(target);
    // exe.setBuildMode(mode);
    // exe.install();

    // const run_cmd = exe.run();
    // run_cmd.step.dependOn(b.getInstallStep());
    // if (b.args) |args| {
    //     run_cmd.addArgs(args);
    // }

    // // Run target
    // const run_step = b.step("run", "Run the app");
    // run_step.dependOn(&run_cmd.step);

    // // Include code kept in other .zig files
    // exe.addPackagePath("libwirer232", "src/libwirer232.zig");

    // // Build a static library
    // const lib = b.addStaticLibrary("test_library", "src/libwirer232.zig");
    // lib.setBuildMode(mode);
    // lib.install();

    // // Unit test target
    // const test_step = b.step("test", "Run library tests");
    // var main_tests = b.addTest("test/test_wirer232.zig");
    // main_tests.setBuildMode(mode);
    // main_tests.addPackagePath("libwirer232", "src/libwirer232.zig");
    // test_step.dependOn(&main_tests.step);

    // Replace Makefile
    const make_step = b.step("make", "Build the project using make");
    make_step.makeFn = runMake;

    // If this is set as the default_step it will be run with every 'zig build'
    b.default_step = make_step;

    // A custom target (ls -alh /tmp)
    const ls_step = b.step("ls", "List the contents of the /tmp directory");
    ls_step.makeFn = ls;
}

///**ls** List the contents of the /tmp directory.
fn ls(self: *std.build.Step) !void {

    // self is unused
    _ = self;

    // Zig and its bizarre memory allocation shenanigans
    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    var allocator: *std.mem.Allocator = &arena.allocator;
    defer arena.deinit();

    // The arguments must be split first (you can use std.mem.split if
    // you have all of your arguments in one string)
    var command = try std.ChildProcess.init(&[_][]const u8{ "/bin/ls", "-alh" }, allocator);
    defer command.deinit();

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
    var allocator: *std.mem.Allocator = &arena.allocator;
    defer arena.deinit();

    // The arguments must be split first (you can use std.mem.split if
    // you have all of your arguments in one string)
    var command = try std.ChildProcess.init(&[_][]const u8{"make"}, allocator);
    defer command.deinit();

    // By default, standard input is inherited, but to give the child program
    // a specific input, you need to request it to open a pipe with parent program.
    command.stdin_behavior = .Pipe;

    // Spawn the process and write to its standard input.
    try command.spawn();
    try command.stdin.?.writer().print("plot sin(1/x)\n", .{});

    // Now wait for the program to end.
    _ = try command.wait();
}
