# ░█▀▀░█▀█░█▀▄░█▄█░█▀█░▀█▀░▀█▀░▀█▀░█▀█░█▀▀
# ░█▀▀░█░█░█▀▄░█░█░█▀█░░█░░░█░░░█░░█░█░█░█
# ░▀░░░▀▀▀░▀░▀░▀░▀░▀░▀░░▀░░░▀░░▀▀▀░▀░▀░▀▀▀

format:
	clang-format -style=file:res/.clang-format -i $(FORMAT_FILES) || /bin/true
	black --line-length 80 $(FORMAT_FILES_PYTHON)

tidy:
	clang-tidy --config-file=$(PROJECT_ROOT)/res/.clang-tidy $(TIDY_FILES)

bangtidy:
	clang-tidy --fix --config-file=$(PROJECT_ROOT)/res/.clang-tidy $(TIDY_FILES)
