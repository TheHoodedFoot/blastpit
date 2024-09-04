# ░▀█▀░█▄█░█▀▀░█░█░▀█▀
# ░░█░░█░█░█░█░█░█░░█░
# ░▀▀▀░▀░▀░▀▀▀░▀▀▀░▀▀▀

CIMGUI_DIR         = $(SUBMODULES_DIR)/cimgui
IMGUI_DIR          = $(CIMGUI_DIR)/imgui
CIMNODES_DIR         = $(SUBMODULES_DIR)/cimnodes
IMNODES_DIR         = $(SUBMODULES_DIR)/cimnodes/imnodes

CIMGUI_CPP_INC     = -I$(CIMGUI_DIR) -I$(IMGUI_DIR) -I$(IMGUI_DIR)/examples/libs/glfw/include
CIMGUI_C_INC       = -I$(CIMGUI_DIR) -I$(CIMNODES_DIR) -I$(CIMGUI_DIR)/generator/output -I$(PROJECT_ROOT)/src/imgui -I$(PROJECT_ROOT)/src/include -I$(SUBMODULES_DIR)/mxml -I$(PROJECT_ROOT)/src/sds $(shell pkg-config --cflags libpq)

CIMGUI_CPP_DEFINES = -DIMGUI_IMPL_API="extern \"C\"" -DIMGUI_IMPL_OPENGL_LOADER_GL3W -DIMNODES_NAMESPACE=imnodes
CIMGUI_C_DEFINES   = -DIMGUI_IMPL_API=" " -DCIMGUI_USE_OPENGL3 -DBUILD_DIR=\"$(BUILD_DIR)/\" -DPROJECT_ROOT=\"$(PROJECT_ROOT)/\" -DIMNODES_NAMESPACE=imnodes
CIMGUI_LIBS        = -lm -lGL -ldl -lglfw $(shell pkg-config --libs libpq)

CIMGUI_SRCS        = cimgui imgui imgui_impl_glfw imgui_impl_opengl3 imgui_demo imgui_draw imgui_widgets imgui_tables gl3w ig_common ink_common psql imnodes cimnodes 
CIMGUI_OBJS        = $(patsubst %,$(BUILD_DIR)/%.o,$(notdir $(CIMGUI_SRCS)))
# .SECONDARY:		$(CIMGUI_OBJS) $(BUILD_DIR)/ig_ringgen.o $(BUILD_DIR)/ig_multipass.o $(BUILD_DIR)/ig_common.o $(BUILD_DIR)/ig_database.o $(BUILD_DIR)/psql.o $(BUILD_DIR)/ig_cimnodes.o $(BUILD_DIR)/ig_template.o

# Tracy
# TRACY_CC                =  $(CXX) # Needed for tracy, which is c++
# TRACY_CPPFLAGS          += -rdynamic
# TRACY_CPPFLAGS          += -g3 -fno-omit-frame-pointer
# TRACY_CPPFLAGS          += -pg --coverage
# TRACY_CPPFLAGS          += -fpermissive # Tracy needs c++ compiler which is less forgiving of errors
# TRACY_EXTERNAL_CPPFLAGS =  -fno-sanitize=all -fpermissive
# TRACY_EXTERNAL_CPPFLAGS = 
# TRACY_OBJS =

imgui:	$(BUILD_DIR)/ig_multipass $(BUILD_DIR)/ig_ringgen $(BUILD_DIR)/ig_database $(BUILD_DIR)/ig_cimnodes $(BUILD_DIR)/ig_template

imgui_run:	$(BUILD_DIR)/ig_database
	@if [ ! -f "$(BUILD_DIR)/MyImage01.jpg" ]; then cp $(PROJECT_ROOT)/res/img/MyImage01.jpg $(BUILD_DIR); fi
		$^

.INTERMEDIATE: $(CIMGUI_OBJS) $(EXTERNAL_OBJS) $(UNITY_OBJS) $(LIBBLASTPIT_OBJS)
$(BUILD_DIR)/imgui_libs.a:	$(CIMGUI_OBJS)
		ar -crs $@ $^

$(BUILD_DIR)/ig_%:	$(BUILD_DIR)/ig_%.o $(BUILD_DIR)/imgui_libs.a $(BUILD_DIR)/external_libs.a
	$(CXX) $(CPPFLAGS) -Wfatal-errors -L. $^ $(TRACY_OBJS) -o $@ $(CIMGUI_LIBS)

$(BUILD_DIR)/%.o:	$(CIMGUI_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) -o $@ $(CIMGUI_CPP_DEFINES) $(CIMGUI_CPP_INC) -c $^

$(BUILD_DIR)/%.o:	$(IMGUI_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) -o $@ $(CIMGUI_CPP_DEFINES) $(CIMGUI_CPP_INC) -c $^

$(BUILD_DIR)/%.o:	$(CIMNODES_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) -o $@ -I $(CIMNODES_DIR) -I $(IMNODES_DIR) $(CIMGUI_CPP_DEFINES) $(CIMGUI_CPP_INC) -c $^

$(BUILD_DIR)/%.o:	$(IMNODES_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) -o $@ -I $(IMNODES_DIR) $(CIMGUI_CPP_DEFINES) -DIMGUI_DEFINE_MATH_OPERATORS -Wno-error $(CIMGUI_CPP_INC) -c $^

$(BUILD_DIR)/%.o:	$(IMGUI_DIR)/backends/%.cpp | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) -o $@ $(CIMGUI_CPP_DEFINES) $(CIMGUI_CPP_INC) -c $^

$(BUILD_DIR)/%.o:	$(PROJECT_ROOT)/src/sds/%.c | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) -o $@ $(CIMGUI_C_DEFINES) $(CIMGUI_C_INC) $(UNITY_DEFS) -I$(UNITY_FIXTURE_DIR) -I$(UNITY_DIR) -c $^

$(BUILD_DIR)/%.o:	$(PROJECT_ROOT)/src/imgui/%.c | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) -o $@ $(CIMGUI_C_DEFINES) $(CIMGUI_C_INC) $(TRACY_INCFLAGS) -c $^

$(BUILD_DIR)/%.o:	$(PROJECT_ROOT)/src/inkscape/%.c | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) -o $@ $(CIMGUI_C_DEFINES) $(CIMGUI_C_INC) $(TRACY_INCFLAGS) $(UNITY_DEFS) -I$(UNITY_FIXTURE_DIR) -I$(UNITY_DIR) -c $^

# Trigger rebuild if header changes?
$(PROJECT_ROOT)/src/inkscape/%.c:	$(PROJECT_ROOT)/src/inkscape/ig_common.h

imgui_help:
	xdg-open "https://pthom.github.io/imgui_manual_online/manual/imgui_manual.html"

valimgui:	$(BUILD_DIR)/ink_imgui
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrind-out.log $^
