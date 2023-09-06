# ░█░░░█░█░█▀▀░█░░
# ░█░░░▀▄▀░█░█░█░░
# ░▀▀▀░░▀░░▀▀▀░▀▀▀

LVGL_PATH ?= $(SUBMODULES_DIR)/lvgl/src
LV_DRIVERS_PATH = $(SUBMODULES_DIR)/lv_drivers

LVGL_CORE_SRCS += $(shell find $(LVGL_PATH)/core -type f -name '*.c')
LVGL_CORE_OBJS += $(patsubst %.c,%.o,$(LVGL_CORE_SRCS))

LVGL_DRAW_SRCS += $(shell find $(LVGL_PATH)/draw -maxdepth 1 -type f -name '*.c')
LVGL_DRAW_SRCS += $(shell find $(LVGL_PATH)/draw/sw -type f -name '*.c')
LVGL_DRAW_OBJS += $(patsubst %.c,%.o,$(LVGL_DRAW_SRCS))

LVGL_SDL_SRCS += $(shell find $(LVGL_PATH)/draw/sdl -type f -name '*.c')
LVGL_SDL_OBJS += $(patsubst %.c,%.o,$(LVGL_SDL_SRCS))

LVGL_FONT_SRCS += $(shell find $(LVGL_PATH)/font -type f -name '*.c')
LVGL_FONT_OBJS += $(patsubst %.c,%.o,$(LVGL_FONT_SRCS))

LVGL_HAL_SRCS += $(shell find $(LVGL_PATH)/hal -type f -name '*.c')
LVGL_HAL_OBJS += $(patsubst %.c,%.o,$(LVGL_HAL_SRCS))

# LVGL_LAYOUTS_SRCS += $(shell find $(LVGL_PATH)/layouts -type f -name '*.c')
# LVGL_LAYOUTS_OBJS += $(patsubst %.c,%.o,$(LVGL_LAYOUTS_SRCS))

# LVGL_LIBS_SRCS += $(shell find $(LVGL_PATH)/libs -type f -name '*.c')
# LVGL_LIBS_OBJS += $(patsubst %.c,%.o,$(LVGL_LIBS_SRCS))

LVGL_MISC_SRCS += $(shell find $(LVGL_PATH)/misc -type f -name '*.c')
LVGL_MISC_OBJS += $(patsubst %.c,%.o,$(LVGL_MISC_SRCS))

# LVGL_OTHERS_SRCS += $(shell find $(LVGL_PATH)/others -type f -name '*.c')
# LVGL_OTHERS_OBJS += $(patsubst %.c,%.o,$(LVGL_OTHERS_SRCS))

# LVGL_THEMES_SRCS += $(shell find $(LVGL_PATH)/themes -type f -name '*.c')
# LVGL_THEMES_OBJS += $(patsubst %.c,%.o,$(LVGL_THEMES_SRCS))

LVGL_WIDGETS_SRCS += $(shell find $(LVGL_PATH)/widgets -type f -name '*.c')
LVGL_WIDGETS_OBJS += $(patsubst %.c,%.o,$(LVGL_WIDGETS_SRCS))

LVGL_EXTRAS_SRCS += $(shell find $(LVGL_PATH)/extra -type f -name '*.c')
LVGL_EXTRAS_OBJS += $(patsubst %.c,%.o,$(LVGL_EXTRAS_SRCS))

LV_DRIVERS_SDL_SRCS += $(shell find $(LV_DRIVERS_PATH)/sdl -type f -name '*.c')
LV_DRIVERS_SDL_OBJS += $(patsubst %.c,%.o,$(LV_DRIVERS_SDL_SRCS))

LV_DRIVERS_INDEV_SRCS += $(shell find $(LV_DRIVERS_PATH)/indev -type f -name '*.c')
LV_DRIVERS_INDEV_OBJS += $(patsubst %.c,%.o,$(LV_DRIVERS_INDEV_SRCS))

LVGL_LIBRARY = $(BUILD_DIR)/lvgl.a
LVGL_LIBRARY_OBJS = $(LVGL_CORE_OBJS) $(LVGL_DRAW_OBJS) $(LVGL_SDL_OBJS) $(LVGL_FONT_OBJS) $(LVGL_HAL_OBJS) $(LVGL_LAYOUTS_OBJS) $(LVGL_LIBS_OBJS) $(LVGL_MISC_OBJS) $(LVGL_OTHERS_OBJS) $(LVGL_THEMES_OBJS) $(LVGL_WIDGETS_OBJS) $(LVGL_EXTRAS_OBJS)

LV_DRIVERS_LIBRARY = $(BUILD_DIR)/lv_drivers.a
LV_DRIVERS_LIBRARY_OBJS = $(LV_DRIVERS_SDL_OBJS) $(LV_DRIVERS_INDEV_OBJS)

# -DLV_CONF_SKIP requires all parameters to be specified as defines

LVFLAGS = -DLV_CONF_SKIP -DLV_CONF_INCLUDE_SIMPLE -DLV_DRV_NO_CONF \
	  -DUSE_SDL -DSDL_HOR_RES=480 -DSDL_VER_RES=272 -DSDL_ZOOM=1 \
	  -DSDL_INCLUDE_PATH=\"SDL2/SDL.h\" \
	  -DLV_MEM_CUSTOM=1 "-DLV_MEM_SIZE=(128U * 1024U)" \
	  -DLV_FONT_MONTSERRAT_10 \
	  -DLV_FONT_MONTSERRAT_16 -DLV_FONT_MONTSERRAT_24 -DLV_FONT_MONTSERRAT_48 \

	  # -DLV_LOG_PRINTF=1 -DLV_LOG_LEVEL=LV_LOG_LEVEL_INFO \

# LVFLAGS = -DLV_CONF_SKIP -DLV_CONF_INCLUDE_SIMPLE -DLV_LOG_PRINTF=1 -DUSE_SDL -DSDL_HOR_RES=960 -DSDL_VER_RES=640 -DSDL_ZOOM=1 -DSDL_INCLUDE_PATH=\"SDL2/SDL.h\" -DLV_MEM_CUSTOM=1 "-DLV_MEM_SIZE=(128U * 1024U)" -DLV_FONT_MONTSERRAT_16 -DLV_FONT_MONTSERRAT_24 -DLV_FONT_MONTSERRAT_48
# LVFLAGS += -DLV_LVGL_H_INCLUDE_SIMPLE -DLV_DRV_NO_CONF 

LV_INCDIRS = -I$(SRC_DIR)/lvgl -I$(SUBMODULES_DIR) -I$(SUBMODULES_DIR)/lvgl -I$(SUBMODULES_DIR)/lvgl/src/extra -I$(SUBMODULES_DIR)/lv_drivers $(shell pkg-config --cflags sdl2) 

.PHONY:			lvgl

lvgl:			$(BUILD_DIR)/lvgl_multipass

$(BUILD_DIR)/lvgl_multipass:	$(BUILD_DIR)/lvgl_multipass.o $(BUILD_DIR)/lvgl_hal.o | $(LVGL_LIBRARY) $(LV_DRIVERS_LIBRARY)
			$(CC) -o $@ $(CPPFLAGS) $^ $(LVGL_LIBRARY) $(LV_DRIVERS_LIBRARY) $(shell pkg-config --libs sdl2)

$(BUILD_DIR)/%.o: $(SRC_DIR)/lvgl/%.c | $(BUILD_DIR)
			$(CC) -o $@ $(CPPFLAGS) -c $(LVFLAGS) $(LV_INCDIRS) $^

lvgl_libs:		$(LVGL_LIBRARY) $(LV_DRIVERS_LIBRARY)

$(LVGL_LIBRARY):	$(LVGL_LIBRARY_OBJS) | $(BUILD_DIR)
			ar -crs $@ $^
			ranlib $@

$(LV_DRIVERS_LIBRARY):	$(LV_DRIVERS_LIBRARY_OBJS) | $(BUILD_DIR)
			ar -crs $@ $^
			ranlib $@

%.o:			%.c
			@# LVGL has unused parameters preventing compilation as at 25/10/22
			$(CC) -o $@ $(CPPFLAGS) -Wno-error=unused-parameter -c $(LVFLAGS) $(LV_INCDIRS) $^

lvgl_clean:
			rm -f $(LVGL_LIBRARY) $(LVGL_LIBRARY_OBJS) $(LV_DRIVERS_LIBRARY) $(LV_DRIVERS_LIBRARY_OBJS)
