GIT_DESCRIBE = $(shell git describe --tags --always --long)

.PHONY: doc
doc:
	(cat ${PROJECT_ROOT}/res/doc/Doxyfile; echo "PROJECT_NUMBER=$(GIT_DESCRIBE)") | doxygen -
