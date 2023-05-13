# ░▀█▀░█▀▄░█▀█░█▀▀░█░█
# ░░█░░█▀▄░█▀█░█░░░░█░
# ░░▀░░▀░▀░▀░▀░▀▀▀░░▀░

TRACY_PROFILER_DIR = $(SUBMODULES_DIR)/tracy/profiler/build/unix
TRACY_PROFILER_BINARY = $(TRACY_PROFILER_DIR)/Tracy-release
TRACY_CAPTURE_DIR = $(SUBMODULES_DIR)/tracy/capture/build/unix
TRACY_CAPTURE_BINARY = $(TRACY_CAPTURE_DIR)/capture-release
TRACY_CSVEXPORT_DIR = $(SUBMODULES_DIR)/tracy/csvexport/build/unix
TRACY_CSVEXPORT_BINARY = $(TRACY_CSVEXPORT_DIR)/csvexport-release

tracy:	$(TRACY_PROFILER_BINARY)
	$^ -a 127.0.0.1 &

tracycap:	$(TRACY_CAPTURE_BINARY)
	$^ -f -o /tmp/tracycap.tracy &

tracyexport:	$(TRACY_CSVEXPORT_BINARY)
	$^ /tmp/tracycap.tracy > /tmp/tracyexport_$(PROJECT)_$(GIT_HEAD).csv

tracyopen:	$(TRACY_PROFILER_BINARY)
	$^ /tmp/tracycap.tracy

tracy_help:
	xdg-open $(PROJECT_ROOT)/.git/untracked/docs/tracy.pdf

$(TRACY_PROFILER_BINARY):	$(TRACY_PROFILER_DIR)
	cd $(TRACY_PROFILER_DIR); CPLUS_INCLUDE_PATH=/usr/include/capstone make

$(TRACY_CAPTURE_BINARY):	$(TRACY_CAPTURE_DIR)
	cd $(TRACY_CAPTURE_DIR); CPLUS_INCLUDE_PATH=/usr/include/capstone make

$(TRACY_CSVEXPORT_BINARY):	$(TRACY_CSVEXPORT_DIR)
	cd $(TRACY_CSVEXPORT_DIR); CPLUS_INCLUDE_PATH=/usr/include/capstone make
