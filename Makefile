# var != command - Set var to shell output of command
# var ?= value - Set var to value if not already set
# var := value - Set var without recursively expanding value
# var  = value - Set var recursively expanding value

# Create titles with:
# :r! figlet -t -d ~/usr/share/fonts/figlet-fonts -f pagga <text> | sed 's/^/\# /'

# Include other makefiles with 'include <file.mk>'


# ░█░░░█▀█░█▀▀░█▀█░▀█▀░▀█▀░█▀█░█▀█░█▀▀
# ░█░░░█░█░█░░░█▀█░░█░░░█░░█░█░█░█░▀▀█
# ░▀▀▀░▀▀▀░▀▀▀░▀░▀░░▀░░▀▀▀░▀▀▀░▀░▀░▀▀▀

PROJECT_ROOT    := $(shell git rev-parse --show-toplevel)
PROJECT         := $(shell basename $(PROJECT_ROOT))
DOTGIT_DIR      := $(shell $(PROJECT_ROOT)/res/bin/get_dotgit_dir.sh)
BUILD_DIR       ?= $(PROJECT_ROOT)/build
SRC_DIR         := $(PROJECT_ROOT)/src
LIBBLASTPIT_DIR := $(SRC_DIR)/libblastpit
SUBMODULES_DIR  := $(SRC_DIR)/submodules
REDIST_DIR      := $(DOTGIT_DIR)/untracked/redist
GIT_HOOKS       := ${PROJECT_ROOT}/$(shell git config --get core.hooksPath)

# User defines

CPPFLAGS        += $(addprefix -D, $(USER_DEFINES))
CPPFLAGS        += -pthread -pipe
GIT_HEAD         = $(shell git rev-parse --short HEAD)
MAXJOBS         ?= $(shell nproc)
USERNAME        ?= $(shell whoami)
USER_DEFINES     = SPACENAV


# ░▀█▀░█▀█░█▀█░█░░░█▀▀
# ░░█░░█░█░█░█░█░░░▀▀█
# ░░▀░░▀▀▀░▀▀▀░▀▀▀░▀▀▀

AR               = llvm-ar
RANLIB           = llvm-ranlib
DBG              = lldb

# Compilers (Note: using Zig with bear fails to create compile database)

CC = gcc-13
CXX = g++-13

analyze analyze_build:              	 CC                      ?= clang
analyze analyze_build:              	 CXX                     ?= clang++
asan    asan_build msan msan_build: 	 CC                      ?= clang
asan    asan_build msan msan_build: 	 CXX                     ?= clang++
cross   cross_build:                	 CC                      ?= zig cc -target i386-windows-gnu
cross   cross_build:                	 CXX                     ?= zig c++ -target i386-windows-gnu
debug   debug_build:                	 CC                      ?= clang
debug   debug_build:                	 CXX                     ?= clang++
profile profile_build:              	 CC                      = $(CXX) # Needed for tracy, which is c++
release release_build:              	 CC                      ?= zig cc #-target x86_64-linux-musl
release release_build:              	 CXX                     ?= zig c++ #-target x86_64-linux-musl

# Compiler Warnings
debug_build:                  	 CPPFLAGS                += -Wall -Wextra -Werror #-Wpedantic
release release_build:              	 CPPFLAGS                += -Wall -Wextra -Wpedantic -Wfatal-errors

# Optimization
cross cross_build:                  	 CPPFLAGS                += -Ofast
debug_build:                  	 CPPFLAGS                += -O0
profile profile_build:              	 CPPFLAGS                += -Ofast -rdynamic
release release_build:              	 CPPFLAGS                += -Ofast -flto #-flto causes segfaults in getopt and others

# Debugging
analyze analyze_build: 	 CPPFLAGS += -g3 -fno-omit-frame-pointer -DDEBUG_LEVEL=1
asan asan_build:       	 CPPFLAGS += -g3 -fno-omit-frame-pointer -DDEBUG_LEVEL=1
debug_build:     	 CPPFLAGS += -g3 -fno-omit-frame-pointer -DDEBUG_LEVEL=1
msan msan_build:       	 CPPFLAGS += -g3 -fno-omit-frame-pointer -DDEBUG_LEVEL=1
profile profile_build: 	 CPPFLAGS += -g3 -fno-omit-frame-pointer -DDEBUG_LEVEL=1

# Linker Flags
GCCVERSIONGTEQ4 := $(shell expr `gcc -dumpversion | cut -f1 -d.` \>= 12)
ifeq "$(GCCVERSIONGTEQ4)" "1"
          	 LDFLAGS                 += -fuse-ld=mold
endif
debug_build:                  	 LDFLAGS                 += -Wl,-O1 -Wl,--as-needed

# gcc-11 does not recognise -fuse-ld=mold
# debug_build:                  	 LDFLAGS                 += -fuse-ld=mold

# Flags for External Sources
asan asan_build:                    	 EXTERNAL_CPPFLAGS       += -DMG_ENABLE_LOG=0 -DMG_MAX_RECV_BUF_SIZE=10000000
cross cross_build:                  	 EXTERNAL_CPPFLAGS       += -DMG_ENABLE_LOG=0 -DMG_MAX_RECV_BUF_SIZE=10000000
cross cross_build:                  	 EXTERNAL_CPPFLAGS       += -fno-stack-protector -D_GLIBCXX_USE_CXX_ABI=1
debug_build:                  	 EXTERNAL_CPPFLAGS       += -DMG_ENABLE_LOG=0 -DMG_MAX_RECV_BUF_SIZE=10000000
debug_build:                  	 EXTERNAL_CPPFLAGS       += -fno-sanitize=all -O0
msan msan_build:                    	 EXTERNAL_CPPFLAGS       += -DMG_ENABLE_LOG=0 -DMG_MAX_RECV_BUF_SIZE=10000000
release release_build:              	 EXTERNAL_CPPFLAGS       += -DMG_ENABLE_LOG=0 # Mongoose log messages can bork the returned Inkscape XML
release release_build:              	 EXTERNAL_CPPFLAGS       += -g3

# Hardening
release release_build:              	 CPPFLAGS                += -D_FORTIFY_SOURCE=3

# Profiling
profile profile_build:              	 CPPFLAGS                += -DTRACY_ENABLE
profile profile_build:              	 CPPFLAGS                += -pg --coverage
profile profile_build:              	 INCFLAGS                += -I$(SUBMODULES_DIR)/tracy/public/tracy
profile profile_build:              	 TRACY_OBJS               = $(BUILD_DIR)/TracyClient.o
profile profile_build:              	 CPPFLAGS                += -fpermissive # Tracy needs c++ compiler which is less forgiving of errors
profile profile_build: 	EXTERNAL_CPPFLAGS = -fno-sanitize=all -fpermissive

# Miscellaneous
debug_build:                  	 CPPFLAGS                += -MMD

# Cross compiling
cross cross_build:                  	 BUILD_DIR               := $(PROJECT_ROOT)/build/win32
cross cross_build:                  	 CPPFLAGS                += -fno-stack-protector -D_GLIBCXX_USE_CXX_ABI=1
cross cross_build:                  	 DISABLE_STEAMCONTROLLER  = "1"
cross cross_build:                  	 LDFLAGS                 += -lc++
cross cross_build:                  	 WIN32FLAGS              += -lws2_32

# Sanitizer Flags
# asan asan_build msan msan_build:	SANFLAGS += -fno-omit-frame-pointer -fno-optimize-sibling-calls
# asan asan_build msan msan_build:	SANFLAGS += -fsanitize-recover=all -fsanitize-blacklist=$(PROJECT_ROOT)/res/.sanitize_blacklist.txt
# asan asan_build msan msan_build:	SANLDFLAGS += -Wl,-rpath,$(shell dirname $(shell clang -print-file-name=libclang_rt.ubsan_standalone-x86_64.so))
# asan asan_build msan msan_build:	SHARED_SANFLAGS += -shared-libsan
# asan asan_build:	SANFLAGS += -fsanitize=address -fsanitize-address-use-after-scope
# asan asan_build:	SANFLAGS += -fsanitize=undefined,implicit-conversion,nullability,integer
# cross cross_build:	CPPFLAGS                  += -fno-sanitize=all
# debug debug_build:	CPPFLAGS          += -fsanitize=undefined
# msan msan_build:	SANFLAGS += -fsanitize=memory -fsanitize-memory-track-origins
# profile profile_build: 	CPPFLAGS += -fno-sanitize=all
# release release_build:	CPPFLAGS += -fno-sanitize=all
# release release_build:	EXTERNAL_CPPFLAGS += -fno-sanitize=all -Ofast -w # lto causes problems with unity


# ░█▀▀░█▀█░█░█░█▀▄░█▀▀░█▀▀░█▀▀
# ░▀▀█░█░█░█░█░█▀▄░█░░░█▀▀░▀▀█
# ░▀▀▀░▀▀▀░▀▀▀░▀░▀░▀▀▀░▀▀▀░▀▀▀

# Library source and object files
LIBMXML_SRCS        := mxml-attr.c mxml-entity.c mxml-file.c mxml-get.c mxml-index.c mxml-node.c mxml-private.c mxml-search.c mxml-set.c mxml-string.c 
LIBMXML_OBJS        := $(patsubst %.c,$(BUILD_DIR)/%.o,$(LIBMXML_SRCS))
LIBBLASTPIT_SOURCES := blastpit.c websocket.c xml.c
LIBBLASTPIT_OBJS    := $(LIBMXML_OBJS) $(BUILD_DIR)/sds.o $(BUILD_DIR)/mongoose.o $(patsubst %.c,$(BUILD_DIR)/%.o,$(LIBBLASTPIT_SOURCES))
LIBBLASTPIT_SRCS    := $(patsubst %.c,$(LIBBLASTPIT_DIR)/%.c,$(LIBBLASTPIT_SOURCES))
LIBBLASTPIT_TARGETS := $(BUILD_DIR)/libblastpit.a $(BUILD_DIR)/_blastpy.so $(BUILD_DIR)/blastpy.py 

# Unit test code (unity)
UNITY_FIXTURE_DIR = $(SUBMODULES_DIR)/unity/extras/fixture/src
UNITY_DIR         = $(SUBMODULES_DIR)/unity/src
UNITY_OBJS        = $(BUILD_DIR)/unity.o $(BUILD_DIR)/unity_fixture.o
UNITY_DEFS        = -DUNITY_OUTPUT_COLOR -DUNITY_FIXTURE_NO_EXTRAS

# Includes
INCFLAGS = -I$(UNITY_DIR) -I$(UNITY_FIXTURE_DIR) -I$(LIBBLASTPIT_DIR) -I $(PROJECT_ROOT)/res/cfg -I$(SUBMODULES_DIR)/mongoose -I$(SUBMODULES_DIR)/mxml -I$(SRC_DIR)/sds

# Libraries
LIBS  = -lm -ldl

# Blastpy
BLASTPY_FILES  = $(BUILD_DIR)/blastpy_wrap.o

# Debugging
DEBUG_COMMAND ?= $(shell head -n1 .debugcmd)
DEBUG_TARGET  ?= $(shell tail -n1 .debugcmd)

FORMAT_FILES        = src/libblastpit/*.{h,c} src/lmos/{lmos-tray,lmos,parser}.{hpp,cpp} src/lmos/{main,traysettings}.cpp src/video/*.{h,c,cpp} src/imgui/*.c src/scaffolding/*.c src/inkscape/*.{c,h} src/windows/*.c src/include/*.h src/include/*/*.h src/database/*.c src/lvgl/*.{c,h} src/scaps/*.c
FORMAT_FILES_PYTHON = src/libblastpit/*.py src/inkscape/*.py src/scaffolding/*.py
FORMAT_FILES_XML    = src/inkscape/*.inx
FORMAT_FILES_HTML   = doc/reference_manuals/lmos.html

TIDY_FILES          = src/libblastpit/{wscli,websocket}.c

# Unit Tests
# TEST_SOURCES  = t_linkedlist.c t_message.c t_xml.c t_websocket.c t_client_command.c
TEST_SOURCES = $(notdir $(wildcard $(LIBBLASTPIT_DIR)/ut_*.c)) $(notdir $(wildcard $(SRC_DIR)/scaffolding/ut_*.c)) $(notdir $(wildcard $(SRC_DIR)/inkscape/ut_*.c))
TEST_OBJS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(TEST_SOURCES))
TEST_BINARIES = $(patsubst %.c,$(BUILD_DIR)/%_x,$(TEST_SOURCES))
# UT_C_SRCS = $(wildcard $(SRC_DIR)/libblastpit/ut_*.c*) $(wildcard $(SRC_DIR)/scaffolding/ut_*.c*) $(wildcard $(SRC_DIR)/inkscape/ut_*.c*)
# UT_C_EXES = $(patsubst %.c,$(BUILD_DIR)/%_x,$(notdir $(UT_C_SRCS)))
# UT_PY_SRCS= $(SRC_DIR)/libblastpit/ut_*.py* $(SRC_DIR)/scaffolding/ut_*.py* $(SRC_DIR)/inkscape/ut_*.py*


# ░█▀▄░█▀▀░█▀▀░▀█▀░█▀█░█▀▀░█▀▀
# ░█▀▄░█▀▀░█░░░░█░░█▀▀░█▀▀░▀▀█
# ░▀░▀░▀▀▀░▀▀▀░▀▀▀░▀░░░▀▀▀░▀▀▀

.PHONY:	all clean debug release debug_build release_build cross cross_build unit_tests system_tests asan asan_build msan msan_build ebuild profile profile_build lmosx qmake lmos imgui imgui_run run_unit_tests run_system_tests targets wsci python_targets

# We have to wrap debug_build and release_build below because
# the secondary call to make would otherwise forget
# the target-specific variables defined above
all: 		debug

analyze:	clean
		scan-build --status-bugs -maxloop 20 --force-analyze-debug-code --exclude $(SUBMODULES_DIR)/mongoose --exclude $(SRC_DIR)/sds --show-description $(MAKE) -f $(PROJECT_ROOT)/Makefile analyze_build

asan:	clean $(BUILD_DIR) .tags
		$(MAKE) -f $(PROJECT_ROOT)/Makefile asan_build

msan:	clean $(BUILD_DIR) .tags
		$(MAKE) -f $(PROJECT_ROOT)/Makefile msan_build

release:	$(BUILD_DIR)
		$(MAKE) -f $(PROJECT_ROOT)/Makefile release_build 
		# $(MAKE) -f $(PROJECT_ROOT)/Makefile unit_tests

profile:	clean $(BUILD_DIR)
		$(shell cd $(SUBMODULES_DIR)/tracy; git clean -dfx)
		$(MAKE) -f $(PROJECT_ROOT)/Makefile profile_build || echo "If building tracy fails, git-clean the tracy directory and retry"
		$(MAKE) -f $(PROJECT_ROOT)/Makefile tracycap

		# This binary should contain tracy profiling as the statistics are saved in the /tmp directory
		# We have to disable the check on the webserver for some reason
		TRACY_NO_INVARIANT_CHECK=1 $(BUILD_DIR)/ut_websocket_x
		
		# Give it time to write the output
		sleep 2
		$(MAKE) -f $(PROJECT_ROOT)/Makefile tracyexport

cross:		pngs
		BUILD_DIR=$(PROJECT_ROOT)/build/win32 $(MAKE) -f $(PROJECT_ROOT)/Makefile cross_build

alltargetscheck:
		make clean
		CC="zig cc" CXX="zig c++" make debug
		CC="zig cc" CXX="zig c++" make unit_tests
		CC="zig cc" CXX="zig c++" make system_tests
		make clean
		CC="zig cc" CXX="zig c++" make release
		CC="zig cc" CXX="zig c++" make unit_tests
		CC="zig cc" CXX="zig c++" make system_tests
		make clean
		make asan
		make test_asan
		make clean
		make msan
		make test_msan
		make clean
		make cross
		make wscli_portable
		make clean
		make profile

# Prevent intermediate files being deleted and rebuilt every time
.SECONDARY: 	$(LIBBLASTPIT_OBJS) $(TEST_OBJS)
targets:	$(BUILD_DIR) $(LIBBLASTPIT_OBJS) $(UNITY_OBJS) $(TEST_BINARIES)

accessories:	ueye cameras images

debug: 		debug_build | $(BUILD_DIR) .tags
		$(MAKE) -f $(PROJECT_ROOT)/Makefile unit_tests
		@$(PROJECT_ROOT)/res/bin/run_after_build.sh

debug_build:	.tags targets wscli python_targets imgui lvgl

analyze_build:	targets

asan_build:	targets

msan_build:	targets

release_build:	targets python_targets cameras wscli imgui

.SECONDEXPANSION:
profile_build:	$(LIBBLASTPIT_OBJS) $(UNITY_OBJS) $(TEST_BINARIES) $$(TRACY_OBJS)

test_build:	targets

cross_build:	$(BUILD_DIR) $(LIBBLASTPIT_OBJS)

ebuild:		$(BUILD_DIR) $(LIBBLASTPIT_OBJS)

clean:
		@rm -rf $(BUILD_DIR) 2>/dev/null || /bin/true
		@rm -rf $(PROJECT_ROOT)/.{cache,ccls-cache,pytest_cache} 2>/dev/null || /bin/true
		@rm -f $(PROJECT_ROOT)/{.tags,compile_command*.json} >/dev/null 2>/dev/null || /bin/true

distclean:
		git clean -dfx || /bin/true
		git submodule foreach 'git clean -dfx' || /bin/true
		rm -rf /home/$(shell whoami)/.cache/zig 2>/dev/null || /bin/true

$(BUILD_DIR):
	if [ ! -d "$(BUILD_DIR)" ]; then mkdir -p $(BUILD_DIR); fi
	# ln -fs $(PROJECT_ROOT)/.git/untracked/myconfig.py $(BUILD_DIR)/


include $(SRC_DIR)/makefiles/cameras.mk
include $(SRC_DIR)/makefiles/compiling.mk
include $(SRC_DIR)/makefiles/cross_compiling.mk
include $(SRC_DIR)/makefiles/formatting.mk
include $(SRC_DIR)/makefiles/fuzzing.mk
include $(SRC_DIR)/makefiles/images.mk
include $(SRC_DIR)/makefiles/imgui.mk
include $(SRC_DIR)/makefiles/lvgl.mk
include $(SRC_DIR)/makefiles/python.mk
include $(SRC_DIR)/makefiles/scaps.mk
include $(SRC_DIR)/makefiles/testing.mk
include $(SRC_DIR)/makefiles/tracy.mk
include $(SRC_DIR)/makefiles/wscli.mk
include $(SRC_DIR)/makefiles/helpers.mk
include $(SRC_DIR)/makefiles/doxygen.mk
