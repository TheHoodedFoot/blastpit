const std = @import("std");
const builtin = @import("builtin");

const Builder = std.build.Builder;

pub fn build(b: *Builder) void {

    // if (os.tag == std.Target.Os.Tag.windows) {
    // std.debug.warn("builtin.os: {}\n", .{ std.Target.Os.Tag.windows });
    // }

    // Boilerplate for zig
    const target = b.standardTargetOptions(.{});
    const mode = b.standardReleaseOptions();

    // Allocators for string concatenation
    var arena_alloc = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena_alloc.deinit();
    const arena = &arena_alloc.allocator;

    //
    //  libblastpit
    //

    // We need at least one zig file,
    // so fool it with this preexisting build.zig.
    const libblastpit = b.addStaticLibrary("blastpit", "build.zig");

    // Add the main sources to the library
    const libblastpit_c_files = &[_][]const u8 {
        "blastpit.c",
        "linkedlist.c",
        "message.c",
        "mqtt.c",
        "new.c",
        "parser.c",
    };

    // I think that the 'inline' forces compile-time parsing,
    // which allows for the use of the ++ operator.
    inline for (libblastpit_c_files) |c_file| {
        libblastpit.addCSourceFile("src/libblastpit/" ++ c_file, &[_][]const u8 { "-fPIC" });
    }

    // For linux we can just add mongoose.c and websocket.c to the list of files
    libblastpit.addCSourceFile("src/libblastpit/websocket.c", &[_][]const u8 { "-fPIC" });
    libblastpit.addCSourceFile("src/submodules/mongoose/mongoose.c", &[_][]const u8 { "-fPIC" });

    // For windows, we need to build the object files first using zig cc,
    // so use 'make mongoosewin' first and then add the files to the build:
    // libblastpit.addObjectFile("build/mongoose.o");
    // libblastpit.addObjectFile("build/websocket.o");

    libblastpit.addIncludeDir("/usr/include");
    libblastpit.addIncludeDir("src/submodules/mongoose");

    libblastpit.setBuildMode(mode);
    libblastpit.install();

    // Binary
    const blastmine = b.addExecutable("blastmine", "src/blastmine/blastmine.zig");
    blastmine.addIncludeDir("src/blastmine");
    blastmine.addLibPath("build/lib");
    blastmine.linkSystemLibrary("blastpit");
    blastmine.addCSourceFile("src/blastmine/blastmine.c", &[_][]const u8 { });
    blastmine.setTarget(target);
    blastmine.setBuildMode(mode);
    blastmine.install();

    // const test_cmd = unittest.run();
    // test_cmd.step.dependOn(b.getInstallStep());

    // const test_step = b.step("test", "Run the unit tests");
    // test_step.dependOn(&test_cmd.step);
}

// Run a command and get the output
// const includes_output = try b.exec(&[_][]const u8{ llvm_config_exe, "--includedir" });
// const mk_build_bin_dir = try b.exec(&[_][]const u8{ "mkdir", "-p", "build/bin" });

// Or build a c program directly with clang
// const bin_dest = "build/bin/ws_server";
// const websockets = b.addSystemCommand(&[_][]const u8{
//     "clang",
//     "src/libblastpit/minimal-ws-server/minimal-ws-server.c",
//     "-o",
//     bin_dest,
//     "-lwebsockets",
// });

// unittest.addIncludeDir("/usr/include");
// unittest.linkSystemLibrary("pthread");
// unittest.linkSystemLibrary("stdc++");

// const run_objcopy = b.addSystemCommand(&[_][]const u8{
//     "cp", "/home/tmp/zigcash/blastpit/lib/libblastpit.a",
//     "/home/thf/projects/blastpit/build/"
// });
// run_objcopy.step.dependOn(&libblastpit.step);

// b.default_step.dependOn(&run_objcopy.step);

// Add a custom build step for the directory creation
// const run_mkdir = b.addSystemCommand(&[_][]const u8{
//     "mkdir", "-p", "build/bin",
// });

// Depend on the build directory being created
// unittest.step.dependOn(&libblastpit.step);
// unittest.step.dependOn(&run_mkdir.step);


pub fn concatenate() anyerror!void {

    var buf: [100]u8 = undefined;

    var num: u8 = 123;

    var result = std.fmt.bufPrint(buf[0..], "{}", .{num});
    result = std.fmt.bufPrint(buf[3..], "{}", .{num});

    try std.io.getStdOut().outStream().print("Type: {}\n", .{@TypeOf(result)});
    try std.io.getStdOut().outStream().print("{}\n", .{buf});

}
