# ░▀█▀░█▀▀░█▀▀░▀█▀░▀█▀░█▀█░█▀▀
# ░░█░░█▀▀░▀▀█░░█░░░█░░█░█░█░█
# ░░▀░░▀▀▀░▀▀▀░░▀░░▀▀▀░▀░▀░▀▀▀

unit_tests: $(TEST_BINARIES) #$(BUILD_DIR)/_blastpy.so
	@# Unit test output prevents vim-dispatch quickfix window from closing on successful build
	@# So we log the output and only display it if the tests fail
	@echo $(TEST_BINARIES) | xargs -n1 -P$(shell nproc) /bin/sh -c >$(BUILD_DIR)/unit_tests.log 2>&1 || sh -c "if command -v failed &> /dev/null; then failed; fi; cat $(BUILD_DIR)/unit_tests.log"

	@# @UBSAN_OPTIONS=print_stacktrace=1 sh -c "$(PROJECT_ROOT)/res/bin/testrunner.sh" # --no-python"
	@sh -c "if command -v passed &> /dev/null; then passed; fi"

system_tests: $(BUILD_DIR)/_blastpy.so
	@UBSAN_OPTIONS=print_stacktrace=1 TRACY_NO_INVARIANT_CHECK=1 sh -c "$(PROJECT_ROOT)/res/bin/testrunner.sh --long"
	@sh -c "if command -v passed &> /dev/null; then passed; fi"

test_asan:
	@sh -c "$(PROJECT_ROOT)/res/bin/testrunner.sh -asan"
	@sh -c "if command -v passed &> /dev/null; then passed; fi"

test_msan:
	@sh -c "$(PROJECT_ROOT)/res/bin/testrunner.sh -msan"
	@sh -c "if command -v passed &> /dev/null; then passed; fi"

test_inkscape:
	pytest -vv $(SRC_DIR)/inkscape

ut:	$(UT_C_EXES)
	echo $(UT_C_EXES) | xargs -n1 -P$(shell nproc) /bin/sh -c
