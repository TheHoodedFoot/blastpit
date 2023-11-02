# ░█░░░█▄█░█▀█░█▀▀
# ░█░░░█░█░█░█░▀▀█
# ░▀▀▀░▀░▀░▀▀▀░▀▀▀

PLATFORM   = x86-windows-gnu
WINEDIR    = lmos
WINEPRE    = /home/$(shell whoami)/wine/$(WINEDIR)

LMOS_SRC  = $(SRC_DIR)/lmos/lmos.c
LMOS_OBJ  = $(BUILD_DIR)/lmos.o
LMOS_EXE  = $(BUILD_DIR)/lmos.exe
LMOS_LIBS =  -lole32 -loleaut32 -luuid -lwsock32
# LMOS_START_SCRIPT = $(PROJECT_ROOT)/res/bin/lmos.sh

FLAGS = -g3 -O0 -fno-omit-frame-pointer -fno-stack-protector

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
WINEPREFIX="${HOME}/wine/lmos"

# ░▄▀▄░▀█▀░░░█░░░█▄█░█▀█░█▀▀
# ░█\█░░█░░░░█░░░█░█░█░█░▀▀█
# ░░▀\░░▀░░░░▀▀▀░▀░▀░▀▀▀░▀▀▀

# Qt (Wine) - Must be run from within the 32-bit environment
$(BUILD_DIR)/win32/debug/platforms $(BUILD_DIR)/win32/release/platforms:
	mkdir -p $(BUILD_DIR)/win32/{release,debug}/platforms
	cp -v $(WINEPREFIX)/drive_c/Qt/Qt5.14.2/5.14.2/mingw73_32/bin/Qt5{Core,Gui,Widgets}.dll $(BUILD_DIR)/win32/release/
	cp -v $(WINEPREFIX)/drive_c/Qt/Qt5.14.2/5.14.2/mingw73_32/plugins/platforms/qwindows.dll $(BUILD_DIR)/win32/release/platforms
	cp -v $(WINEPREFIX)/drive_c/Qt/Qt5.14.2/5.14.2/mingw73_32/bin/Qt5{Core,Gui,Widgets}.dll $(BUILD_DIR)/win32/debug/
	cp -v $(WINEPREFIX)/drive_c/Qt/Qt5.14.2/5.14.2/mingw73_32/plugins/platforms/qwindows.dll $(BUILD_DIR)/win32/debug/platforms

qmake $(BUILD_DIR)/win32/Makefile: $(BUILD_DIR) images
	mkdir -p $(BUILD_DIR)/win32
	git describe --long --dirty --always > $(BUILD_DIR)/win32/git_version.txt
	git branch --show-current > $(BUILD_DIR)/win32/git_branch.txt
	git log -1 --pretty=format:%s > $(BUILD_DIR)/win32/git_description.txt
	git show -s --format=%ci > $(BUILD_DIR)/win32/git_date.txt
	env WINEARCH="win32" \
		WINEPREFIX="${HOME}/wine/lmos" \
		WINEDEBUG="fixme-all" \
		WINEPATH="C:\\Qt\\Qt5.14.2\\Tools\\mingw730_32\\bin" \
		$(WINE_BINARY) $(WINEPREFIX)/drive_c/Qt/Qt5.14.2/5.14.2/mingw73_32/bin/qmake.exe \
		-o $(BUILD_DIR)/win32/Makefile \
		src\\lmos\\lmos.pro

lmos $(BUILD_DIR)/win32/debug/lmostray.exe $(BUILD_DIR)/win32/release/lmostray.exe:	$(BUILD_DIR)/win32/Makefile $(BUILD_DIR)/win32/debug/platforms $(BUILD_DIR)/win32/release/platforms $(WIN32_LIBBLASTPIT_OBJS) $(WIN32_EXTERNAL_OBJS)
	env WINEARCH="win32" \
		WINEPREFIX="${HOME}/wine/lmos" \
		WINEDEBUG="fixme-all" \
		WINEPATH="C:\\Qt\\Qt5.14.2\\Tools\\mingw730_32\\bin" \
		$(WINE_BINARY) $(WINEPREFIX)/drive_c/Qt/Qt5.14.2/Tools/mingw730_32/bin/mingw32-make.exe -C $(BUILD_DIR)/win32

lmostar:	$(BUILD_DIR)/win32/release/lmostray.exe $(BUILD_DIR)/win32/debug/lmostray.exe tarball

lmosx:		$(BUILD_DIR)/win32/release/lmostray.exe
		env WINEARCH="win32" \
		WINEPREFIX="$(WINEPRE)" \
		WINEDEBUG="fixme-all" \
		WINEPATH="C:\\Qt\\Qt5.14.2\\Tools\\mingw730_32\\bin" \
		$(WINE_BINARY) $(BUILD_DIR)/win32/release/lmostray.exe

lmoslin $(BUILD_DIR)/win32/lmostray: $(BUILD_DIR)/win32/Makefile
	make -C $(BUILD_DIR)/win32

lmoslinx:	$(BUILD_DIR)/win32/lmostray
	LSAN_OPTIONS=suppressions=$(PROJECT_ROOT)/.leak_blacklist.txt $(BUILD_DIR)/win32/lmostray

$(BUILD_DIR)/win32/%.o:	$(LIBBLASTPIT_DIR)/%.c
	zig cc -target x86-windows-gnu -fno-stack-protector -c $(INCFLAGS) -o $@ $^

$(BUILD_DIR)/win32/%.o:	$(SUBMODULES_DIR)/mxml/%.c
	zig cc -target x86-windows-gnu -fno-stack-protector -c $(INCFLAGS) -o $@ $^

$(BUILD_DIR)/win32/%.o:	$(SUBMODULES_DIR)/sds/%.c
	zig cc -target x86-windows-gnu -fno-stack-protector -c $(INCFLAGS) -o $@ $^

$(BUILD_DIR)/win32/%.o:	$(SUBMODULES_DIR)/mongoose/%.c
	zig cc -target x86-windows-gnu -fno-stack-protector -c $(INCFLAGS) -o $@ $^

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


# ░█▀█░█▀▀░█░█░░░█▀▀░░░█░░░█▄█░█▀█░█▀▀
# ░█░█░█▀▀░█▄█░░░█░░░░░█░░░█░█░█░█░▀▀█
# ░▀░▀░▀▀▀░▀░▀░░░▀▀▀░░░▀▀▀░▀░▀░▀▀▀░▀▀▀

lmoscom:	$(LMOS_EXE)

lmos_run:	$(LMOS_EXE)
		WINEARCH="win32" WINEPREFIX=$(WINEPRE) wine $^ #2>/dev/null

# lmos_server:	$(LMOS_START_SCRIPT)
# 		$^ &

lmos_debug:	$(LMOS_EXE)
		WINEPREFIX=$(WINEPRE) winedbg --gdb $^ 2>/dev/null

$(LMOS_OBJ):	$(LMOS_SRC) | $(BUILD_DIR)
		zig cc -target $(PLATFORM) $(FLAGS) -MD -c -o $@ -I$(SRC_DIR)/lmos $^ 

$(LMOS_EXE):	$(LMOS_OBJ)
		zig cc -target $(PLATFORM) $(FLAGS) -o $@ $^ $(LMOS_LIBS)

.PHONY:		idispatch invoke

idispatch:	$(BUILD_DIR)/idispatch.exe
		WINEARCH="win32" WINEPREFIX="$(WINEPRE)" WINEDEBUG="fixme-all" wine $^
$(BUILD_DIR)/idispatch.exe:	$(SRC_DIR)/lmos/idispatch_test.c
		zig cc -target x86-windows-gnu -fno-stack-protector -o $@ -I/home/thf/projects/blastpit/src/lmos $^ -lole32 -loleaut32 -luuid -lwsock32 -MJ compile_commands.json

invoke:		$(BUILD_DIR)/invoke.exe
		WINEARCH="win32" WINEPREFIX="$(WINEPRE)" WINEDEBUG="fixme-all" wine $^
$(BUILD_DIR)/invoke.exe:	$(SRC_DIR)/lmos/invoke_test.cpp | $(BUILD_DIR)
		zig c++ -target x86-windows-gnu -fno-stack-protector -o $@ -I/home/thf/projects/blastpit/src/lmos $^ -lole32 -loleaut32 -luuid -lwsock32 -MJ compile_commands.json

invokec:		$(BUILD_DIR)/invokec.exe
		WINEARCH="win32" WINEPREFIX="$(WINEPRE)" WINEDEBUG="fixme-all" wine $^
$(BUILD_DIR)/invokec.exe:	$(SRC_DIR)/lmos/invoke_test.c | $(BUILD_DIR)
		zig cc -target x86-windows-gnu -fno-stack-protector -o $@ -I/home/thf/projects/blastpit/src/lmos $^ -lole32 -loleaut32 -luuid -lwsock32 -MJ compile_commands.json

iexplore:	$(BUILD_DIR)/iexplore.exe
		WINEARCH="win32" WINEPREFIX="$(WINEPRE)" WINEDEBUG="fixme-all" wine $^
$(BUILD_DIR)/iexplore.exe:	$(SRC_DIR)/lmos/iexplore.cpp
		zig c++ -target x86-windows-gnu -fno-stack-protector -o $@ -I/home/thf/projects/blastpit/src/lmos $^ -lole32 -loleaut32 -luuid -lwsock32 -MJ compile_commands.json










$(BUILD_DIR)/win32/Makefile_idispatch: $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/win32
	env WINEARCH="win32" \
		WINEPREFIX="${HOME}/wine/lmos" \
		WINEDEBUG="fixme-all" \
		WINEPATH="C:\\Qt\\Qt5.14.2\\Tools\\mingw730_32\\bin" \
		$(WINE_BINARY) $(WINEPREFIX)/drive_c/Qt/Qt5.14.2/5.14.2/mingw73_32/bin/qmake.exe \
		-o $@ \
		src\\lmos\\qt_minimal_lmos\\qt_minimal_lmos.pro

lmos_idispatch $(BUILD_DIR)/win32/debug/lmos_idispatch.exe:	$(BUILD_DIR)/win32/Makefile_idispatch $(BUILD_DIR)/win32/debug/platforms $(BUILD_DIR)/win32/release/platforms $(WIN32_LIBBLASTPIT_OBJS) $(WIN32_EXTERNAL_OBJS)
	env WINEARCH="win32" \
		WINEPREFIX="${HOME}/wine/lmos" \
		WINEDEBUG="fixme-all" \
		WINEPATH="C:\\Qt\\Qt5.14.2\\Tools\\mingw730_32\\bin" \
		$(WINE_BINARY) $(WINEPREFIX)/drive_c/Qt/Qt5.14.2/Tools/mingw730_32/bin/mingw32-make.exe -f Makefile_idispatch -C $(BUILD_DIR)/win32

lmos_idispatchx:	$(BUILD_DIR)/win32/debug/lmos_idispatch.exe
		env WINEARCH="win32" \
		WINEPREFIX="$(WINEPRE)" \
		WINEDEBUG="fixme-all" \
		WINEPATH="C:\\Qt\\Qt5.14.2\\Tools\\mingw730_32\\bin" \
		$(WINE_BINARY) $^

# -include $(LMOS_OBJ:.o=.d)
