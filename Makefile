# var != command - Set var to shell output of comma
# var ?= value - Set var to value if not already set
# var := value - Set var without recursively expanding value
# var = value - Set var recursively expanding value


# Locations
PROJECT_ROOT    := $(shell git rev-parse --show-toplevel)
PROJECT         := $(shell basename $(PROJECT_ROOT))
BUILD_DIR       := $(PROJECT_ROOT)/build
# RELATIVE_ROOT   := $(shell realpath --relative-to $(BUILD_DIR) $(PROJECT_ROOT))
LIBBLASTPIT_DIR := $(PROJECT_ROOT)/src/libblastpit
SUBMODULES_DIR  := $(PROJECT_ROOT)/src/submodules
GIT_HOOKS       := ${PROJECT_ROOT}/$(shell git config --get core.hooksPath)

# User defines
USER_DEFINES  = SPACENAV MG_ENABLE_FILESYSTEM=1
CPPFLAGS     += $(addprefix -D, $(USER_DEFINES))
CPPFLAGS     += -pthread -pipe
MAXJOBS      ?= $(shell nproc)

# Linker
LD      = ld.lld
LDFLAGS = -fuse-ld=lld
# LDFLAGS = -fuse-ld=gold -Wl,--threads

# Debugger
DBG = lldb

# Tools
AR = llvm-ar
RANLIB = llvm-ranlib

# Compiler flags
debug_build:	CPPFLAGS += -Wall -Wpedantic -Wextra
debug_build:	CPPFLAGS += -Werror -Wfatal-errors
debug_build:	CPPFLAGS += -Og -g3
debug_build:	CPPFLAGS += -DDEBUG_LEVEL=5
debug_build:	CC        = zig cc
debug_build:	CXX       = zig c++
# debug_build:	CC        = ccache clang
# debug_build:	CXX       = ccache clang++
# debug_build: 	SANFLAGS += -fsanitize=undefined,implicit-conversion,nullability,integer
# debug_build: 	SANFLAGS += -fsanitize=address -fsanitize-address-use-after-scope
# debug_build: 	SANFLAGS += -fsanitize=memory -fsanitize-memory-track-origins
debug_build:	SANFLAGS += -fno-omit-frame-pointer -fno-optimize-sibling-calls
# debug_build:	SANFLAGS += -fsanitize-recover=all -fsanitize-blacklist=$(PROJECT_ROOT)/.sanitize_blacklist.txt
# debug_build:	SANLDFLAGS += -Wl,-rpath,$(shell dirname $(shell clang -print-file-name=libclang_rt.ubsan_standalone-x86_64.so))
# debug_build: 	SHARED_SANFLAGS += -shared-libsan
debug_build:	EXTERNAL_CPPFLAGS = -Og -g3 -w

release_build:	CC = zig cc #-target x86_64-linux-musl
release_build:	CXX = zig c++ #-target x86_64-linux-musl
# TODO: Using anything other than -O0 here causes unit tests to fail
release_build:	CPPFLAGS += -fno-sanitize=all -O0 #-flto causes segfaults in getopt and others
release_build:	EXTERNAL_CPPFLAGS += -fno-sanitize=all -Ofast -w # lto causes problems with unity

# Still useful to debug release crashes. Strip when everything is fully working
release_build:	CPPFLAGS += -g3
release_build:	EXTERNAL_CPPFLAGS += -g3

ebuild:		CC = zig cc -target x86_64-linux-gnu
ebuild:		CXX = zig c++ -target x86_64-linux-gnu
ebuild:		CPPFLAGS += -march=x86_64

profile: 	CPPFLAGS += -pg --coverage

# Library source and object files
LIBMXML_SRCS        := mxml-attr.c mxml-entity.c mxml-file.c mxml-get.c mxml-index.c mxml-node.c mxml-private.c mxml-search.c mxml-set.c mxml-string.c 
LIBMXML_OBJS        := $(patsubst %.c,%.o,$(LIBMXML_SRCS))
LIBBLASTPIT_SOURCES := blastpit.c websocket.c xml.c
LIBBLASTPIT_OBJS    := $(patsubst %.c,%.o,$(LIBBLASTPIT_SOURCES)) sds.o mongoose.o
LIBBLASTPIT_SRCS    := $(patsubst %.c,$(LIBBLASTPIT_DIR)/%.c,$(LIBBLASTPIT_SOURCES)) $(SUBMODULES_DIR)/sds/sds.c
LIBBLASTPIT_TARGETS := libblastpit.a _blastpy.so blastpy.py 

# Unit test code (unity)
UNITY_FIXTURE_DIR = $(SUBMODULES_DIR)/unity/extras/fixture/src
UNITY_DIR         = $(SUBMODULES_DIR)/unity/src
UNITY_OBJS        = unity.o unity_fixture.o
UNITY_DEFS        = -DUNITY_OUTPUT_COLOR -DUNITY_FIXTURE_NO_EXTRAS

# Includes
INCFLAGS = -I.  -I$(UNITY_DIR) -I$(UNITY_FIXTURE_DIR) -I$(LIBBLASTPIT_DIR) -I$(SUBMODULES_DIR)/mongoose -I$(SUBMODULES_DIR)/sds -I$(SUBMODULES_DIR)/mxml

# Python inclues for SWIG
PYTHON_INCS = $(shell python-config --includes)

# Libraries
LIBS  = -lm

# Blastpy
BLASTPY_FILES  = blastpy_wrap.o 
# BLASTPY_SRCS   = $(patsubst %.o,$(SRCDIR)/%.c,$(BLASTPY_FILES))
BLASTPY_FILES += libblastpit.a
BLASTPY_LIBS   = 

# Debugging
DEBUG_COMMAND ?= $(shell head -n1 .debugcmd)
DEBUG_TARGET  ?= $(shell tail -n1 .debugcmd)

FORMAT_FILES        = src/libblastpit/*.{h,c,cpp} src/lmos/{lmos-tray,lmos,parser}.{hpp,cpp} src/lmos/{main,traysettings}.cpp src/video/*.cpp
FORMAT_FILES_PYTHON = res/bin/*.py src/libblastpit/*.py src/inkscape/*.py
FORMAT_FILES_XML    = src/inkscape/*.inx
FORMAT_FILES_HTML   = doc/reference_manuals/lmos.html

# Unit Tests
# TEST_SOURCES  = t_linkedlist.c t_message.c t_xml.c t_websocket.c t_client_command.c
TEST_SOURCES = $(notdir $(wildcard $(LIBBLASTPIT_DIR)/t_*.c))
TEST_BINARIES = $(patsubst %.c,%_x,$(TEST_SOURCES))



.PHONY:	all clean debug release debug_build release_build cross cli ebuild

# We have to wrap debug_build and release_build below because
# the secondary call to make would otherwise forget
# the target-specific variables defined above
test:		debug
		$(MAKE) -j$(MAXJOBS) -C $(BUILD_DIR) -f $(PROJECT_ROOT)/Makefile test_run

all: 		debug
		# @printf "\033k$(PROJECT)\033\\"

debug: 		$(BUILD_DIR) .tags
		bear -a $(MAKE) -j$(MAXJOBS) -C $(BUILD_DIR) -f $(PROJECT_ROOT)/Makefile debug_build

release:	$(BUILD_DIR)
		$(MAKE) -j$(MAXJOBS) -C $(BUILD_DIR) -f $(PROJECT_ROOT)/Makefile release_build 
		$(MAKE) -j$(MAXJOBS) -C $(BUILD_DIR) -f $(PROJECT_ROOT)/Makefile test_run

targets:	$(BUILD_DIR) $(LIBBLASTPIT_TARGETS) _blastpy.so $(UNITY_OBJS) $(TEST_BINARIES) cli images

debug_build:	targets

release_build:	targets

test_build:	targets

ebuild:		$(LIBBLASTPIT_TARGETS)

clean:
	rm -rf $(BUILD_DIR)/{{__pycache__,win32,docs},*.{c,a,so,o,py},*_x,wscli} compile_commands.json .tags{,extra} .clangd || /bin/true
	pkill wscli || /bin/true

$(BUILD_DIR) $(BUILD_DIR)/win32:
	mkdir -p $(BUILD_DIR)/win32
	# ln -fs $(PROJECT_ROOT)/.git/untracked/myconfig.py $(BUILD_DIR)/

# Libblastpit Recipes
sds.o:	$(SUBMODULES_DIR)/sds/sds.c
	$(CC) $(CPPFLAGS) $(SANFLAGS) $(INCFLAGS) -c -fPIC $^ -o $@

libblastpit.a: $(LIBBLASTPIT_OBJS) $(LIBMXML_OBJS) $(STEAM_OBJS)
	$(AR) -crs $@ $^
	$(RANLIB) $@
 
blastpy.py:	blastpy_wrap.c

blastpy_wrap.c:	libblastpit.a # This forces rerunning swig on blastpit change
	swig -o blastpy_wrap.c -python $(PROJECT_ROOT)/src/libblastpit/blastpy.i

blastpy_wrap.o:	blastpy_wrap.c
	$(CC) -fPIC -I$(PROJECT_ROOT)/src/libblastpit -I$(SUBMODULES_DIR)/mongoose -o $@ -c $^ $(PYTHON_INCS)

_blastpy.so:	blastpy_wrap.o $(BLASTPY_FILES)
	$(CXX) $(CPPFLAGS) -shared $(SANLDFLAGS) $(SANFLAGS) $(SHARED_SANFLAGS) $(BLASTPY_FILES) -o $@ $(BLASTPY_LIBS)

unity_fixture.o: $(UNITY_FIXTURE_DIR)/unity_fixture.c
	$(CC) $(EXTERNAL_CPPFLAGS) $(SANFLAGS) $(UNITY_DEFS) $(INCFLAGS) -c $< -o $@

unity.o:	$(UNITY_DIR)/unity.c
	$(CC) $(EXTERNAL_CPPFLAGS) $(SANFLAGS) $(UNITY_DEFS) $(INCFLAGS) -c $< -o $@

t_%_x:	t_%.o $(UNITY_OBJS) libblastpit.a
	$(CXX) $(CPPFLAGS) $(INCFLAGS) $(UNITY_DEFS) $(SANFLAGS) $(UNITY_OBJS) $< -L. -o $@ $(LIBS) $(BUILD_DIR)/libblastpit.a $(LDFLAGS)

%.o:	$(LIBBLASTPIT_DIR)/%.c
	$(CC) $(CPPFLAGS) $(SANFLAGS) $(INCFLAGS) $(UNITY_DEFS) -c -fPIC $^ -o $@

%.o:	$(SUBMODULES_DIR)/mongoose/%.c
	$(CC) $(EXTERNAL_CPPFLAGS) $(SANFLAGS) $(INCFLAGS) $(UNITY_DEFS) -c -fPIC $^ -o $@

%.o:	$(SUBMODULES_DIR)/mxml/%.c
	$(CC) $(EXTERNAL_CPPFLAGS) $(SANFLAGS) $(INCFLAGS) -c -fPIC $^ -o $@

%.o:	%.c
	$(CC) $(CPPFLAGS) $(SANFLAGS) $(INCFLAGS) $(UNITY_DEFS) -c -fPIC $^ -o $@

%.o:	%.cpp
	$(CXX) $(CPPFLAGS) $(SANFLAGS) $(INCFLAGS) -c -fPIC $^ -o $@


# Install
python:	$(BUILD_DIR)/blastpy.py $(BUILD_DIR)/_blastpy.so
	$(MAKE) -C $(BUILD_DIR) -f $(PROJECT_ROOT)/Makefile python_install

python_install:
	python $(LIBBLASTPIT_DIR)/setup.py install --user


# Testing and Debugging
test_run:
	@UBSAN_OPTIONS=print_stacktrace=1 sh -c $(PROJECT_ROOT)/res/bin/testrunner.sh
	passed

.tags:
	@(${GIT_HOOKS}/ctags >/dev/null 2>&1 &)

# Formatting
format:
	clang-format -style=file -i $(FORMAT_FILES) || /bin/true


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
# 	make wine
#
# from within 32-bit chroot
#
# Change setting in lmos.pro to debug_and_release for release version

WINE_ARCH = "win32"
WINE_LIBS = "C:\\Qt\\Qt5.14.2\\Tools\\mingw730_32\\bin"
WINE_CXX = $(WINE_LIBS)"\\g++.exe"
QT_DLL_DIR = "Qt/Qt5.14.2/5.14.2/mingw730_32/bin"

# Qt (Wine) - Must be run from within the 32-bit environment
$(BUILD_DIR)/win32/debug/platforms $(BUILD_DIR)/win32/release/platforms:
	mkdir -p $(BUILD_DIR)/win32/{release,debug}/platforms
	cp -v $(WINEPREFIX)/drive_c/Qt/Qt5.14.2/5.14.2/mingw73_32/bin/Qt5{Core,Gui,Widgets}.dll $(BUILD_DIR)/win32/release/
	cp -v $(WINEPREFIX)/drive_c/Qt/Qt5.14.2/5.14.2/mingw73_32/plugins/platforms/qwindows.dll $(BUILD_DIR)/win32/release/platforms
	cp -v $(WINEPREFIX)/drive_c/Qt/Qt5.14.2/5.14.2/mingw73_32/bin/Qt5{Core,Gui,Widgets}.dll $(BUILD_DIR)/win32/debug/
	cp -v $(WINEPREFIX)/drive_c/Qt/Qt5.14.2/5.14.2/mingw73_32/plugins/platforms/qwindows.dll $(BUILD_DIR)/win32/debug/platforms

qmake $(BUILD_DIR)/win32/Makefile: $(BUILD_DIR)
	git describe --long --dirty --always > $(BUILD_DIR)/git_version.txt
	git branch --show-current > $(BUILD_DIR)/git_branch.txt
	git log -1 --pretty=format:%s > $(BUILD_DIR)/git_description.txt
	git show -s --format=%ci > $(BUILD_DIR)/git_date.txt
	env WINEARCH="win32" \
		WINEPATH="C:\\Qt\\Qt5.14.2\\Tools\\mingw730_32\\bin" \
		wine $(WINEPREFIX)/drive_c/Qt/Qt5.14.2/5.14.2/mingw73_32/bin/qmake.exe \
		-o $(BUILD_DIR)/win32/Makefile \
		src\\lmos\\lmos.pro

lmos $(BUILD_DIR)/win32/debug/lmostray.exe $(BUILD_DIR)/win32/release/lmostray.exe:	$(BUILD_DIR)/win32/Makefile $(BUILD_DIR)/win32/debug/platforms $(BUILD_DIR)/win32/release/platforms $(BUILD_DIR)/win32/libblastpit.a
	env WINEARCH="win32" \
		WINEPATH="C:\\Qt\\Qt5.14.2\\Tools\\mingw730_32\\bin" \
		wine $(WINEPREFIX)/drive_c/Qt/Qt5.14.2/Tools/mingw730_32/bin/mingw32-make.exe -C $(BUILD_DIR)/win32 -j$(MAXJOBS)

lmosrelease:	$(BUILD_DIR)/win32/release/lmostray.exe tarball

lmosx:	$(BUILD_DIR)/win32/release/lmostray.exe
	env WINEARCH="win32" \
		WINEPATH="C:\\Qt\\Qt5.14.2\\Tools\\mingw730_32\\bin" \
		wine $^

$(BUILD_DIR)/Makefile:	$(PROJECT_ROOT)/src/lmos/lmos.pro
	qmake -o $@ $(PROJECT_ROOT)/src/lmos/lmos.pro

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


cross $(BUILD_DIR)/win32/libblastpit.a:	$(BUILD_DIR)/win32
	CC="zig cc -target i386-windows-gnu -fno-sanitize=all -Ofast" \
	   CXX="zig c++ -target i386-windows-gnu -fno-sanitize=all -std=c++11 -Ofast" \
	   CPPFLAGS="-fno-stack-protector -D_GLIBCXX_USE_CXX_ABI=1" \
	   EXTERNAL_CPPFLAGS="-fno-stack-protector -D_GLIBCXX_USE_CXX_ABI=1" \
	   LDFLAGS="-lc++" \
	   DISABLE_STEAMCONTROLLER="1" \
	   $(MAKE) -C $(BUILD_DIR)/win32/ -f $(PROJECT_ROOT)/Makefile libblastpit.a

# DON'T use 'localhost' under win32. It doesn't resolve correctly (ipv6?)

wscli:	libblastpit.a
	$(CXX) $(CPPFLAGS) $(SANFLAGS) -c -I ../src/libblastpit -I ../src/submodules/mongoose -o wscli.o -x c ../src/libblastpit/wscli.c
	$(CXX) $(CPPFLAGS) $(SANFLAGS) -o wscli wscli.o libblastpit.a

server:	$(BUILD_DIR)/wscli
	$^ -s 8000

ueye:	$(BUILD_DIR)/libblastpit.a
	zig c++ \
		-I src/libblastpit \
		-I src/submodules/mongoose \
		-I res/redist \
		-o build/ueye \
		src/video/ueye_simple.cpp \
		$^ \
		-L res/redist \
		-lueye_api

# Packaging

tarball:	lmos
		cd build/win32/release && \
		zip -r /tmp/lmos.zip \
		--exclude=\*.o \
		--exclude=\*.h \
		--exclude=\*.cpp \
		Qt5Core.dll Qt5Gui.dll Qt5Widgets.dll lmostray.exe platforms && \
		cd $(WINEPREFIX)/drive_c/Qt/Qt5.14.2/Tools/mingw730_32/i686-w64-mingw32/lib && \
		zip /tmp/lmos.zip \
		libstdc++-6.dll libgcc_s_dw2-1.dll libwinpthread-1.dll
