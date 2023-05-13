# ░▀█▀░█▄█░█▀▀░█░█░▀█▀
# ░░█░░█░█░█░█░█░█░░█░
# ░▀▀▀░▀░▀░▀▀▀░▀▀▀░▀▀▀

CIMGUI_DIR         = $(SUBMODULES_DIR)/cimgui
IMGUI_DIR          = $(CIMGUI_DIR)/imgui
CIMGUI_CPP_INC     = -I$(CIMGUI_DIR) -I$(IMGUI_DIR) -I$(IMGUI_DIR)/examples/libs/glfw/include
CIMGUI_C_INC       = -I$(CIMGUI_DIR) -I$(CIMGUI_DIR)/generator/output -I$(PROJECT_ROOT)/src/imgui -I$(PROJECT_ROOT)/src/include -I$(SUBMODULES_DIR)/mxml -I$(SRC_DIR)/sds $(shell pkg-config --cflags libpq)
CIMGUI_CPP_DEFINES = -DIMGUI_IMPL_API="extern \"C\"" -DIMGUI_IMPL_OPENGL_LOADER_GL3W
CIMGUI_C_DEFINES   = -DIMGUI_IMPL_API=" " -DCIMGUI_USE_OPENGL3 -DBUILD_DIR=\"$(BUILD_DIR)/\" -DPROJECT_ROOT=\"$(PROJECT_ROOT)/\"
CIMGUI_LIBS        = -lm -lGL -ldl -lglfw $(shell pkg-config --libs libpq)

CIMGUI_SRCS        = cimgui imgui imgui_impl_glfw imgui_impl_opengl3 imgui_demo imgui_draw imgui_widgets imgui_tables gl3w sds ig_common ink_common psql
CIMGUI_OBJS        = $(patsubst %,$(BUILD_DIR)/%.o,$(notdir $(CIMGUI_SRCS)))
.SECONDARY:		$(CIMGUI_OBJS) $(BUILD_DIR)/ig_multipass_main.o $(BUILD_DIR)/ig_ringgen_main.o $(BUILD_DIR)/ig_ringgen.o $(BUILD_DIR)/ig_multipass.o $(BUILD_DIR)/ig_common.o $(BUILD_DIR)/ig_database.o $(BUILD_DIR)/psql.o

imgui:	$(BUILD_DIR)/ig_multipass $(BUILD_DIR)/ig_ringgen $(BUILD_DIR)/ig_database

imgui_run:	$(BUILD_DIR)/ig_database
	@if [ ! -f "$(BUILD_DIR)/MyImage01.jpg" ]; then cp $(PROJECT_ROOT)/res/img/MyImage01.jpg $(BUILD_DIR); fi
		$^

$(BUILD_DIR)/ig_%:	$(BUILD_DIR)/ig_%_main.o $(BUILD_DIR)/ig_%.o $(CIMGUI_OBJS) | $(BUILD_DIR) $(LIBMXML_OBJS)
	$(CXX) $(CPPFLAGS) -Wfatal-errors -o $@ $(LIBMXML_OBJS) $^ $(CIMGUI_LIBS)

$(BUILD_DIR)/%.o:	$(CIMGUI_DIR)/%.cpp
	$(CXX) $(CPPFLAGS) -o $@ $(CIMGUI_CPP_DEFINES) $(CIMGUI_CPP_INC) -c $^

$(BUILD_DIR)/%.o:	$(IMGUI_DIR)/%.cpp
	$(CXX) $(CPPFLAGS) -o $@ $(CIMGUI_CPP_DEFINES) $(CIMGUI_CPP_INC) -c $^

$(BUILD_DIR)/%.o:	$(IMGUI_DIR)/backends/%.cpp
	$(CXX) $(CPPFLAGS) -o $@ $(CIMGUI_CPP_DEFINES) $(CIMGUI_CPP_INC) -c $^

$(BUILD_DIR)/%.o:	$(PROJECT_ROOT)/src/imgui/%.c
	$(CC) $(CPPFLAGS) -o $@ $(CIMGUI_C_DEFINES) $(CIMGUI_C_INC) -c $^

$(BUILD_DIR)/%.o:	$(PROJECT_ROOT)/src/inkscape/%.c
	$(CC) $(CPPFLAGS) -o $@ $(CIMGUI_C_DEFINES) $(CIMGUI_C_INC) $(UNITY_DEFS) -I$(UNITY_FIXTURE_DIR) -I$(UNITY_DIR) -c $^

# Trigger rebuild if header changes?
$(PROJECT_ROOT)/src/inkscape/%.c:	$(PROJECT_ROOT)/src/inkscape/ig_common.h

imgui_help:
	xdg-open "https://pthom.github.io/imgui_manual_online/manual/imgui_manual.html"

valimgui:	$(BUILD_DIR)/ink_imgui
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrind-out.log $^
