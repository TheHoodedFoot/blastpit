# Obtain the precompiled glfw library (https://www.glfw.org/download)
# and extract the file glfw-3.3.8.bin.WIN64/lib-mingw-w64/libglfw3.a

PROJECT_ROOT   := $(shell git rev-parse --show-toplevel)
PROJECT        := $(shell basename $(PROJECT_ROOT))
DOTGIT_DIR     := $(shell $(PROJECT_ROOT)/res/bin/get_dotgit_dir.sh)
BUILD_DIR      ?= $(PROJECT_ROOT)/build/win32
SUBMODULES_DIR := $(PROJECT_ROOT)/src/submodules

# TARGET         ?= x86_64
TARGET         ?= i386

CC              = zig cc -target $(TARGET)-windows-gnu
CXX             = zig c++ -target $(TARGET)-windows-gnu
CPPFLAGS        = -Wfatal-errors -Wall -Wpedantic -Wextra -MMD
CPPFLAGS       += -Ofast -g3 -fno-omit-frame-pointer

.PHONY: all clean help glfw rasterizer

all: 		$(BUILD_DIR)/glfw.exe $(BUILD_DIR)/imgui.exe $(BUILD_DIR)/rasterizer.exe

$(BUILD_DIR):
		mkdir -p $(BUILD_DIR)

clean:
		rm -rf $(BUILD_DIR)

# nanosvg

NANOSVG_DIR     = $(SUBMODULES_DIR)/nanosvg

$(BUILD_DIR)/glfw.exe:	glfw_example.c | $(BUILD_DIR)
		$(CC) $^ -o $@ -I. -I$(PROJECT_ROOT)/src/include -I $(NANOSVG_DIR)/src -lgdi32 -lopengl32 $(TARGET)_libglfw3.a

glfw:		$(BUILD_DIR)/glfw.exe
		wine $(BUILD_DIR)/$^

$(BUILD_DIR)/rasterizer.exe:	rasterizer.c | $(BUILD_DIR)
		$(CC) $^ -o $@ -I. -I$(PROJECT_ROOT)/src/include -I $(NANOSVG_DIR)/src -I $(NANOSVG_DIR)/example -lm

rasterizer:	$(BUILD_DIR)/rasterizer.exe
		wine $(BUILD_DIR)/$^


# imgui

CIMGUI_DIR     = $(SUBMODULES_DIR)/cimgui
IMGUI_DIR      = $(CIMGUI_DIR)/imgui

CPP_INC        = -I$(CIMGUI_DIR) -I$(IMGUI_DIR) -I$(IMGUI_DIR)/examples/libs/glfw/include
C_INC          = -I$(CIMGUI_DIR) -I$(CIMGUI_DIR)/generator/output -I$(PROJECT_ROOT)/src/imgui -I$(PROJECT_ROOT)/src/include -DBUILD_DIR=\"$(BUILD_DIR)/\"

CPP_DEFINES    = -DIMGUI_IMPL_API="extern \"C\"" -DIMGUI_IMPL_OPENGL_LOADER_GL3W
C_DEFINES      = -DIMGUI_IMPL_API=" " -DCIMGUI_USE_OPENGL3
LIBS           = -lgdi32 -lopengl32 $(TARGET)_libglfw3.a

OBJS = $(CIMGUI_DIR)/cimgui.o $(IMGUI_DIR)/imgui.o $(IMGUI_DIR)/backends/imgui_impl_glfw.o $(IMGUI_DIR)/backends/imgui_impl_opengl3.o $(IMGUI_DIR)/imgui_demo.o $(IMGUI_DIR)/imgui_draw.o $(IMGUI_DIR)/imgui_widgets.o $(IMGUI_DIR)/imgui_tables.o $(PROJECT_ROOT)/src/windows/imgui_test.o $(PROJECT_ROOT)/src/windows/gl3w.o
BOBS = $(patsubst %,$(BUILD_DIR)/%,$(notdir $(OBJS)))

.SECONDARY:	$(BOBS)

imgui:		imgui.exe
		ln -sf $(PROJECT_ROOT)/res/img/MyImage01.jpg $(BUILD_DIR)
		wine $^

$(BUILD_DIR)/%.o:	$(CIMGUI_DIR)/%.cpp
	$(CXX) $(CPPFLAGS) $(CPP_DEFINES) $(CPP_INC) -o $@ -c $^

$(BUILD_DIR)/%.o:	$(IMGUI_DIR)/%.cpp
	$(CXX) $(CPPFLAGS) $(CPP_DEFINES) $(CPP_INC) -o $@ -c $^

$(BUILD_DIR)/%.o:	$(IMGUI_DIR)/backends/%.cpp
	$(CXX) $(CPPFLAGS) $(CPP_DEFINES) $(CPP_INC) -o $@ -c $^

$(BUILD_DIR)/%.o:	%.c
	$(CC) $(CPPFLAGS) $(C_DEFINES) $(C_INC) -o $@ -c $^

$(BUILD_DIR)/imgui.exe:	$(BOBS) | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) -Wfatal-errors -o $@ $(BOBS) $(LIBS)

help:
	xdg-open "https://pthom.github.io/imgui_manual_online/manual/imgui_manual.html"
