# var != command - Set var to shell output of comma
# var ?= value - Set var to value if not already set
# var := value - Set var without recursively expanding value
# var = value - Set var recursively expanding value

# Locations
PROJECT_ROOT    := $(shell git rev-parse --show-toplevel)
PROJECT         := $(shell basename $(PROJECT_ROOT))
BUILD_DIR       ?= $(PROJECT_ROOT)/build
# RELATIVE_ROOT   := $(shell realpath --relative-to $(BUILD_DIR) $(PROJECT_ROOT))
SRC_DIR         := $(PROJECT_ROOT)/src
LIBBLASTPIT_DIR := $(SRC_DIR)/libblastpit
SUBMODULES_DIR  := $(SRC_DIR)/submodules
REDIST_DIR      := $(PROJECT_ROOT)/.git/untracked/redist
GIT_HOOKS       := ${PROJECT_ROOT}/$(shell git config --get core.hooksPath)

# User defines
USER_DEFINES  = SPACENAV
CPPFLAGS     += $(addprefix -D, $(USER_DEFINES))
CPPFLAGS     += -pthread -pipe
MAXJOBS      ?= $(shell nproc)

# Debugger
DBG = lldb

# Tools
AR = llvm-ar
RANLIB = llvm-ranlib

# Compiler flags
debug debug_build asan asan_build msan msan_build:	CPPFLAGS += -Wall -Wpedantic -Wextra
debug debug_build asan asan_build msan msan_build:	CPPFLAGS += -Werror
debug debug_build asan asan_build msan msan_build:	CPPFLAGS += -O0 -g3
debug debug_build asan asan_build msan msan_build:	CPPFLAGS += -DDEBUG_LEVEL=1
#debug debug_build asan asan_build msan msan_build:	CPPFLAGS += -DDEBUG_LEVEL=3
debug debug_build asan asan_build msan msan_build:	CPPFLAGS += -MMD

# Enabling Mongoose logging causes Inkscape to complain that the output from the extension cannot be parsed,
# so only enable it when debugging websockets
debug debug_build asan asan_build msan msan_build:	EXTERNAL_CPPFLAGS += -DMG_ENABLE_LOG=0 -DMG_MAX_RECV_BUF_SIZE=10000000

analyze analyze_build:		CC = clang
analyze analyze_build:		CXX = clang++
analyze analyze_build:		CPPFLAGS = -Og -g3

# Use gcc for debugging, clang for sanitizing, and zig for release/cross
debug debug_build:		CC        = clang
debug debug_build:		CXX       = clang++
debug debug_build:		CPPFLAGS += -fno-sanitize=all -Og
debug debug_build:		EXTERNAL_CPPFLAGS += -fno-sanitize=all -Og
#debug debug_build:		BEAR_OPTS = --use-cc "$(CC)" --use-c++ "$(CXX)"

# ...but Clang is necessary for an address/memory sanitizer build 
asan asan_build msan msan_build:	CC        = clang
asan asan_build msan msan_build:	CXX       = clang++
asan asan_build:	SANFLAGS += -fsanitize=undefined,implicit-conversion,nullability,integer
asan asan_build:	SANFLAGS += -fsanitize=address -fsanitize-address-use-after-scope
msan msan_build:	SANFLAGS += -fsanitize=memory -fsanitize-memory-track-origins
asan asan_build msan msan_build:	SANFLAGS += -fno-omit-frame-pointer -fno-optimize-sibling-calls
asan asan_build msan msan_build:	SANFLAGS += -fsanitize-recover=all -fsanitize-blacklist=$(PROJECT_ROOT)/res/.sanitize_blacklist.txt
asan asan_build msan msan_build:	SANLDFLAGS += -Wl,-rpath,$(shell dirname $(shell clang -print-file-name=libclang_rt.ubsan_standalone-x86_64.so))
asan asan_build msan msan_build:	SHARED_SANFLAGS += -shared-libsan
asan asan_build msan msan_build:	EXTERNAL_CPPFLAGS = -Og -g3 -w

release release_build:	CC = gcc #zig cc #-target x86_64-linux-musl
release release_build:	CXX = g++ #zig c++ #-target x86_64-linux-musl
release release_build:	CPPFLAGS += -fno-sanitize=all -Ofast #-flto causes segfaults in getopt and others
release release_build:	CPPFLAGS += -Wfatal-errors
release release_build:	CPPFLAGS += -DDEBUG_LEVEL=1
release release_build:	EXTERNAL_CPPFLAGS += -fno-sanitize=all -Ofast -w # lto causes problems with unity
release release_build:	EXTERNAL_CPPFLAGS += -DMG_ENABLE_LOG=0 # Mongoose log messages can bork the returned Inkscape XML

# Still useful to debug release crashes. Strip when everything is fully working
release release_build:	CPPFLAGS += -g3
release release_build:	EXTERNAL_CPPFLAGS += -g3

profile profile_build: 	TRACY_OBJS = $(BUILD_DIR)/TracyClient.o
profile profile_build: 	CC = g++ # Needed for tracy
profile profile_build: 	CXX = g++
profile profile_build: 	CPPFLAGS += -fno-sanitize=all -fpermissive -Wfatal-errors
# profile profile_build: 	CPPFLAGS += -pg --coverage -fpermissive
profile profile_build: 	CPPFLAGS += -Ofast
profile profile_build: 	EXTERNAL_CPPFLAGS = -fno-sanitize=all -Ofast -fpermissive
profile profile_build:	CPPFLAGS += -DTRACY_ENABLE
profile profile_build:	INCFLAGS += -I$(SUBMODULES_DIR)/tracy

# cross $(BUILD_DIR)/win32/libblastpit.a:	$(BUILD_DIR)/win32
cross cross_build:	CC = zig cc -target i386-windows-gnu -fno-sanitize=all -Ofast
cross cross_build:	CXX = zig c++ -target i386-windows-gnu -fno-sanitize=all -Ofast
cross cross_build:	CPPFLAGS += -fno-stack-protector -D_GLIBCXX_USE_CXX_ABI=1
cross cross_build:	EXTERNAL_CPPFLAGS += -fno-stack-protector -D_GLIBCXX_USE_CXX_ABI=1
cross cross_build:	WIN32FLAGS += -lws2_32
# cross cross_build:	LDFLAGS += -lc++
cross cross_build:	DISABLE_STEAMCONTROLLER="1"
cross cross_build:	EXTERNAL_CPPFLAGS += -DMG_ENABLE_LOG=0 -DMG_MAX_RECV_BUF_SIZE=10000000
cross cross_build:	BUILD_DIR := $(PROJECT_ROOT)/build/win32

# Library source and object files
LIBMXML_SRCS        := mxml-attr.c mxml-entity.c mxml-file.c mxml-get.c mxml-index.c mxml-node.c mxml-private.c mxml-search.c mxml-set.c mxml-string.c 
LIBMXML_OBJS        := $(patsubst %.c,$(BUILD_DIR)/%.o,$(LIBMXML_SRCS))
LIBBLASTPIT_SOURCES := blastpit.c websocket.c xml.c
LIBBLASTPIT_OBJS    := $(LIBMXML_OBJS) $(BUILD_DIR)/sds.o $(BUILD_DIR)/mongoose.o $(patsubst %.c,$(BUILD_DIR)/%.o,$(LIBBLASTPIT_SOURCES))
LIBBLASTPIT_SRCS    := $(patsubst %.c,$(LIBBLASTPIT_DIR)/%.c,$(LIBBLASTPIT_SOURCES)) $(SUBMODULES_DIR)/sds/sds.c
LIBBLASTPIT_TARGETS := $(BUILD_DIR)/libblastpit.a $(BUILD_DIR)/_blastpy.so $(BUILD_DIR)/blastpy.py 

# Unit test code (unity)
UNITY_FIXTURE_DIR = $(SUBMODULES_DIR)/unity/extras/fixture/src
UNITY_DIR         = $(SUBMODULES_DIR)/unity/src
UNITY_OBJS        = $(BUILD_DIR)/unity.o $(BUILD_DIR)/unity_fixture.o
UNITY_DEFS        = -DUNITY_OUTPUT_COLOR -DUNITY_FIXTURE_NO_EXTRAS

# Includes
INCFLAGS = -I $(PROJECT_ROOT)/.git/untracked -I$(UNITY_DIR) -I$(UNITY_FIXTURE_DIR) -I$(LIBBLASTPIT_DIR) -I $(PROJECT_ROOT)/res/cfg -I$(SUBMODULES_DIR)/mongoose -I$(SUBMODULES_DIR)/sds -I$(SUBMODULES_DIR)/mxml

# Python inclues for SWIG
PYTHON_INCS = $(shell python-config --includes)

# Libraries
LIBS  = -lm -ldl

# Blastpy
BLASTPY_FILES  = $(BUILD_DIR)/blastpy_wrap.o

# Debugging
DEBUG_COMMAND ?= $(shell head -n1 .debugcmd)
DEBUG_TARGET  ?= $(shell tail -n1 .debugcmd)

FORMAT_FILES        = src/libblastpit/*.{h,c} src/lmos/{lmos-tray,lmos,parser}.{hpp,cpp} src/lmos/{main,traysettings}.cpp src/video/*.{h,c,cpp}
FORMAT_FILES_PYTHON = res/bin/*.py src/libblastpit/*.py src/inkscape/*.py
FORMAT_FILES_XML    = src/inkscape/*.inx
FORMAT_FILES_HTML   = doc/reference_manuals/lmos.html

TIDY_FILES          = src/lmos/parser.cpp

# Unit Tests
# TEST_SOURCES  = t_linkedlist.c t_message.c t_xml.c t_websocket.c t_client_command.c
TEST_SOURCES = $(notdir $(wildcard $(LIBBLASTPIT_DIR)/t_*.c))
TEST_BINARIES = $(patsubst %.c,$(BUILD_DIR)/%_x,$(TEST_SOURCES))


.PHONY:	all clean debug release debug_build release_build cross cross_build test asan asan_build msan msan_build test_build cli ebuild profile profile_build lmosx qmake lmos

# We have to wrap debug_build and release_build below because
# the secondary call to make would otherwise forget
# the target-specific variables defined above
all: 		debug test

test:
		$(MAKE) -j$(MAXJOBS) -f $(PROJECT_ROOT)/Makefile test_run

debug: 		$(BUILD_DIR) .tags
		bear $(BEAR_OPTS) --append -- $(MAKE) -j$(MAXJOBS) -f $(PROJECT_ROOT)/Makefile debug_build
		# $(MAKE) -j$(MAXJOBS) -f $(PROJECT_ROOT)/Makefile test_run

analyze:	clean
		scan-build --status-bugs -maxloop 20 --force-analyze-debug-code --exclude $(SUBMODULES_DIR) --show-description $(MAKE) -j$(MAXJOBS) -f $(PROJECT_ROOT)/Makefile analyze_build

asan:	$(BUILD_DIR) .tags
		bear $(BEAR_OPTS) --append -- $(MAKE) -j$(MAXJOBS) -f $(PROJECT_ROOT)/Makefile asan_build

msan:	$(BUILD_DIR) .tags
		bear $(BEAR_OPTS) --append -- $(MAKE) -j$(MAXJOBS) -f $(PROJECT_ROOT)/Makefile msan_build

release:	$(BUILD_DIR)
		$(MAKE) -j$(MAXJOBS) -f $(PROJECT_ROOT)/Makefile release_build 
		# $(MAKE) -j$(MAXJOBS) -f $(PROJECT_ROOT)/Makefile test_run

profile:	$(BUILD_DIR)
		$(MAKE) -j$(MAXJOBS) -f $(PROJECT_ROOT)/Makefile profile_build

cross:		pngs
		BUILD_DIR=$(PROJECT_ROOT)/build/win32 $(MAKE) -j$(MAXJOBS) -f $(PROJECT_ROOT)/Makefile cross_build

alltargetscheck:
		make clean
		make debug
		make test
		# make test_inkscape
		make clean
		make release
		make test
		# make test_inkscape
		make clean
		make asan
		make test_asan
		make clean
		make msan
		make test_msan
		make clean
		make cross
		make wscli_portable

targets:	$(BUILD_DIR) $(LIBBLASTPIT_OBJS) $(UNITY_OBJS) $(TEST_BINARIES)

python_targets:	$(BUILD_DIR)/_blastpy.so $(BUILD_DIR)/blastpy.py

debug_build:	targets ueye cameras wscli images python_targets

analyze_build:	targets

asan_build:	targets

msan_build:	targets

release_build:	targets python_targets #ueye cameras wscli

.SECONDEXPANSION:
profile_build:	$(LIBBLASTPIT_OBJS) $(UNITY_OBJS) $(TEST_BINARIES) $$(TRACY_OBJS)

test_build:	targets

cross_build:	$(BUILD_DIR) $(LIBBLASTPIT_OBJS)

ebuild:		$(BUILD_DIR) $(LIBBLASTPIT_OBJS)

clean:
	rm -r $(BUILD_DIR)/__pycache__/* $(BUILD_DIR)/win32/{debug,release}/* $(BUILD_DIR)/win32/* $(BUILD_DIR)/* || /bin/true
	rm -r $(PROJECT_ROOT)/.{cache,ccls-cache,pytest_cache} || /bin/true
	rm -f $(PROJECT_ROOT)/{.tags,compile_command.json} || /bin/true
	rmdir $(BUILD_DIR)/__pycache__ $(BUILD_DIR)/win32/{debug,release} $(BUILD_DIR)/win32 $(BUILD_DIR) || /bin/true
	#$(MAKE) .tags
	# pkill wscli || /bin/true

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	# ln -fs $(PROJECT_ROOT)/.git/untracked/myconfig.py $(BUILD_DIR)/

# Libblastpit Recipes
$(BUILD_DIR)/sds.o:	$(SUBMODULES_DIR)/sds/sds.c
	$(CC) $(EXTERNAL_CPPFLAGS) $(SANFLAGS) $(INCFLAGS) -c -fPIC $^ -o $@

# $(BUILD_DIR)/libblastpit.a: $(LIBBLASTPIT_OBJS) $(LIBMXML_OBJS) $(STEAM_OBJS)
# 	$(AR) -crs $@ $^
# 	$(RANLIB) $@
 
$(BUILD_DIR)/blastpy.py:	$(BUILD_DIR)/blastpy_wrap.c

$(BUILD_DIR)/blastpy_wrap.c:	$(LIBBLASTPIT_OBJS) # This forces rerunning swig on blastpit change
	swig -o $(BUILD_DIR)/blastpy_wrap.c -python $(PROJECT_ROOT)/src/libblastpit/blastpy.i

$(BUILD_DIR)/blastpy_wrap.o:	$(BUILD_DIR)/blastpy_wrap.c
	$(CC) -fPIC -I$(PROJECT_ROOT)/src/libblastpit -I$(SUBMODULES_DIR)/mongoose -o $@ -c $^ $(PYTHON_INCS)

$(BUILD_DIR)/_blastpy.so:	$(BUILD_DIR)/blastpy_wrap.o $(BLASTPY_FILES)
	$(CC) $(CPPFLAGS) -shared $(SANLDFLAGS) $(SANFLAGS) $(SHARED_SANFLAGS) $(BLASTPY_FILES) -o $@ $(BLASTPY_LIBS) $(LIBBLASTPIT_OBJS)

$(BUILD_DIR)/unity_fixture.o: $(UNITY_FIXTURE_DIR)/unity_fixture.c
	$(CC) $(EXTERNAL_CPPFLAGS) $(SANFLAGS) $(UNITY_DEFS) $(INCFLAGS) -c $^ -o $@

$(BUILD_DIR)/unity.o:	$(UNITY_DIR)/unity.c
	$(CC) $(EXTERNAL_CPPFLAGS) $(SANFLAGS) $(UNITY_DEFS) $(INCFLAGS) -c $^ -o $@

$(BUILD_DIR)/TracyClient.o:	$(SUBMODULES_DIR)/tracy/TracyClient.cpp
	$(CXX) $(CPPFLAGS) $(INCFLAGS) -c -fPIC $^ -o $@

$(BUILD_DIR)/t_%_x:	$(BUILD_DIR)/t_%.o $(LIBBLASTPIT_OBJS) $(UNITY_OBJS) $$(TRACY_OBJS)
	$(CC) $(CPPFLAGS) $(INCFLAGS) $(UNITY_DEFS) $(SANFLAGS) -L. $^ -o $@ $(LIBS) $(LDFLAGS)

$(BUILD_DIR)/t_%.o:	$(LIBBLASTPIT_DIR)/t_%.c
	$(CC) $(CPPFLAGS) $(SANFLAGS) $(INCFLAGS) $(UNITY_DEFS) -c -fPIC $^ -o $@

$(BUILD_DIR)/%.o:	$(LIBBLASTPIT_DIR)/%.c
	$(CC) $(CPPFLAGS) $(SANFLAGS) $(INCFLAGS) $(UNITY_DEFS) -c -fPIC $^ -o $@

$(BUILD_DIR)/%.o:	$(SUBMODULES_DIR)/mongoose/%.c
	$(CC) $(EXTERNAL_CPPFLAGS) $(SANFLAGS) $(INCFLAGS) $(UNITY_DEFS) -c -fPIC $^ -o $@

$(BUILD_DIR)/%.o:	$(SUBMODULES_DIR)/mxml/%.c
	$(CC) $(EXTERNAL_CPPFLAGS) $(SANFLAGS) $(INCFLAGS) -c -fPIC $^ -o $@

$(BUILD_DIR)/%.o:	%.c
	$(CC) $(CPPFLAGS) $(SANFLAGS) $(INCFLAGS) $(UNITY_DEFS) -c -fPIC $^ -o $@

$(BUILD_DIR)/%.o:	%.cpp
	$(CXX) $(CPPFLAGS) $(SANFLAGS) $(INCFLAGS) -c -fPIC $^ -o $@

# Install
python:	$(BUILD_DIR)/blastpy.py $(BUILD_DIR)/_blastpy.so
	$(MAKE) -C $(BUILD_DIR) -f $(PROJECT_ROOT)/Makefile python_install

python_install:
	python $(LIBBLASTPIT_DIR)/setup.py install --user


# Testing and Debugging
test_run:
	@UBSAN_OPTIONS=print_stacktrace=1 sh -c $(PROJECT_ROOT)/res/bin/testrunner.sh
	@sh -c "if command -v passed &> /dev/null; then passed; fi"

test_asan:
	@sh -c "$(PROJECT_ROOT)/res/bin/testrunner.sh -asan"
	passed

test_msan:
	@sh -c "$(PROJECT_ROOT)/res/bin/testrunner.sh -msan"
	passed

test_inkscape:
	pytest -vv $(SRC_DIR)/inkscape

.tags:
	@(${GIT_HOOKS}/ctags >/dev/null 2>&1 &)

# Formatting
format:
	clang-format -style=file -i $(FORMAT_FILES) || /bin/true
	black --line-length 80 $(FORMAT_FILES_PYTHON)

tidy:
	clang-tidy --config-file=$(PROJECT_ROOT)/res/.clang-tidy $(TIDY_FILES)

# Documentation and Resource Generation
$(PROJECT_ROOT)/src/lmos/lmos-tray.ico:	$(PROJECT_ROOT)/res/img/blastpit.svg
	convert -background none -define icon:auto-resize=256,128,64,48,32,16 $^ $@

%.ico:	%.svg
	convert -background none -define icon:auto-resize=256,128,64,48,32,16 $^ $@

%.png:	%.svg
	rsvg-convert --width 256 --height 256 --format png $^ > $@

images:	pngs $(PROJECT_ROOT)/src/lmos/lmos-tray.ico

pngs:	$(PROJECT_ROOT)/res/img/laseractive.png $(PROJECT_ROOT)/res/img/noconnection.png $(PROJECT_ROOT)/res/img/nolaser.png $(PROJECT_ROOT)/res/img/blastpit.png


# Cross Compiling and Emulation

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
		$(WINE_BINARY) $(WINEPREFIX)/drive_c/Qt/Qt5.14.2/Tools/mingw730_32/bin/mingw32-make.exe -C $(BUILD_DIR)/win32 -j$(MAXJOBS)

lmosrelease:	$(BUILD_DIR)/release/lmostray.exe tarball

lmosx:
	env WINEARCH="win32" \
		WINEPREFIX="${HOME}/.wine/32bit" \
		WINEPATH="C:\\Qt\\Qt5.14.2\\Tools\\mingw730_32\\bin" \
		$(WINE_BINARY) $(BUILD_DIR)/win32/release/lmostray.exe

lmoslin $(BUILD_DIR)/lmostray: $(BUILD_DIR)/Makefile
	make -C $(BUILD_DIR) -j$(MAXJOBS)

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

$(BUILD_DIR)/wscli:	$(LIBBLASTPIT_OBJS)
	$(CC) $(CPPFLAGS) $(SANFLAGS) -c -I $(LIBBLASTPIT_DIR)/libblastpit -I $(SUBMODULES_DIR)/mongoose -o $(BUILD_DIR)/wscli.o -x c $(LIBBLASTPIT_DIR)/wscli.c
	$(CC) $(CPPFLAGS) $(SANFLAGS) -o $(BUILD_DIR)/wscli $(BUILD_DIR)/wscli.o $(LIBBLASTPIT_OBJS) $(WIN32FLAGS)
#	-lc++

server:	$(BUILD_DIR)/wscli
	$^ -s $(shell grep WS_SERVER_LOCAL res/cfg/myconfig.py | awk '{print $$3}' | sed s/\"//g)

wscli:	$(BUILD_DIR) $(BUILD_DIR)/wscli
ueye:	$(BUILD_DIR) $(BUILD_DIR)/ueye

wscli_portable:
	make clean
	# Disable undefined behaviour sanitiser in zig since Mongoose fails
	CC="zig cc -target i386-linux-musl -Og -g3 -fno-sanitize=all" CXX="zig c++ -target i386-linux-musl -Og -g3 -fno-sanitize=all" CPPFLAGS="-Og -g3 -fno-sanitize=all" make wscli

debug_wscli_portable:
	qemu-i386 -g 9999 -- $(BUILD_DIR)/wscli -s ws://0.0.0.0:8000 &
	gdb -ex "file build/wscli" -ex "target remote :9999"

$(BUILD_DIR)/ueye_opencv.o:	$(SRC_DIR)/video/ueye_opencv.cpp
	bear $(BEAR_OPTS) --append -- g++ -c -I $(LIBBLASTPIT_DIR) -I $(REDIST_DIR) $(shell pkg-config opencv4 --cflags) -o $@ $^

$(BUILD_DIR)/ueye:	$(LIBBLASTPIT_OBJS) $(SRC_DIR)/video/ueye.c $(SRC_DIR)/video/ueye_opencv.h $(BUILD_DIR)/ueye_opencv.o
	bear $(BEAR_OPTS) --append -- g++ -I $(LIBBLASTPIT_DIR) -I $(REDIST_DIR) $(shell pkg-config opencv4 --cflags) -o $@ $(SRC_DIR)/video/ueye.c -L $(REDIST_DIR) $(BUILD_DIR)/ueye_opencv.o -lueye_api -pthread $(shell pkg-config opencv4 --libs) $(LIBBLASTPIT_OBJS)

# Camera

cameras:	$(BUILD_DIR) $(BUILD_DIR)/veho $(BUILD_DIR)/eakins $(BUILD_DIR)/orbitmp $(BUILD_DIR)/canny

# TODO: Find out why we cannot use 'zig c++' here (fails to link)
$(BUILD_DIR)/veho:	$(SRC_DIR)/video/capture.cpp
	g++ -DCAPTURE_DEVICE=\"/dev/v4l/by-id/usb-Vimicro_Co._ltd_Vimicro_USB2.0_UVC_PC_Camera-video-index0\" -DCAPTURE_X_RESOLUTION=640 -DCAPTURE_Y_RESOLUTION=480 -DCAMERA_NAME=\"veho\" -std=c++11 $(CXXFLAGS) -o $@ $(shell pkg-config opencv4 --cflags) $^ $(shell pkg-config opencv4 --libs)

$(BUILD_DIR)/eakins:	 $(SRC_DIR)/video/capture.cpp
	g++ -DCAPTURE_DEVICE=\"/dev/v4l/by-id/usb-VXIS_Inc_ezcap_U3_capture-video-index0\" -DCAPTURE_X_RESOLUTION=1920 -DCAPTURE_Y_RESOLUTION=1080 -DCAMERA_NAME=\"eakins\" -std=c++11 $(CXXFLAGS) -o $@ $(shell pkg-config opencv4 --cflags) $^ $(shell pkg-config opencv4 --libs)

$(BUILD_DIR)/orbitmp:	$(SRC_DIR)/video/capture.cpp
	g++ -DCAPTURE_DEVICE=\"/dev/v4l/by-id/usb-046d_08cc_11CDE8A2-video-index0\" -DCAPTURE_X_RESOLUTION=640 -DCAPTURE_Y_RESOLUTION=480 -DCAMERA_NAME=\"orbitmp\" -std=c++11 $(CXXFLAGS) -o $@ $(shell pkg-config opencv4 --cflags) $^ $(shell pkg-config opencv4 --libs)

# Canny edge detector demo
$(BUILD_DIR)/canny:	$(SRC_DIR)/video/canny.cpp
	g++ $(CXXFLAGS) -o $@ $(shell pkg-config opencv4 --cflags) $^ $(shell pkg-config opencv4 --libs)

# Packaging

tarball:	lmos
		cd build/release && \
		zip -r /tmp/lmos.zip \
		--exclude=\*.o \
		--exclude=\*.h \
		--exclude=\*.cpp \
		Qt5Core.dll Qt5Gui.dll Qt5Widgets.dll lmostray.exe platforms && \
		cd $(WINEPREFIX)/drive_c/Qt/Qt5.14.2/Tools/mingw730_32/i686-w64-mingw32/lib && \
		zip /tmp/lmos.zip \
		libstdc++-6.dll libgcc_s_dw2-1.dll libwinpthread-1.dll
