# ░█░█░█▀▀░█▀▀░█░░░▀█▀
# ░█▄█░▀▀█░█░░░█░░░░█░
# ░▀░▀░▀▀▀░▀▀▀░▀▀▀░▀▀▀

wscli:	$(BUILD_DIR)/wscli

# Using the x86 target here makes the binary more portable
wscli_portable:
	make clean
	make $(BUILD_DIR)
	CC="zig cc -target x86-linux-musl -Ofast -fno-sanitize=all" CXX="zig c++ -target x86-linux-musl -Ofast -fno-sanitize=all" CPPFLAGS="-Ofast -fno-sanitize=all" make $(EXTERNAL_OBJS) wscli

wscli_portable_debug:
	make clean
	make $(BUILD_DIR)
	# Disable undefined behaviour sanitiser in zig since Mongoose fails
	CC="zig cc -target x86-linux-musl -O0 -g3 -fno-sanitize=all" CXX="zig c++ -target x86-linux-musl -O0 -g3 -fno-sanitize=all" CPPFLAGS="-O0 -g3 -fno-sanitize=all" make wscli

debug_wscli_portable:
	qemu-x86 -g 9999 -- $(BUILD_DIR)/wscli -s ws://0.0.0.0:8000 &
	gdb -ex "file build/wscli" -ex "target remote :9999"

