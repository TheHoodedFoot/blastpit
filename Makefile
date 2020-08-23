# var != command - Set var to shell output of comma
# var ?= value - Set var to value if not already set
# var := value - Set var without recursively expanding value
# var = value - Set var recursively expanding value


# Locations
PROJECT_ROOT    := $(shell git rev-parse --show-toplevel)
PROJECT         := $(shell basename $(PROJECT_ROOT))
BUILD_DIR       := $(PROJECT_ROOT)/build
# RELATIVE_ROOT   := $(shell realpath --relative-to $(BUILD_DIR) $(PROJECT_ROOT))
BLASTMINE_DIR   := $(PROJECT_ROOT)/src/blastmine
LIBBLASTPIT_DIR := $(PROJECT_ROOT)/src/libblastpit
SUBMODULES_DIR  := $(PROJECT_ROOT)/src/submodules
WEBAPP_DIR      := $(PROJECT_ROOT)/src/webapp
GIT_HOOKS       := ${PROJECT_ROOT}/$(shell git config --get core.hooksPath)

# User defines
USER_DEFINES  = SPACENAV MG_ENABLE_FILESYSTEM=1
USER_DEFINES += DEBUG_LEVEL=5
CPPFLAGS     += $(addprefix -D, $(USER_DEFINES))
CPPFLAGS     += -pthread
# MAXJOBS       = -j1
MAXJOBS       = -j$(shell nproc)

# Linker
LD      = ld.lld
LDFLAGS = -fuse-ld=lld

# Debugger
DBG = lldb

# Tools
AR = llvm-ar
RANLIB = llvm-ranlib

# Compiler flags
debug_build:	CPPFLAGS += -Wall -Wpedantic -Wextra
debug_build:	CPPFLAGS += -Werror
debug_build:	CPPFLAGS += -Og -g3
# debug_build:	CC        = zig cc
# debug_build:	CXX       = zig c++
debug_build:	CC        = ccache clang
debug_build:	CXX       = ccache clang++
# Swig does not work correctly with the undefined behaviour sanitizer settings below
# debug_build: 	CPPFLAGS += -fsanitize=undefined,implicit-conversion,nullability,integer -fno-omit-frame-pointer
# debug_build: 	CPPFLAGS += -fsanitize=address

release_build:	CPPFLAGS += -Ofast

profile: 	CPPFLAGS += -pg --coverage

# Library source and object files
LIBBLASTPIT_SOURCES := blastpit.c websocket.c mongoose.c
LIBBLASTPIT_OBJS    := $(patsubst %.c,%.o,$(LIBBLASTPIT_SOURCES)) xml.o pugixml.o sds.o
LIBBLASTPIT_SRCS    := $(patsubst %.c,$(LIBBLASTPIT_DIR)/%.c,$(LIBBLASTPIT_SOURCES)) $(SUBMODULES_DIR)/sds/sds.c
LIBBLASTPIT_TARGETS := libblastpit.a _blastpy.so blastpy.py 

# Unit test code (unity)
UNITY_FIXTURE_DIR = $(SUBMODULES_DIR)/unity/extras/fixture/src
UNITY_DIR         = $(SUBMODULES_DIR)/unity/src
UNITY_OBJS        = unity.o unity_fixture.o
UNITY_DEFS        = -DUNITY_OUTPUT_COLOR -DUNITY_FIXTURE_NO_EXTRAS

# Includes
INCFLAGS = -I. -I$(UNITY_DIR) -I$(UNITY_FIXTURE_DIR) -I$(SUBMODULES_DIR)/mongoose -I$(SUBMODULES_DIR)/sds

# Python inclues for SWIG
PYTHON_INCS = $(shell python-config --includes)

# Libraries
LIBS  = -lm

# Blastpy
BLASTPY_FILES  = blastpy_wrap.o xml.o pugixml.o
BLASTPY_SRCS   = $(patsubst %.o,$(SRCDIR)/%.c,$(BLASTPY_FILES))
BLASTPY_FILES += libblastpit.a
BLASTPY_LIBS   = 

# Blastmine
BLASTMINE_TARGETS = blastmine

# Webapp
WEBAPP_TARGETS = $(PROJECT).html $(PROJECT).js $(PROJECT).wasm

# Debugging
DEBUG_COMMAND ?= $(shell head -n1 .debugcmd)
DEBUG_TARGET  ?= $(shell tail -n1 .debugcmd)

FORMAT_FILES        = src/libblastpit/*.{h,hpp,c,cpp} src/blastmine/*.{h,hpp,cpp} src/lmos/{lmos-tray,lmos,main,parser,traysettings}.{hpp,cpp}
FORMAT_FILES_PYTHON = res/bin/*.py src/libblastpit/*.py src/inkscape/*.py
FORMAT_FILES_XML    = src/inkscape/*.inx
FORMAT_FILES_HTML   = doc/reference_manuals/lmos.html

# Unit Tests
# TEST_SOURCES  = t_linkedlist.c t_message.c t_xml.c t_websocket.c t_client_command.c
TEST_SOURCES = $(notdir $(wildcard $(LIBBLASTPIT_DIR)/t_*.c))
TEST_BINARIES = $(patsubst %.c,%_x,$(TEST_SOURCES))



.PHONY:	all clean debug release debug_build release_build emcc cross cli

# We have to wrap debug_build and release_build below because
# the secondary call to make would otherwise forget
# the target-specific variables defined above
all: 		test
		@printf "\033k$(PROJECT)\033\\"

debug: 		$(BUILD_DIR) .tags
		bear -a $(MAKE) $(MAXJOBS) -C $(BUILD_DIR) -f $(PROJECT_ROOT)/Makefile debug_build

release:	$(BUILD_DIR)
		$(MAKE) $(MAXJOBS) -C $(BUILD_DIR) -f $(PROJECT_ROOT)/Makefile release_build 

targets:	$(BUILD_DIR) $(LIBBLASTPIT_TARGETS) _blastpy.so blastmine $(UNITY_OBJS) $(TEST_BINARIES) cli

debug_build:	targets

release_build:	targets emcc webapp

test_build:	targets

clean:
	rm -rf $(BUILD_DIR) compile_commands.json .tags{,extra} .clangd
	pkill wscli || /bin/true

$(BUILD_DIR) $(BUILD_DIR)/lmosgui/:
	mkdir -p $(BUILD_DIR)/{win32,lmosgui}
	ln -fs $(PROJECT_ROOT)/.git/untracked/myconfig.py $(BUILD_DIR)/

emcc:	
	@if [[ ! $$(command -v emcc) ]]; then echo -e "\n\e[31mYou need to source the emscripten environment setup script. Use:\e[39m\n\n	pushd src/submodules/emsdk && source emsdk_env.sh && popd\n\n"; exit 255; fi

# Libblastpit Recipes
xml.o:	$(PROJECT_ROOT)/src/libblastpit/xml.cpp
	$(CXX) $(CPPFLAGS) -I$(SUBMODULES_DIR)/pugixml/src -c -fPIC $^ -o $@

pugixml.o:	$(SUBMODULES_DIR)/pugixml/src/pugixml.cpp
	$(CXX) $(CPPFLAGS) -c -fPIC $^ -o $@

sds.o:	$(SUBMODULES_DIR)/sds/sds.c
	$(CC) $(CPPFLAGS) -c -fPIC $^ -o $@

libblastpit.a: $(LIBBLASTPIT_OBJS)
	$(AR) -crs $@ $^
	$(RANLIB) $@
 
blastpy.py:	blastpy_wrap.c

blastpy_wrap.c:	libblastpit.a # This forces rerunning swig on blastpit change
	swig -o blastpy_wrap.c -outdir $(BUILD_DIR) -python $(PROJECT_ROOT)/src/libblastpit/blastpy.i

blastpy_wrap.o:	blastpy_wrap.c
	$(CC) -fPIC -I$(PROJECT_ROOT)/src/libblastpit -I$(SUBMODULES_DIR)/mongoose -o $@ -c $^ $(PYTHON_INCS)

_blastpy.so:	blastpy_wrap.o $(BLASTPY_FILES)
	$(CXX) $(CPPFLAGS) -shared $(BLASTPY_FILES) -o $@ $(BLASTPY_LIBS)

unity_fixture.o: $(UNITY_FIXTURE_DIR)/unity_fixture.c
	$(CC) $(CPPFLAGS) $(UNITY_DEFS) $(CPPFLAGS) $(INCFLAGS) -c $< -o $@

unity.o:	$(UNITY_DIR)/unity.c
	$(CC) $(CPPFLAGS) $(UNITY_DEFS) $(CPPFLAGS) $(INCFLAGS) -c $< -o $@

t_%_x:	t_%.o $(UNITY_OBJS) libblastpit.a
	$(CXX) $(CPPFLAGS) $(UNITY_DEFS) $(SANFLAGS) $(UNITY_OBJS) $< -L. -lblastpit -o $@ $(LIBS)

%.o:	%.cpp
	$(CXX) $(CPPFLAGS) $(INCFLAGS) -c -fPIC $^ -o $@

%.o:	$(LIBBLASTPIT_DIR)/%.c
	$(CC) $(CPPFLAGS) $(SANFLAGS) $(INCFLAGS) $(UNITY_DEFS) -c -fPIC $^ -o $@

%.o:	$(SUBMODULES_DIR)/mongoose/%.c
	$(CC) $(INCFLAGS) $(UNITY_DEFS) -c -fPIC $^ -o $@



# Blastmine Recipes
blastmine:	$(BLASTMINE_DIR)/blastmine.zig
	zig build-exe $^ --cache-dir $(BUILD_DIR) --color on -femit-docs -dynamic



# Webapp Recipes
webapp: 	$(WEBAPP_TARGETS)

$(PROJECT).html:	$(WEBAPP_DIR)/$(PROJECT).c
	@if [[ ! $$(command -v emcc) ]]; then echo -e "\n\n\e[31mYou need to source ~/usr/src/SDK/emsdk/emsdk_env.sh to setup the Emscripten environment\e[39m\n\n"; exit 255; else emcc $^ -o $@; fi

$(PROJECT).js $(PROJECT).wasm: $(PROJECT).html

webapp_run:	$(WEBAPP_TARGETS)
	# TODO: Build this into libblastpit using mongoose
	killall -q python || /bin/true
	xdg-open http://localhost:8000/$(PROJECT).html &
	python -m http.server --directory $(BUILD_DIR)

# Miscellaneous Recipes
octave:	$(BUILD_DIR) octave_interpreter octave_no_interpreter1 octave_no_interpreter2

octave_%:	$(BLASTMINE_DIR)/octave_%.cpp
	$(CXX) $^ -o $@ -I$(shell octave-config -p OCTINCLUDEDIR) -I$(shell octave-config -p OCTINCLUDEDIR)/.. -L$(shell octave-config -p OCTLIBDIR) -lcppunit -loctave -loctinterp


# Testing and Debugging
test:	debug
	$(MAKE) $(MAXJOBS) -C $(BUILD_DIR) -f $(PROJECT_ROOT)/Makefile test_run

test_run:
	@#if [[ $(shell cat /proc/sys/vm/overcommit_memory) != "0" ]]; then echo 0 | sudo tee /proc/sys/vm/overcommit_memory; fi
	@# zig test $(PROJECT_ROOT)/src/blastmine/blastmine.zig --cache-dir $(BUILD_DIR) --color on || failed
	@# zig test $(PROJECT_ROOT)/src/libblastpit/websocket.zig --cache-dir $(BUILD_DIR) -I $(LIBBLASTPIT_DIR) --library libblastpit.a -lc --color on || failed
	@UBSAN_OPTIONS=print_stacktrace=1 sh -c $(PROJECT_ROOT)/res/bin/testrunner.sh
	passed

dbug:
	$(DEBUG_COMMAND) $(DEBUG_TARGET)
	
pydbug:
	PYTHONPATH=build gdb -ex r --args python3 src/libblastpit/t_client_command.py

.tags:
	@(${GIT_HOOKS}/ctags >/dev/null 2>&1 &)


# Analysis, Fuzzing and Profiling
fuzz:	
	make clean
	CC=afl-clang CXX=afl-clang++ make
	echo performance | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
	mkdir -p /tmp/afl-{in,out}
	cp $(FUZZ_BINARY) /tmp/
	cp $(FUZZ_INPUTS) /tmp/afl-in/
	cd /tmp/
	afl-fuzz -i /tmp/afl-in -o /tmp/afl-out /tmp/$(FUZZ_BINARY)

valgrind:	$(TEST_BINARIES)
	@echo -e "\e[33mRunning valgrind tests...\e[39m"
	@sh -c "./testrunner.sh -valgrind"

analyse:
	# $(MAKE) clean
	scan-build $(MAKE) -C $(BUILD_DIR) -f $(SRCDIR)/Makefile all
	$(BROWSER) /tmp/scan-build*


# Formatting
format:
	clang-format -style=file -i $(FORMAT_FILES) || /bin/true

formatextra:	vale
	/bin/sh -c 'for file in $(FORMAT_FILES_XML); do xmllint --format --nsclean --output $$file $$file; done'
	/bin/sh -c 'for file in $(FORMAT_FILES_HTML); do xmllint --format --nsclean --html --output $$file $$file; done'
	autopep8 --global-config=$(PROJECT_ROOT)/res/cfg/pycodestyle --aggressive --aggressive -i $(FORMAT_FILES_PYTHON)
	flake8 --ignore=E402 --max-complexity=10 --show-source $(FORMAT_FILES_PYTHON)

tabconvert:
	/bin/sh -c 'for file in $(FORMAT_FILES_PYTHON); do unexpand -t4 $$file > unexpanded.tmp; mv unexpanded.tmp $$file; done'


# Documentation and Resource Generation
%.ico:	%.svg
	convert -background none -define icon:auto-resize=256,128,64,48,32,16 $^ $@
%.png:	%.svg
	convert -background none $^ $@
icons:	res/img/laseractive.ico res/img/noconnection.ico res/img/nolaser.ico
pngs:	res/img/laseractive.png res/img/noconnection.png res/img/nolaser.png
	# convert -background none -define icon:auto-resize=256,128,64,48,32,16 src/img/blastpit.svg src/img/tentacle.ico
	# convert -background none src/img/blastpit.svg src/img/tentacle.png && convert -background none src/img/blastpit.svg src/img/tripmine.png

tex:
	/bin/sh -c "cd doc/manual; xelatex -output-directory=$(PROJECT_ROOT)/build blastpit.tex"
	biber --input-directory=$(BUILD_DIR) --output-directory=$(BUILD_DIR) blastpit.bcf
	makeindex $(BUILD_DIR)/blastpit.idx -s doc/manual/imports/styleind.ist
	/bin/sh -c "cd doc/manual; xelatex -output-directory=$(PROJECT_ROOT)/build blastpit.tex"
	/bin/sh -c "cd doc/manual; xelatex -output-directory=$(PROJECT_ROOT)/build blastpit.tex"
	qpdfview $(BUILD_DIR)/blastpit.pdf

	# /bin/sh -c 'cd doc/manual; xelatex blastpit && biber blastpit && makeindex blastpit.idx -s imports/styleind.ist && xelatex blastpit && xelatex blastpit'

graph:	$(patsubst %,$(GRAPHDIR)/%.pdf, bp bp_database)



# Cross Compiling and Emulation

# Use:
# 	make wine
#
# from within 32-bit chroot
#
# Change setting in lmos.pro to debug_and_release for release version

WHINE_PREFIX = "${HOME}/.wine/rofin"
WHINE_ARCH = "win32"
WHINE_LIBS = "C:\\Qt\\Qt5.14.2\\Tools\\mingw730_32\\bin"
WHINE_CXX = $(WHINE_LIBS)"\\g++.exe"

# Qt (Wine) - Must be run from within the 32-bit environment
WINEDIR=${HOME}/.wine/rofin

$(BUILD_DIR)/win32/debug/platforms $(BUILD_DIR)/win32/release/platforms:
	mkdir -p $(BUILD_DIR)/win32/{release,debug}/platforms
	cp -v ~/.wine/rofin/drive_c/Qt/Qt5.14.2/5.14.2/mingw73_32/bin/Qt5{Core,Gui,Widgets}.dll $(BUILD_DIR)/win32/release/
	cp -v ~/.wine/rofin/drive_c/Qt/Qt5.14.2/5.14.2/mingw73_32/plugins/platforms/qwindows.dll $(BUILD_DIR)/win32/release/platforms
	cp -v ~/.wine/rofin/drive_c/Qt/Qt5.14.2/5.14.2/mingw73_32/bin/Qt5{Core,Gui,Widgets}.dll $(BUILD_DIR)/win32/debug/
	cp -v ~/.wine/rofin/drive_c/Qt/Qt5.14.2/5.14.2/mingw73_32/plugins/platforms/qwindows.dll $(BUILD_DIR)/win32/debug/platforms

qmake $(BUILD_DIR)/win32/Makefile: $(BUILD_DIR)
	env WINEPREFIX="$(WINEDIR)" \
		WINEARCH="win32" \
		WINEPATH="C:\\Qt\\Qt5.14.2\\Tools\\mingw730_32\\bin" \
		wine $(WINEDIR)/drive_c/Qt/Qt5.14.2/5.14.2/mingw73_32/bin/qmake.exe \
		-o $(BUILD_DIR)/win32/Makefile \
		src\\lmos\\lmos.pro

lmos $(BUILD_DIR)/win32/debug/lmosgui.exe $(BUILD_DIR)/win32/release/lmosgui.exe:	$(BUILD_DIR)/win32/Makefile $(BUILD_DIR)/win32/debug/platforms $(BUILD_DIR)/win32/release/platforms
	env WINEPREFIX="$(WINEDIR)" \
		WINEARCH="win32" \
		WINEPATH="C:\\Qt\\Qt5.14.2\\Tools\\mingw730_32\\bin" \
		wine $(WINEDIR)/drive_c/Qt/Qt5.14.2/Tools/mingw730_32/bin/mingw32-make.exe -C $(BUILD_DIR)/win32 $(MAXJOBS)

lmostray:	$(BUILD_DIR)/win32/debug/lmosgui.exe
	env WINEPREFIX="$(WINEDIR)" \
		WINEARCH="win32" \
		WINEPATH="C:\\Qt\\Qt5.14.2\\Tools\\mingw730_32\\bin" \
		wine $^

lmostrayrelease:	$(BUILD_DIR)/win32/release/lmosgui.exe tarball
	env WINEPREFIX="$(WINEDIR)" \
		WINEARCH="win32" \
		WINEPATH="C:\\Qt\\Qt5.14.2\\Tools\\mingw730_32\\bin" \
		wine $^

$(BUILD_DIR)/lmosgui/Makefile:	$(BUILD_DIR)/lmosgui/ $(PROJECT_ROOT)/src/lmos/lmos.pro
	qmake -o $@ $(PROJECT_ROOT)/src/lmos/lmos.pro

lmoslin:	$(BUILD_DIR)/lmosgui/Makefile
	make -C $(BUILD_DIR)/lmosgui $(MAXJOBS)
	$(BUILD_DIR)/lmosgui/lmosgui

boost:
	cp -v ~/.wine/rofin/drive_c/Qt/Qt5.14.2/5.14.2/mingw73_32/bin/libwinpthread-1.dll .
	env WINEPREFIX="$(WINEDIR)" \
		WINEARCH="win32" \
		wine "C:\\Qt\\Qt5.14.2\\Tools\\mingw730_32\\bin\\g++.exe" \
		"-IZ:/home/thf/projects/blastpit/src/submodules/websocketpp" \
		"-IZ:/home/tmp/boost_1_73_0" \
		-v \
		examples/debug_server/debug_server.cpp \
		-lwsock32 -lws2_32

# Display the default Wine include search paths
winincsearch:
	echo | \
	env WINEPREFIX="$(WINEDIR)" \
		WINEARCH="win32" \
		wine "C:\\Qt\\Qt5.14.2\\Tools\\mingw730_32\\bin\\g++.exe" \
		-v \
		-E \
		-c -

# Build Mongoose example using mingw and Wine
mingoose:
	env \
		WINEPREFIX="/home/thf/.wine/rofin" \
		WINEARCH="win32" \
		WINEPATH="C:\\Qt\\Qt5.14.2\\Tools\\mingw730_32\\bin" \
		WINEDLLPATH="C:\\Qt\\Qt5.14.2\\5.14.2\\mingw73_32\\bin" \
		wine ~/.wine/rofin/drive_c/Qt/Qt5.14.2/Tools/mingw730_32/bin/gcc.exe \
		-I ~/projects/blastpit/src/submodules/mongoose \
		-o $@ \
		~/projects/blastpit/src/submodules/mongoose/mongoose.c \
		~/projects/blastpit/src/submodules/mongoose/examples/simplest_web_server/simplest_web_server.c \
		-lwsock32

# Build Mongoose example by cross-compiling with Zig
mongoose:	$(BUILD_DIR)
	zig cc \
		-target i386-windows-gnu \
		-DCS_PLATFORM=CS_P_WINDOWS \
		-o $(BUILD_DIR)/mongoose.exe \
		-I ~/projects/blastpit/src/submodules/mongoose \
		-L /home/thf/.wine/rofin/drive_c/Qt/Qt5.14.2/Tools/mingw730_32/i686-w64-mingw32/lib \
		src/submodules/mongoose/mongoose.c \
		src/submodules/mongoose/examples/simplest_web_server/simplest_web_server.c \
		-lwsock32

rungoose:	
	env \
		WINEPREFIX="/home/thf/.wine/rofin" \
		WINEARCH="win32" \
		WINEPATH="C:\\Qt\\Qt5.14.2\\Tools\\mingw730_32\\bin" \
		WINEDLLPATH="C:\\Qt\\Qt5.14.2\\5.14.2\\mingw73_32\\bin" \
		wine $(BUILD_DIR)/mongoose.exe

# Build the mongoose object file for either linux or windows
websocketlin:	$(BUILD_DIR)
	zig cc \
		-o $(BUILD_DIR)/mongoose.o \
		-I src/libblastpit \
		-I src/submodules/mongoose \
		-c src/submodules/mongoose/mongoose.c
	zig cc \
		-o $(BUILD_DIR)/websocket.o \
		-I src/libblastpit \
		-I src/submodules/mongoose \
		-c src/libblastpit/websocket.c

websocketwin:	$(BUILD_DIR)
	zig cc \
		-target i386-windows-gnu \
		-DCS_PLATFORM=CS_P_WINDOWS \
		-o $(BUILD_DIR)/websocket.o \
		-I ~/projects/blastpit/src/submodules/mongoose \
		-I ~/projects/blastpit/src/submodules/sds \
		-c src/libblastpit/websocket.c
	zig cc \
		-target i386-windows-gnu \
		-DCS_PLATFORM=CS_P_WINDOWS \
		-o $(BUILD_DIR)/mongoose.o \
		-I src/submodules/mongoose \
		-L /home/thf/.wine/rofin/drive_c/Qt/Qt5.14.2/Tools/mingw730_32/i686-w64-mingw32/lib \
		-c src/submodules/mongoose/mongoose.c \
		-lwsock32

win.exe:	test.cpp src/libblastpit/linkedlist.c src/libblastpit/blastpit.c src/libblastpit/parser.c src/libblastpit/message.c src/libblastpit/new.c src/libblastpit/xml.cpp sub/pugixml/src/pugixml.cpp
	env WINEPREFIX="$(WHINE_PREFIX)" \
		WINEARCH="$(WHINE_ARCH)" \
		PATH="/usr/bin" \
		WINEPATH=$(WHINE_LIBS) \
		wine "$(WHINE_CXX)" \
		-D_GLIBCXX_USE_CXX11_ABI=1 -std=c++11 $^ -o $@

win:	win.exe
	env WINEPREFIX="$(WHINE_PREFIX)" \
		WINEARCH="$(WHINE_ARCH)" \
		WINEPATH="$(WHINE_LIBS)" \
		wine $^

qtcreator:
	env WINEPREFIX="$(WHINE_PREFIX)" \
		WINEARCH="$(WHINE_ARCH)" \
		WINEPATH="$(WHINE_LIBS)" wine $(WHINE_PREFIX)/drive_c/Qt/Qt5.14.2/Tools/QtCreator/bin/qtcreator

cross:	$(BUILD_DIR)
	CC="zig cc -target i386-windows-gnu" CXX="zig c++ -target i386-windows-gnu -std=c++11" CPPFLAGS="-fno-stack-protector -D_GLIBCXX_USE_CXX_ABI=1" LDFLAGS="-lc++" $(MAKE) -C $(BUILD_DIR)/win32/ -f $(PROJECT_ROOT)/Makefile libblastpit.a

# DON'T use 'localhost' under win32. It doesn't resolve correctly (ipv6?)

cli:	$(BUILD_DIR) wscli_lin wscli_win

wscli_win: cross
	CC="zig cc -target i386-windows-gnu" \
		CXX="zig c++ -target i386-windows-gnu" \
		CPPFLAGS="-fno-stack-protector" \
		$(MAKE) -C $(BUILD_DIR)/win32 -f $(PROJECT_ROOT)/Makefile \
		wscli.exe

wscli.exe:
	$(CC) \
		-I ../../src/libblastpit \
		-I ../../src/submodules/mongoose \
		-L /home/thf/.wine/rofin/drive_c/Qt/Qt5.14.2/Tools/mingw730_32/i686-w64-mingw32/lib \
		-o wscli.exe \
		../../src/libblastpit/wscli.c \
		libblastpit.a \
		-lc++ \
		-lwsock32

wscli_lin: $(BUILD_DIR)
	$(MAKE) -C $(BUILD_DIR) -f $(PROJECT_ROOT)/Makefile libblastpit.a
	$(MAKE) -C $(BUILD_DIR) -f $(PROJECT_ROOT)/Makefile wscli

wscli:	libblastpit.a
	$(CXX) $(CPPFLAGS) \
		-c \
		-I ../src/libblastpit \
		-I ../src/submodules/mongoose \
		-o wscli.o \
		-x c \
		../src/libblastpit/wscli.c
	$(CXX) $(CPPFLAGS) \
		-o wscli \
		wscli.o libblastpit.a
		# -L /usr/lib64 \
		# -L /usr/lib/gcc/x86_64-pc-linux-gnu/9.2.0 \
		# -lc -lstdc++

server:	$(BUILD_DIR)/wscli
	$^ -s 8000

winws:	
	env WINEPREFIX="$(WHINE_PREFIX)" \
		WINEARCH="$(WHINE_ARCH)" \
		WINEPATH="$(WHINE_LIBS)" \
		wine build/win32/wscli.exe -c ws://127.0.0.1:8000

vale:	README.md
	vale $^


# Packaging

tarball:
	tar cvJf /tmp/lmos.tar.xz \
		-C build/win32/release \
		--exclude=\*.o \
		--exclude=\*.h \
		--exclude=\*.cpp \
		Qt5Core.dll Qt5Gui.dll Qt5Widgets.dll lmosgui.exe platforms \
		-C ~/.wine/rofin/drive_c/Qt/Qt5.14.2/Tools/mingw730_32/i686-w64-mingw32/lib \
		libstdc++-6.dll libgcc_s_dw2-1.dll


# Building lmos and cli utilities
#
# cli:
# 	make cli
#
# lmos:
# 	(From within 32-bit wine chroot)
# 	make clean lmos
#
# To run lmos:
# 	(From within 32-bit wine chroot)
# 	make lmostray
