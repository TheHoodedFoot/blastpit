# ░█░░░█▄█░█▀█░█▀▀
# ░█░░░█░█░█░█░▀▀█
# ░▀▀▀░▀░▀░▀▀▀░▀▀▀

PLATFORM   = x86_64-windows-gnu
WINEDIR    = scaps
WINEPRE    = /home/$(shell whoami)/wine/$(WINEDIR)

LMOS_SRC  = $(SRC_DIR)/lmos/lmos.c
LMOS_EXE  = $(BUILD_DIR)/lmos.exe
LMOS_LIBS =  -lole32 -loleaut32 -luuid -lwsock32
# LMOS_START_SCRIPT = $(PROJECT_ROOT)/res/bin/lmos.sh

FLAGS = -g3 -O0 -fno-omit-frame-pointer -fno-stack-protector

lmoscom:	$(LMOS_EXE)

lmos_run:	$(LMOS_EXE)
		WINEPREFIX=$(WINEPRE) wine $^ 2>/dev/null

# lmos_server:	$(LMOS_START_SCRIPT)
# 		$^ &

lmos_debug:	$(LMOS_EXE)
		WINEPREFIX=$(WINEPRE) winedbg --gdb $^ 2>/dev/null

$(LMOS_EXE):	$(LMOS_SRC) | $(BUILD_DIR)
		zig cc -target $(PLATFORM) $(FLAGS) -o $@ -I$(SRC_DIR)/lmos $^ $(LMOS_LIBS)

