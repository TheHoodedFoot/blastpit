# ░█░█░█▀▀░█▀▀░█░░░▀█▀
# ░█▄█░▀▀█░█░░░█░░░░█░
# ░▀░▀░▀▀▀░▀▀▀░▀▀▀░▀▀▀

wscli:	$(BUILD_DIR)/wscli

# Using the i386 target here makes the binary more portable
wscli_portable:
	make clean
	CC="zig cc -target i386-linux-musl -Ofast -fno-sanitize=all" CXX="zig c++ -target i386-linux-musl -Ofast -fno-sanitize=all" CPPFLAGS="-Ofast -fno-sanitize=all" make wscli

wscli_portable_debug:
	make clean
	# Disable undefined behaviour sanitiser in zig since Mongoose fails
	CC="zig cc -target i386-linux-musl -O0 -g3 -fno-sanitize=all" CXX="zig c++ -target i386-linux-musl -O0 -g3 -fno-sanitize=all" CPPFLAGS="-O0 -g3 -fno-sanitize=all" make wscli

debug_wscli_portable:
	qemu-i386 -g 9999 -- $(BUILD_DIR)/wscli -s ws://0.0.0.0:8000 &
	gdb -ex "file build/wscli" -ex "target remote :9999"

