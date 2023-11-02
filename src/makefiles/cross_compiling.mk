# ░█▀▀░█▀▄░█▀█░█▀▀░█▀▀░░░█▀▀░█▀█░█▄█░█▀█░▀█▀░█░░░▀█▀░█▀█░█▀▀
# ░█░░░█▀▄░█░█░▀▀█░▀▀█░░░█░░░█░█░█░█░█▀▀░░█░░█░░░░█░░█░█░█░█
# ░▀▀▀░▀░▀░▀▀▀░▀▀▀░▀▀▀░░░▀▀▀░▀▀▀░▀░▀░▀░░░▀▀▀░▀▀▀░▀▀▀░▀░▀░▀▀▀

# Emscripten
emscripten_update:
	git -C $(SUBMODULES_DIR)/emsdk checkout main
	git -C $(SUBMODULES_DIR)/emsdk pull
	$(SUBMODULES_DIR)/emsdk/emsdk install latest
	$(SUBMODULES_DIR)/emsdk/emsdk activate latest

# Build Mongoose example by cross-compiling with Zig
mongoose:	$(BUILD_DIR)
	zig cc \
		-target x86-windows-gnu \
		-DCS_PLATFORM=CS_P_WINDOWS \
		-o $(BUILD_DIR)/mongoose.exe \
		-I ~/projects/blastpit/src/submodules/mongoose \
		-L ~/.wine/32bit/drive_c/Qt/Qt5.14.2/Tools/mingw730_32/i686-w64-mingw32/lib \
		src/submodules/mongoose/mongoose.c \
		src/submodules/mongoose/examples/simplest_web_server/simplest_web_server.c \
		-lwsock32
