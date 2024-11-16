# Only create compilation database files when using clang
ifeq ($(CC), clang)
	DATABASE_CMD = -MJ $@.json
else
	DATABASE_CMD = ""
endif

$(BUILD_DIR)/sds.o:	$(SUBMODULES_DIR)/sds/sds.c | $(BUILD_DIR)
	$(CC) $(EXTERNAL_CPPFLAGS) $(SANFLAGS) $(INCFLAGS) -c -fPIC $^ -o $@

$(BUILD_DIR)/blastpy.py:	$(BUILD_DIR)/blastpy_wrap.c | $(BUILD_DIR)

$(BUILD_DIR)/blastpy_wrap.c:	$(LIBBLASTPIT_OBJS) # This forces rerunning swig on blastpit change
	swig -o $(BUILD_DIR)/blastpy_wrap.c -python $(PROJECT_ROOT)/src/libblastpit/blastpy.i

$(BUILD_DIR)/blastpy_wrap.o:	$(BUILD_DIR)/blastpy_wrap.c | $(BUILD_DIR)
	$(CC) -fPIC -I$(PROJECT_ROOT)/src/libblastpit -I$(SUBMODULES_DIR)/mongoose -o $@ -c $^ $(PYTHON_INCS)

$(BUILD_DIR)/_blastpy.so:	$(BUILD_DIR)/blastpy_wrap.o $(BLASTPY_FILES) $(TRACY_OBJS) $(BUILD_DIR)/libblastpit.a $(BUILD_DIR)/external_libs.a
	$(CXX) $(CPPFLAGS) -shared $(SANLDFLAGS) $(SANFLAGS) $(SHARED_SANFLAGS) $(BLASTPY_FILES) -o $@ $(BLASTPY_LIBS) $(BUILD_DIR)/libblastpit.a $(BUILD_DIR)/external_libs.a $(TRACY_OBJS)

$(BUILD_DIR)/unity_fixture.o: $(UNITY_FIXTURE_DIR)/unity_fixture.c | $(BUILD_DIR)
	$(CC) $(EXTERNAL_CPPFLAGS) $(SANFLAGS) $(UNITY_DEFS) $(INCFLAGS) -c $^ -o $@

$(BUILD_DIR)/unity.o:	$(UNITY_DIR)/unity.c | $(BUILD_DIR)
	$(CC) $(EXTERNAL_CPPFLAGS) $(SANFLAGS) $(UNITY_DEFS) $(INCFLAGS) -c $^ -o $@

# Ensure that TRACY_ENABLE is defined before building TracyClient.o,
# otherwise the file will build but without any symbols
$(BUILD_DIR)/TracyClient.o:	$(SUBMODULES_DIR)/tracy/public/TracyClient.cpp | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(INCFLAGS) -c -fPIC $^ -o $@

$(BUILD_DIR)/ut_%_x:	$(BUILD_DIR)/ut_%.o $(LIBBLASTPIT_OBJS) $(UNITY_OBJS) $(EXTERNAL_OBJS) $(TRACY_OBJS)
	$(CXX) $(CPPFLAGS) $(INCFLAGS) $(UNITY_DEFS) $(SANFLAGS) -L. $^ -o $@ $(LIBS) $(LDFLAGS)

$(BUILD_DIR)/ut_%.o:	$(LIBBLASTPIT_DIR)/ut_%.c | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(SANFLAGS) $(INCFLAGS) $(UNITY_DEFS) -c -fPIC $^ -o $@

$(BUILD_DIR)/%.o:	$(LIBBLASTPIT_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(DATABASE_CMD) $(CPPFLAGS) $(SANFLAGS) $(INCFLAGS) $(UNITY_DEFS) $(TRACY_INCFLAGS) -c -fPIC $^ -o $@

$(BUILD_DIR)/%.o:	$(SUBMODULES_DIR)/mongoose/%.c | $(BUILD_DIR)
	$(CC) $(DATABASE_CMD) $(EXTERNAL_CPPFLAGS) $(SANFLAGS) $(INCFLAGS) $(UNITY_DEFS) -c -fPIC $^ -o $@

$(BUILD_DIR)/%.o:	$(SUBMODULES_DIR)/mxml/%.c | $(BUILD_DIR)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(DATABASE_CMD) $(EXTERNAL_CPPFLAGS) $(SANFLAGS) $(INCFLAGS) -c -fPIC $^ -o $@
