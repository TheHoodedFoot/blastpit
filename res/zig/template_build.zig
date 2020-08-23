// Zig template to run make in git root directory

const std = @import("std");
const Builder = std.build.Builder;

pub fn build(b: *Builder) void {

    // Boilerplate for zig
    const target = b.standardTargetOptions(.{});
    const mode = b.standardReleaseOptions();

    // Allocators for string concatenation
    var arena_alloc = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena_alloc.deinit();
    const arena = &arena_alloc.allocator;

    // Run a command and get the output
    const git_root = b.exec(&[_][]const u8{ "git", "rev-parse", "--show-toplevel" });

    var buffer: [512]u8 = undefined;
    // const allocator = arena.create(&buffer).allocator;

    // var result = try allocator.alloc(u8, 10);
    std.mem.copy(u8, @as([]u8, buffer), git_root);
    std.mem.copy(u8, buffer[git_root.len..], "/build");


    // var build_root: []u8 = undefined;
    // std.mem.copy(u8, build_root, git_root);
    // std.mem.copy(u8, build_root[git_root.len..], "/build");
    std.debug.warn("git_root: {}\n", .{ git_root });
    std.debug.warn("type git_root: {}\n", .{ @typeName(@TypeOf(git_root)) });
    // _ = b.exec(&[_][]const u8{ "make", "-C", build_root, "-f", "../Makefile" });
}
