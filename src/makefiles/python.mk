# Python inclues for SWIG
PYTHON_INCS = $(shell python-config --includes)

python_targets:	$(BUILD_DIR)/_blastpy.so $(BUILD_DIR)/blastpy.py

python:	$(BUILD_DIR)/blastpy.py $(BUILD_DIR)/_blastpy.so
	$(MAKE) -C $(BUILD_DIR) -f $(PROJECT_ROOT)/Makefile python_install

python_install:
	python $(LIBBLASTPIT_DIR)/setup.py install --user


