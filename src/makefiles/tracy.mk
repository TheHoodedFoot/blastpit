# ░▀█▀░█▀▄░█▀█░█▀▀░█░█
# ░░█░░█▀▄░█▀█░█░░░░█░
# ░░▀░░▀░▀░▀░▀░▀▀▀░░▀░

# When building TracyClient.o, ensure that TRACY_ENABLE is set,
# otherwise symbols will not be compiled
#
# When compiling C files for Tracy debugging,
# do not #include "Tracy.hpp". Instead, #include "TracyC.h"

TRACY_PROFILER_DIR = $(SUBMODULES_DIR)/tracy/profiler
TRACY_PROFILER_BINARY = $(TRACY_PROFILER_DIR)/build/tracy-profiler
TRACY_CAPTURE_DIR = $(SUBMODULES_DIR)/tracy/capture
TRACY_CAPTURE_BINARY = $(TRACY_CAPTURE_DIR)/build/tracy-capture
TRACY_CSVEXPORT_DIR = $(SUBMODULES_DIR)/tracy/csvexport
TRACY_CSVEXPORT_BINARY = $(TRACY_CSVEXPORT_DIR)/build/tracy-csvexport

TRACY_USE_X ?= TRUE

tracy:	$(TRACY_PROFILER_BINARY)
	$^ -a 127.0.0.1 &

tracycap:	$(TRACY_CAPTURE_BINARY)
	$^ -f -o /tmp/tracycap.tracy -s 30 &

tracyexport:	$(TRACY_CSVEXPORT_BINARY)
	$^ /tmp/tracycap.tracy > /tmp/tracyexport_$(PROJECT)_$(GIT_HEAD).csv
	# rm -f /tmp/tracycap.tracy

tracyopen:	$(TRACY_PROFILER_BINARY)
	$^ /tmp/tracycap.tracy

tracy_help:
	xdg-open $(PROJECT_ROOT)/.git/untracked/docs/tracy.pdf

$(TRACY_PROFILER_DIR)/Makefile:
	CPM_SOURCE_CACHE=TRUE cmake -S $(TRACY_PROFILER_DIR) -B $(TRACY_PROFILER_DIR)/build -DCMAKE_BUILD_TYPE=Release -DLEGACY=$(TRACY_USE_X) -DDOWNLOAD_CAPSTONE=OFF -DFETCHCONTENT_QUIET=OFF

$(TRACY_CAPTURE_DIR)/Makefile:
	CPM_SOURCE_CACHE=TRUE cmake -S $(TRACY_CAPTURE_DIR) -B $(TRACY_CAPTURE_DIR)/build -DCMAKE_BUILD_TYPE=Release -DLEGACY=$(TRACY_USE_X) -DDOWNLOAD_CAPSTONE=OFF -DFETCHCONTENT_QUIET=OFF

$(TRACY_CSVEXPORT_DIR)/Makefile:
	CPM_SOURCE_CACHE=TRUE cmake -S $(TRACY_CSVEXPORT_DIR) -B $(TRACY_CSVEXPORT_DIR)/build -DCMAKE_BUILD_TYPE=Release -DLEGACY=$(TRACY_USE_X) -DDOWNLOAD_CAPSTONE=OFF -DFETCHCONTENT_QUIET=OFF

$(TRACY_PROFILER_BINARY):	$(TRACY_PROFILER_DIR)/Makefile
	make -C $(TRACY_PROFILER_DIR)/build

$(TRACY_CAPTURE_BINARY):	$(TRACY_CAPTURE_DIR)/Makefile
	make -C $(TRACY_CAPTURE_DIR)/build

$(TRACY_CSVEXPORT_BINARY):	$(TRACY_CSVEXPORT_DIR)/Makefile
	make -C $(TRACY_CSVEXPORT_DIR)/build
