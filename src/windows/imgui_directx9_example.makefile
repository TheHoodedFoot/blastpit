PROJECT_ROOT   := $(shell git rev-parse --show-toplevel)
PROJECT        := $(shell basename $(PROJECT_ROOT))
DOTGIT_DIR     := $(shell $(PROJECT_ROOT)/res/bin/get_dotgit_dir.sh)
BUILD_DIR      ?= $(PROJECT_ROOT)/build/win32
SUBMODULES_DIR := $(PROJECT_ROOT)/src/submodules
IMGUI_DIR       = $(SUBMODULES_DIR)/cimgui/imgui
DEMO_DIR = $(IMGUI_DIR)/examples/example_win32_directx9

# TARGET         ?= x86_64
TARGET         ?= i386

DEMO_SRCS = main.cpp
IMGUI_SRCS = imgui.cpp imgui_draw.cpp imgui_tables.cpp imgui_widgets.cpp imgui_demo.cpp
WIN32_SRCS = imgui_impl_dx9.cpp imgui_impl_win32.cpp
SOURCES = $(DEMO_SRCS) $(IMGUI_SRCS) $(WIN32_SRCS)

OBJS        = $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SOURCES))

INCLUDES = -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends

WINE_LIBDIR = /usr/lib/$(shell ls /usr/lib | grep wine-staging | sort -V | tail -n1)/wine/i386-unix
LIBS = -ld3d9 -lgdi32 -ldwmapi

CXX = zig c++ -target $(TARGET)-windows-gnu

all:	$(BUILD_DIR) $(BUILD_DIR)/imgui_dx9.exe

$(BUILD_DIR)/imgui_dx9.exe:	$(OBJS) | $(BUILD_DIR)
	$(CXX) -o $@ $^ -L$(WINE_LIBDIR) $(LIBS)

$(BUILD_DIR)/%.o:	$(DEMO_DIR)/%.cpp
	$(CXX) $(INCLUDES) -c -o $@ $^

$(BUILD_DIR)/%.o:	$(IMGUI_DIR)/%.cpp
	$(CXX) $(INCLUDES) -c -o $@ $^

$(BUILD_DIR)/%.o:	$(IMGUI_DIR)/backends/%.cpp
	$(CXX) $(INCLUDES) -c -o $@ $^

.PHONY:	$(BUILD_DIR) clean

$(BUILD_DIR):
		mkdir -p $(BUILD_DIR)

clean:
	rm -f $(BUILD_DIR)/*.o
