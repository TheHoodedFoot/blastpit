# ░█▀▀░█▀▄░█▀█░█▀▀░█▀▀░░░█▀▀░█▀█░█▄█░█▀█░▀█▀░█░░░▀█▀░█▀█░█▀▀
# ░█░░░█▀▄░█░█░▀▀█░▀▀█░░░█░░░█░█░█░█░█▀▀░░█░░█░░░░█░░█░█░█░█
# ░▀▀▀░▀░▀░▀▀▀░▀▀▀░▀▀▀░░░▀▀▀░▀▀▀░▀░▀░▀░░░▀▀▀░▀▀▀░▀▀▀░▀░▀░▀▀▀

# Use:
# 	make lmos
#
# from within 32-bit chroot
#
# Change setting in lmos.pro to debug_and_release for release version

WINE_ARCH = "win32"
WINE_LIBS = "C:\\Qt\\Qt5.14.2\\Tools\\mingw730_32\\bin"
WINE_CXX = $(WINE_LIBS)"\\g++.exe"
QT_DLL_DIR = "Qt/Qt5.14.2/5.14.2/mingw730_32/bin"
WINE_BINARY = wine
WINEPREFIX="${HOME}/.wine/32bit"

# Emscripten

emscripten_update:
	git -C $(SUBMODULES_DIR)/emsdk checkout main
	git -C $(SUBMODULES_DIR)/emsdk pull
	$(SUBMODULES_DIR)/emsdk/emsdk install latest
	$(SUBMODULES_DIR)/emsdk/emsdk activate latest

# Qt (Wine) - Must be run from within the 32-bit environment
$(BUILD_DIR)/win32/debug/platforms $(BUILD_DIR)/win32/release/platforms:
	mkdir -p $(BUILD_DIR)/win32/{release,debug}/platforms
	cp -v $(WINEPREFIX)/drive_c/Qt/Qt5.14.2/5.14.2/mingw73_32/bin/Qt5{Core,Gui,Widgets}.dll $(BUILD_DIR)/win32/release/
	cp -v $(WINEPREFIX)/drive_c/Qt/Qt5.14.2/5.14.2/mingw73_32/plugins/platforms/qwindows.dll $(BUILD_DIR)/win32/release/platforms
	cp -v $(WINEPREFIX)/drive_c/Qt/Qt5.14.2/5.14.2/mingw73_32/bin/Qt5{Core,Gui,Widgets}.dll $(BUILD_DIR)/win32/debug/
	cp -v $(WINEPREFIX)/drive_c/Qt/Qt5.14.2/5.14.2/mingw73_32/plugins/platforms/qwindows.dll $(BUILD_DIR)/win32/debug/platforms

qmake $(BUILD_DIR)/win32/Makefile: $(BUILD_DIR)
	git describe --long --dirty --always > $(BUILD_DIR)/win32/git_version.txt
	git branch --show-current > $(BUILD_DIR)/win32/git_branch.txt
	git log -1 --pretty=format:%s > $(BUILD_DIR)/win32/git_description.txt
	git show -s --format=%ci > $(BUILD_DIR)/win32/git_date.txt
	env WINEARCH="win32" \
		WINEPREFIX="${HOME}/.wine/32bit" \
		WINEPATH="C:\\Qt\\Qt5.14.2\\Tools\\mingw730_32\\bin" \
		$(WINE_BINARY) $(WINEPREFIX)/drive_c/Qt/Qt5.14.2/5.14.2/mingw73_32/bin/qmake.exe \
		-o $(BUILD_DIR)/win32/Makefile \
		src\\lmos\\lmos.pro

lmos $(BUILD_DIR)/win32/debug/lmostray.exe $(BUILD_DIR)/win32/release/lmostray.exe:	cross $(BUILD_DIR)/win32/Makefile $(BUILD_DIR)/win32/debug/platforms $(BUILD_DIR)/win32/release/platforms $(LIBBLASTPIT_OBJS)
	env WINEARCH="win32" \
		WINEPREFIX="${HOME}/.wine/32bit" \
		WINEPATH="C:\\Qt\\Qt5.14.2\\Tools\\mingw730_32\\bin" \
		$(WINE_BINARY) $(WINEPREFIX)/drive_c/Qt/Qt5.14.2/Tools/mingw730_32/bin/mingw32-make.exe -C $(BUILD_DIR)/win32

lmostar:	$(BUILD_DIR)/win32/release/lmostray.exe $(BUILD_DIR)/win32/debug/lmostray.exe tarball

lmosx:		$(BUILD_DIR)/win32/release/lmostray.exe
	env WINEARCH="win32" \
		WINEPREFIX="${HOME}/.wine/32bit" \
		WINEPATH="C:\\Qt\\Qt5.14.2\\Tools\\mingw730_32\\bin" \
		$(WINE_BINARY) $(BUILD_DIR)/win32/release/lmostray.exe

lmoslin $(BUILD_DIR)/lmostray: $(BUILD_DIR)/Makefile
	make -C $(BUILD_DIR)

lmoslinx:	$(BUILD_DIR)/lmostray
	LSAN_OPTIONS=suppressions=$(PROJECT_ROOT)/.leak_blacklist.txt $(BUILD_DIR)/lmostray

# Build Mongoose example by cross-compiling with Zig
mongoose:	$(BUILD_DIR)
	zig cc \
		-target i386-windows-gnu \
		-DCS_PLATFORM=CS_P_WINDOWS \
		-o $(BUILD_DIR)/mongoose.exe \
		-I ~/projects/blastpit/src/submodules/mongoose \
		-L ~/.wine/32bit/drive_c/Qt/Qt5.14.2/Tools/mingw730_32/i686-w64-mingw32/lib \
		src/submodules/mongoose/mongoose.c \
		src/submodules/mongoose/examples/simplest_web_server/simplest_web_server.c \
		-lwsock32

$(BUILD_DIR)/wscli:	$(BUILD_DIR)/wscli.o
	$(CC) $(CPPFLAGS) $(SANFLAGS) -o $@ $^ $(LIBBLASTPIT_OBJS) $(WIN32FLAGS)

$(BUILD_DIR)/wscli.o:	$(LIBBLASTPIT_OBJS) $(LIBBLASTPIT_DIR)/wscli.c
	$(CC) $(CPPFLAGS) $(SANFLAGS) -c -I $(LIBBLASTPIT_DIR)/libblastpit -I $(SUBMODULES_DIR)/mongoose -o $@ -x c $(LIBBLASTPIT_DIR)/wscli.c


server:	$(BUILD_DIR)/wscli
	$^ -s $(shell grep WS_SERVER_LOCAL res/cfg/myconfig.py | awk '{print $$3}' | sed s/\"//g)

# Packaging

tarball:	lmos
		cd build/win32/release && \
		cp ../debug/lmostray.exe lmostray_debug.exe && \
		zip -r /tmp/lmos.zip \
		--exclude=\*.o \
		--exclude=\*.h \
		--exclude=\*.cpp \
		Qt5Core.dll Qt5Gui.dll Qt5Widgets.dll lmostray.exe lmostray_debug.exe platforms && \
		cd $(WINEPREFIX)/drive_c/Qt/Qt5.14.2/Tools/mingw730_32/i686-w64-mingw32/lib && \
		zip /tmp/lmos.zip \
		libstdc++-6.dll libgcc_s_dw2-1.dll libwinpthread-1.dll

