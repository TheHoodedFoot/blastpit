# ░█░█░█▀▀░█▀▀░█░░░▀█▀
# ░█▄█░▀▀█░█░░░█░░░░█░
# ░▀░▀░▀▀▀░▀▀▀░▀▀▀░▀▀▀

wscli:	$(BUILD_DIR)/wscli

$(BUILD_DIR)/wscli:	$(BUILD_DIR)/wscli.o $(BUILD_DIR)/libblastpit.a $(BUILD_DIR)/external_libs.a $(TRACY_OBJS)
	$(CXX) $(CPPFLAGS) $(SANFLAGS) -o $@ $^ $(WIN32FLAGS)

$(BUILD_DIR)/wscli.o:	$(LIBBLASTPIT_OBJS) $(LIBBLASTPIT_DIR)/wscli.c
	$(CC) -MJ $@.json $(CPPFLAGS) $(SANFLAGS) -c -I $(LIBBLASTPIT_DIR)/libblastpit -I $(SUBMODULES_DIR)/mongoose -o $@ $(LIBBLASTPIT_DIR)/wscli.c


wscli_server:	$(BUILD_DIR)/wscli
	$^ -s $(shell grep WS_SERVER_LOCAL res/cfg/myconfig.py | awk '{print $$3}' | sed s/\"//g)

# Using the x86 target here makes the binary more portable
wscli_portable wscli_portable_x86:
	make clean
	make $(BUILD_DIR)
	CC="zig cc -target x86-linux-musl -Ofast -fno-sanitize=all" CXX="zig c++ -target x86-linux-musl -Ofast -fno-sanitize=all" CPPFLAGS="-Ofast -fno-sanitize=all" make $(EXTERNAL_OBJS) wscli

wscli_portable_arm:
	make clean
	make $(BUILD_DIR)
	CC="zig cc -target arm-linux-musleabi -Ofast -fno-sanitize=all" CXX="zig c++ -target arm-linux-musleabi -Ofast -fno-sanitize=all" CPPFLAGS="-Ofast -fno-sanitize=all" make $(EXTERNAL_OBJS) wscli

wscli_portable_x86_debug:
	make clean
	make $(BUILD_DIR)
	# Disable undefined behaviour sanitiser in zig since Mongoose fails
	CC="zig cc -target x86-linux-musl -O0 -g3 -fno-sanitize=all" CXX="zig c++ -target x86-linux-musl -O0 -g3 -fno-sanitize=all" CPPFLAGS="-O0 -g3 -fno-sanitize=all" make wscli

debug_wscli_portable_x86:
	qemu-x86 -g 9999 -- $(BUILD_DIR)/wscli -s ws://0.0.0.0:8000 &
	gdb -ex "file build/wscli" -ex "target remote :9999"

