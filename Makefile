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
CPPFLAGS        += -B/usr/libexec/mold
# CPPFLAGS        += -fuse-ld=mold
GIT_HEAD         = $(shell git rev-parse --short HEAD)
MAXJOBS         ?= $(shell nproc)
USERNAME        ?= $(shell whoami)
USER_DEFINES     = SPACENAV


# Compilers (Note: using Zig with bear fails to create compile database)
DEBUG_CC = clang
DEBUG_CXX = clang++
ASAN_CC = clang
ASAN_CXX = clang++
MSAN_CC = clang
MSAN_CXX = clang++
CROSS_CC = zig cc -target x86-windows-gnu
CROSS_CXX = zig c++ -target x86-windows-gnu
PROFILE_CC = gcc # Needed for tracy, which is c++
PROFILE_CXX = g++

# Compiler Warnings
CPPFLAGS += -Wall -Wextra
RELEASE_CPPFLAGS += -Wall -Wextra -Wpedantic -Werror -Wfatal-errors

# Optimization
DEBUG_CPPFLAGS                += -O0
PROFILE_CPPFLAGS                += -O3 -flto
PROFILE_CPPFLAGS  = -fno-omit-frame-pointer
RELEASE_CPPFLAGS                += -Ofast

# Debugging
DEBUG_CPPFLAGS += -g3 -fno-omit-frame-pointer -DDEBUG_LEVEL=1

# Tracy
TRACY_CPPFLAGS += -DTRACY_ENABLE
TRACY_CXXFLAGS = -fpermissive
TRACY_INCFLAGS += -I$(SUBMODULES_DIR)/tracy/public/tracy
# TRACY_OBJS      =  $(BUILD_DIR)/TracyClient.o

ASAN_CPPFLAGS += $(DEBUG_CPPFLAGS)
MSAN_CPPFLAGS += $(DEBUG_CPPFLAGS)
PROFILE_CPPFLAGS += $(TRACY_CPPFLAGS)

# Linker Flags
DEBUG_LDFLAGS  += -Wl,-O1 -Wl,--as-needed

# Flags for External Sources
ASAN_EXTERNAL_CPPFLAGS       += -DMG_ENABLE_LOG=0 -DMG_MAX_RECV_BUF_SIZE=10000000
CROSS_EXTERNAL_CPPFLAGS       += -DMG_ENABLE_LOG=0 -DMG_MAX_RECV_BUF_SIZE=10000000
CROSS_EXTERNAL_CPPFLAGS       += -fno-stack-protector -D_GLIBCXX_USE_CXX_ABI=1
DEBUG_EXTERNAL_CPPFLAGS       += -DMG_ENABLE_LOG=0 -DMG_MAX_RECV_BUF_SIZE=10000000
DEBUG_EXTERNAL_CPPFLAGS       += -fno-sanitize=all -O0
MSAN_EXTERNAL_CPPFLAGS       += -DMG_ENABLE_LOG=0 -DMG_MAX_RECV_BUF_SIZE=10000000
RELEASE_EXTERNAL_CPPFLAGS       += -DMG_ENABLE_LOG=0 # Mongoose log messages can bork the returned Inkscape XML
RELEASE_EXTERNAL_CPPFLAGS       += -g3

# Hardening
RELEASE_CPPFLAGS                += -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2 # Sometimes fails because already set, so unset first

# Profiling
PROFILE_INCFLAGS                += -I$(SUBMODULES_DIR)/tracy/public/tracy
PROFILE_TRACY_OBJS               = $(BUILD_DIR)/TracyClient.o
# PROFILE_CPPFLAGS                += -fpermissive # Tracy needs c++ compiler which is less forgiving of errors
PROFILE_EXTERNAL_CPPFLAGS = -fno-sanitize=all

# Miscellaneous
# DEBUG_CPPFLAGS                += -MMD

# Cross compiling
CROSS_BUILD_DIR               := $(PROJECT_ROOT)/build/win32
CROSS_CPPFLAGS                += -fno-stack-protector -D_GLIBCXX_USE_CXX_ABI=1
CROSS_CPPFLAGS += -DDISABLE_STEAMCONTROLLER
CROSS_LDFLAGS                 += -lc++
cross:                  	 WIN32FLAGS              += -lws2_32

# Sanitizer Flags
ASAN_SANFLAGS             += -fsanitize=address -fsanitize-address-use-after-scope
ASAN_SANFLAGS             += -fsanitize=undefined,implicit-conversion,nullability,integer
ASAN_SANFLAGS             += -shared-libsan
CROSS_CPPFLAGS            += -fno-sanitize=all
DEBUG_CPPFLAGS            += -fsanitize=undefined
MSAN_SANFLAGS             += -fsanitize=memory -fsanitize-memory-track-origins
PROFILE_CPPFLAGS          += -fno-sanitize=all
RELEASE_CPPFLAGS          += -fno-sanitize=all
RELEASE_EXTERNAL_CPPFLAGS += -fno-sanitize=all -Ofast -w # lto causes problems with unity
SANFLAGS                  += -fno-omit-frame-pointer -fno-optimize-sibling-calls
SANFLAGS                  += -fsanitize-recover=all #-fsanitize-blacklist=$(PROJECT_ROOT)/res/.sanitize_blacklist.txt
SANLDFLAGS                += -Wl,-rpath,$(shell dirname $(shell clang -print-file-name=libclang_rt.ubsan_standalone-x86_64.so))

# Enable Figlet if available
ifeq (,$(shell which figlet >/dev/null 2>&1))
FIGLET := echo
else
FIGLET := figlet
endif

# ░█▀▀░█▀█░█░█░█▀▄░█▀▀░█▀▀░█▀▀
# ░▀▀█░█░█░█░█░█▀▄░█░░░█▀▀░▀▀█
# ░▀▀▀░▀▀▀░▀▀▀░▀░▀░▀▀▀░▀▀▀░▀▀▀

# Library source and object files
LIBMXML_SRCS        := mxml-attr.c mxml-file.c mxml-get.c mxml-index.c mxml-node.c mxml-private.c mxml-search.c mxml-set.c mxml-options.c
LIBMXML_OBJS        := $(patsubst %.c,$(BUILD_DIR)/%.o,$(LIBMXML_SRCS))
WIN32_LIBMXML_OBJS        := $(patsubst %.c,$(BUILD_DIR)/win32/%.o,$(LIBMXML_SRCS))
LIBBLASTPIT_SOURCES := blastpit.c websocket.c xml.c

LIBBLASTPIT_OBJS    :=$(patsubst %.c,$(BUILD_DIR)/%.o,$(LIBBLASTPIT_SOURCES))
WIN32_LIBBLASTPIT_OBJS    :=$(patsubst %.c,$(BUILD_DIR)/win32/%.o,$(LIBBLASTPIT_SOURCES))

EXTERNAL_OBJS := $(LIBMXML_OBJS) $(BUILD_DIR)/sds.o $(BUILD_DIR)/mongoose.o
WIN32_EXTERNAL_OBJS := $(WIN32_LIBMXML_OBJS) $(BUILD_DIR)/win32/sds.o $(BUILD_DIR)/win32/mongoose.o

LIBBLASTPIT_SRCS    := $(patsubst %.c,$(LIBBLASTPIT_DIR)/%.c,$(LIBBLASTPIT_SOURCES))
LIBBLASTPIT_TARGETS := $(BUILD_DIR)/libblastpit.a $(BUILD_DIR)/_blastpy.so $(BUILD_DIR)/blastpy.py 

# Unit test code (unity)
UNITY_FIXTURE_DIR = $(SUBMODULES_DIR)/unity/extras/fixture/src
UNITY_DIR         = $(SUBMODULES_DIR)/unity/src
UNITY_OBJS        = $(BUILD_DIR)/unity.o $(BUILD_DIR)/unity_fixture.o
UNITY_DEFS        = -DUNITY_OUTPUT_COLOR -DUNITY_FIXTURE_NO_EXTRAS

# Includes
INCFLAGS = -I$(UNITY_DIR) -I$(UNITY_FIXTURE_DIR) -I$(LIBBLASTPIT_DIR) -I $(PROJECT_ROOT)/res/cfg -I$(SUBMODULES_DIR)/mongoose -I$(SUBMODULES_DIR)/mxml -I$(PROJECT_ROOT)/src/sds

# Libraries
LIBS  = -lm -ldl

# Blastpy
BLASTPY_FILES  = $(BUILD_DIR)/blastpy_wrap.o

# Debugging
DEBUG_COMMAND ?= $(shell head -n1 .debugcmd)
DEBUG_TARGET  ?= $(shell tail -n1 .debugcmd)

FORMAT_FILES        = src/libblastpit/*.{h,c} src/lmos/{lmos-tray,lmos,parser}.{hpp,cpp} src/lmos/{main,traysettings}.cpp src/video/*.{h,c,cpp} src/imgui/*.c src/scaffolding/*.c src/inkscape/*.{c,h} src/windows/*.c src/include/*.h src/include/*/*.h src/database/*.c src/lvgl/*.{c,h} src/scaps/*.{c,cpp,h}
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

libs:		 $(BUILD_DIR)/external_libs.a $(BUILD_DIR)/libblastpit.a #$(BUILD_DIR)/imgui_libs.a 
debug:		.tags
		CPPFLAGS="$(DEBUG_CPPFLAGS)" $(MAKE) CC="$(DEBUG_CC)" CXX="$(DEBUG_CXX)" libs targets python_targets wscli #imgui

# debug:	.tags targets wscli python_targets imgui lvgl
# 		$(MAKE) -f $(PROJECT_ROOT)/Makefile unit_tests
# 		@$(PROJECT_ROOT)/res/bin/run_after_build.sh

analyze:	clean
		scan-build --status-bugs -maxloop 20 --force-analyze-debug-code --exclude $(SUBMODULES_DIR) --show-description -o $(BUILD_DIR) $(MAKE) -f $(PROJECT_ROOT)/Makefile targets

profile:	$(BUILD_DIR)
		# CC="$(PROFILE_CC)" CXX="$(PROFILE_CXX)" CPPFLAGS="$(PROFILE_CPPFLAGS)" CXXFLAGS="$(PROFILE_CXXFLAGS)" INCFLAGS="$(PROFILE_INCFLAGS)" TRACY_OBJS="$(BUILD_DIR)/TracyClient.o" echo \${CC} \$(CXX) \$(CPPFLAGS) \$(CXXFLAGS) \$(TRACY_OBJS)
		# exit 1
		$(MAKE) CC="$(PROFILE_CC)" CXX="$(PROFILE_CXX)" CPPFLAGS="$(PROFILE_CPPFLAGS)" CXXFLAGS="$(PROFILE_CXXFLAGS)" INCFLAGS="$(INCFLAGS) $(PROFILE_INCFLAGS)" TRACY_OBJS="$(BUILD_DIR)/TracyClient.o" EXTERNAL_CPPFLAGS="$(PROFILE_EXTERNAL_CPPFLAGS)" $(BUILD_DIR)/TracyClient.o
		$(MAKE) CC="$(PROFILE_CC)" CXX="$(PROFILE_CXX)" CPPFLAGS="$(PROFILE_CPPFLAGS)" CXXFLAGS="$(PROFILE_CXXFLAGS)" INCFLAGS="$(INCFLAGS) $(PROFILE_INCFLAGS)" TRACY_OBJS="$(BUILD_DIR)/TracyClient.o" EXTERNAL_CPPFLAGS="$(PROFILE_EXTERNAL_CPPFLAGS)" $(EXTERNAL_OBJS)
		$(MAKE) CC="$(PROFILE_CC)" CXX="$(PROFILE_CXX)" CPPFLAGS="$(PROFILE_CPPFLAGS)" CXXFLAGS="$(PROFILE_CXXFLAGS)" INCFLAGS="$(INCFLAGS) $(PROFILE_INCFLAGS)" TRACY_OBJS="$(BUILD_DIR)/TracyClient.o" $(LIBBLASTPIT_OBJS) $(UNITY_OBJS) $(TEST_BINARIES) targets python_targets wscli imgui
		# CC="$(PROFILE_CC)" CXX="$(PROFILE_CXX)" CPPFLAGS="$(PROFILE_CPPFLAGS)" CXXFLAGS="$(PROFILE_CXXFLAGS)" INCFLAGS="$(PROFILE_INCFLAGS)" TRACY_OBJS="$(BUILD_DIR)/TracyClient.o" $(MAKE) $(LIBBLASTPIT_OBJS) $(UNITY_OBJS) $(TEST_BINARIES) $(TRACY_OBJS) targets python_targets wscli imgui
		# $(MAKE) -f $(PROJECT_ROOT)/Makefile profile_build || echo "If building tracy fails, git-clean the tracy directory and retry"

		pkill capture-release || /bin/true
		$(MAKE) -f $(PROJECT_ROOT)/Makefile tracycap

		# sleep 1
		# # This binary should contain tracy profiling as the statistics are saved in the /tmp directory
		# # We have to disable the check on the webserver for some reason
		TRACY_NO_INVARIANT_CHECK=1 $(BUILD_DIR)/ut_websocket_x
		#
		# # Give it time to write the output
		sleep 2
		$(MAKE) -f $(PROJECT_ROOT)/Makefile tracyexport

# cross:		pngs
# 		BUILD_DIR=$(PROJECT_ROOT)/build/win32 $(MAKE) -f $(PROJECT_ROOT)/Makefile libs

alltargetscheck:
		make clean
		make $(BUILD_DIR)
		make release
		# We can't just build with zig cc
		# because cimnodes fails to compile
		# so we just don't compile imgui with zig cc

		# CPPFLAGS="$(DEBUG_CPPFLAGS)" $(MAKE) $(EXTERNAL_OBJS) imgui
		# CC="zig cc" CXX="zig c++" make debug
		# CC="zig cc" CXX="zig c++" make unit_tests
		# CC="zig cc" CXX="zig c++" make system_tests
		# make clean
		# make $(BUILD_DIR)
		# CPPFLAGS="$(RELEASE_CPPFLAGS)" $(MAKE) $(EXTERNAL_OBJS) imgui
		# CC="zig cc" CXX="zig c++" make release
		# CC="zig cc" CXX="zig c++" make unit_tests
		# CC="zig cc" CXX="zig c++" make system_tests

		make clean
		make $(BUILD_DIR)
		echo asan | $(FIGLET)
		make asan
		echo test_asan | $(FIGLET)
		make test_asan
		make clean
		make $(BUILD_DIR)
		echo msan | $(FIGLET)
		make msan
		echo test_msan | $(FIGLET)
		make test_msan
		make clean
		make $(BUILD_DIR)
		echo cross | $(FIGLET)
		make cross
		echo wscli | $(FIGLET)
		make wscli_portable_x86

		make clean
		make $(BUILD_DIR)
		echo profile | $(FIGLET)
		make profile

# Prevent intermediate files being deleted and rebuilt every time
# .SECONDARY: 	$(LIBBLASTPIT_OBJS) $(TEST_OBJS)

targets:	$(BUILD_DIR) $(TEST_BINARIES)

accessories:	ueye cameras images

asan:	targets

msan:	targets

release:	$(BUILD_DIR)
		# We can't use our strict release flags with some submodules
		# because they won't compile
		CPPFLAGS="$(RELEASE_CPPFLAGS)" $(MAKE) $(EXTERNAL_OBJS)
		CPPFLAGS="$(RELEASE_CPPFLAGS)" $(MAKE) $(LIBBLASTPIT_OBJS) $(UNITY_OBJS) $(TEST_BINARIES) targets python_targets cameras wscli
		$(MAKE) imgui

.INTERMEDIATE: $(EXTERNAL_OBJS) $(UNITY_OBJS) $(LIBBLASTPIT_OBJS) $(BUILD_DIR)/TracyClient.o
$(BUILD_DIR)/external_libs.a:	$(EXTERNAL_OBJS) $(UNITY_OBJS)
		ar -crs $@ $^

$(BUILD_DIR)/libblastpit.a:	$(LIBBLASTPIT_OBJS)
		ar -crs $@ $^

$(BUILD_DIR)/libtracy.a:	$(BUILD_DIR)/TracyClient.o
		ar -crs $@ $^

$(BUILD_DIR)/libunity.a:	$(BUILD_DIR)/unity.o $(BUILD_DIR)/unity_fixture.o
		ar -crs $@ $^

# release:	targets python_targets cameras wscli imgui

# .SECONDEXPANSION:
# profile:	$(BUILD_DIR)
# 		$(MAKE) $(LIBBLASTPIT_OBJS) $(UNITY_OBJS) $(TEST_BINARIES) $(TRACY_OBJS)
#
test:	targets

# cross:	$(BUILD_DIR) $(LIBBLASTPIT_OBJS)

# ebuild:		$(BUILD_DIR) $(LIBBLASTPIT_OBJS)

clean:
		@rm -rf $(BUILD_DIR)/*{py,so,c,o,_x} $(BUILD_DIR)/{wscli,ig*,libblastpit.a} $(BUILD_DIR)/win32/* $(BUILD_DIR)/external_libs.a $(BUILD_DIR)/imgui_libs.a 2>/dev/null || /bin/true
		@rm -rf $(PROJECT_ROOT)/.{cache,ccls-cache,pytest_cache} 2>/dev/null || /bin/true
		@rm -f $(PROJECT_ROOT)/{.tags,compile_command*.json} >/dev/null 2>/dev/null || /bin/true

distclean:
		git clean -dfx || /bin/true
		git submodule foreach 'git clean -dfx' || /bin/true
		rm -rf /home/$(shell whoami)/.cache/zig 2>/dev/null || /bin/true

builddir:	$(BUILD_DIR)

$(BUILD_DIR):
	if [ ! -d "$(BUILD_DIR)" ]; then mkdir -p $(BUILD_DIR); fi
	@# ln -fs $(PROJECT_ROOT)/.git/untracked/myconfig.py $(BUILD_DIR)/


include $(SRC_DIR)/makefiles/cameras.mk
include $(SRC_DIR)/makefiles/compiling.mk
include $(SRC_DIR)/makefiles/cross_compiling.mk
include $(SRC_DIR)/makefiles/formatting.mk
include $(SRC_DIR)/makefiles/fuzzing.mk
include $(SRC_DIR)/makefiles/images.mk
include $(SRC_DIR)/makefiles/imgui.mk
#include $(SRC_DIR)/makefiles/lvgl.mk
include $(SRC_DIR)/makefiles/python.mk
include $(SRC_DIR)/makefiles/scaps.mk
include $(SRC_DIR)/makefiles/testing.mk
include $(SRC_DIR)/makefiles/tracy.mk
include $(SRC_DIR)/makefiles/wscli.mk
include $(SRC_DIR)/makefiles/helpers.mk
include $(SRC_DIR)/makefiles/doxygen.mk
include $(SRC_DIR)/makefiles/lmos.mk
