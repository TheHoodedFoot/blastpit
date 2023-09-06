# ░█▀▀░█░█░▀▀█░▀▀█░▀█▀░█▀█░█▀▀
# ░█▀▀░█░█░▄▀░░▄▀░░░█░░█░█░█░█
# ░▀░░░▀▀▀░▀▀▀░▀▀▀░▀▀▀░▀░▀░▀▀▀

FUZZING_TIMEOUT    ?= 60s
FUZZING_INPUT_DIR  ?= $(SRC_DIR)/fuzzing/afl_i
FUZZING_OUTPUT_DIR ?= $(BUILD_DIR)/afl_o
FUZZING_BINARY      = $(BUILD_DIR)/stdin_test

fuzz:		$(FUZZING_BINARY)
		AFL_SKIP_CPUFREQ=1 afl-fuzz -V $(FUZZING_TIMEOUT) -i $(FUZZING_INPUT_DIR) -o $(FUZZING_OUTPUT_DIR) -- $(FUZZING_BINARY) || /bin/true
		@$$( if [[ $$(ls $(FUZZING_OUTPUT_DIR)/default/crashes | wc -l) == 0 ]]; then /bin/true; else /bin/false; fi )

fuzzbot:	fuzz

$(FUZZING_BINARY):	$(SRC_DIR)/fuzzing/stdin_template_afl.c
	afl-clang-fast -o $@ $^
