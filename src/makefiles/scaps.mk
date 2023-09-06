# ░█▀▀░█▀▀░█▀█░█▀█░█▀▀
# ░▀▀█░█░░░█▀█░█▀▀░▀▀█
# ░▀▀▀░▀▀▀░▀░▀░▀░░░▀▀▀

PLATFORM   = x86_64-windows-gnu
WINEDIR    = scaps
WINEPRE    = /home/$(shell whoami)/wine/$(WINEDIR)
WINEACH    = win64
WINEBINARY = wine64
TLB_INCDIR = $(WINEPRE)/drive_c/scaps/sam2d/tlb_include

SCAPS_SRC  = $(SRC_DIR)/scaps/scaps.c $(SRC_DIR)/scaps/sc_samlight_client_ctrl_ex_type_lib_i.c
SCAPS_EXE  = $(BUILD_DIR)/scaps.exe
SCAPS_LIBS = $(WINEPRE)/drive_c/windows/system32/sc_ex64_samlight_client_ctrl.dll -lole32 -loleaut32 -luuid
SCAPS_START_SCRIPT = $(PROJECT_ROOT)/res/bin/scaps.sh

scaps:		$(SCAPS_EXE)

scaps_run:	$(SCAPS_EXE)
		WINEPREFIX=$(WINEPRE) WINEARCH=$(WINEACH) $(WINEBINARY) $^ --download #2>/dev/null

scaps_server:	$(SCAPS_START_SCRIPT)
		$^ &

scaps_debug:	$(SCAPS_EXE)
		WINEPREFIX=$(WINEPRE) WINEARCH=$(WINEACH) winedbg --gdb $^ 2>/dev/null

$(SCAPS_EXE):	$(SCAPS_SRC) | $(BUILD_DIR)
		zig cc -DCOBJMACROS -target $(PLATFORM) -ggdb3 -o $@ -I $(TLB_INCDIR) -I$(SRC_DIR)/scaps $^ $(SCAPS_LIBS)

scaps_doc:
		qpdfview $(PROJECT_ROOT)/.git/untracked/optogon/sc_SAMLight_en.pdf $(PROJECT_ROOT)/.git/untracked/optogon/sc_Client_Control_Interface.pdf &
