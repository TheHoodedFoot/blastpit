const std = @import("std");
const expect = std.testing.expect;

// Import other zig files
const steamcontroller = @import("steamcontroller.zig");

// Import c header files and #defines
// pub usingnamespace @cImport({
//     @cInclude("blastmine_c.h");
//     @cDefine("blastmine_DEFINED_BY_ZIG", "sometext");
// });

// Blastpit engine (blastmine)
// 
// Runs the WebSocket server
// Accepts SVG drawings from Inkscape, input from SpaceMouse and Steam Controller,
// and commands from browser via websockets
//
// Parses and divides an SVG drawing into shadows
// Controls laser and executes drawings.

pub fn blastmine() void {
    std.debug.warn("blastmine.blastmine()\n", .{});
}

test "WS Server: Listen" {
    // Should start a listening server
    // Should accept clients
    expect(true);
}

// WebSocket Server Tests

// Must accept clients
// Must receive data
// Must be able to reply to client
// Must acknowledge receipt of messages
// Must forward messages to other clients
// Must acknowledge failure to forward messages

pub fn main() anyerror!void {
    std.debug.warn("All your codebase are belong to us.\n", .{});
    blastmine();
    steamcontroller.steamcontroller();
}

