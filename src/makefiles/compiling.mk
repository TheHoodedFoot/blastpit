$(BUILD_DIR)/sds.o:	$(SRC_DIR)/sds/sds.c
	$(CC) $(EXTERNAL_CPPFLAGS) $(SANFLAGS) $(INCFLAGS) -c -fPIC $^ -o $@

$(BUILD_DIR)/blastpy.py:	$(BUILD_DIR)/blastpy_wrap.c

$(BUILD_DIR)/blastpy_wrap.c:	$(LIBBLASTPIT_OBJS) # This forces rerunning swig on blastpit change
	swig -o $(BUILD_DIR)/blastpy_wrap.c -python $(PROJECT_ROOT)/src/libblastpit/blastpy.i

$(BUILD_DIR)/blastpy_wrap.o:	$(BUILD_DIR)/blastpy_wrap.c
	$(CC) -fPIC -I$(PROJECT_ROOT)/src/libblastpit -I$(SUBMODULES_DIR)/mongoose -o $@ -c $^ $(PYTHON_INCS)

$(BUILD_DIR)/_blastpy.so:	$(BUILD_DIR)/blastpy_wrap.o $(BLASTPY_FILES)
	$(CC) $(CPPFLAGS) -shared $(SANLDFLAGS) $(SANFLAGS) $(SHARED_SANFLAGS) $(BLASTPY_FILES) -o $@ $(BLASTPY_LIBS) $(LIBBLASTPIT_OBJS)

$(BUILD_DIR)/unity_fixture.o: $(UNITY_FIXTURE_DIR)/unity_fixture.c
	$(CC) $(EXTERNAL_CPPFLAGS) $(SANFLAGS) $(UNITY_DEFS) $(INCFLAGS) -c $^ -o $@

$(BUILD_DIR)/unity.o:	$(UNITY_DIR)/unity.c
	$(CC) $(EXTERNAL_CPPFLAGS) $(SANFLAGS) $(UNITY_DEFS) $(INCFLAGS) -c $^ -o $@

$(BUILD_DIR)/TracyClient.o:	$(SUBMODULES_DIR)/tracy/public/TracyClient.cpp
	$(CXX) $(CPPFLAGS) $(INCFLAGS) -c -fPIC $^ -o $@

$(BUILD_DIR)/ut_%_x:	$(BUILD_DIR)/ut_%.o $(LIBBLASTPIT_OBJS) $(UNITY_OBJS) $$(TRACY_OBJS)
	$(CC) $(CPPFLAGS) $(INCFLAGS) $(UNITY_DEFS) $(SANFLAGS) -L. $^ -o $@ $(LIBS) $(LDFLAGS)

$(BUILD_DIR)/ut_%.o:	$(LIBBLASTPIT_DIR)/ut_%.c
	$(CC) $(CPPFLAGS) $(SANFLAGS) $(INCFLAGS) $(UNITY_DEFS) -c -fPIC $^ -o $@

$(BUILD_DIR)/%.o:	$(LIBBLASTPIT_DIR)/%.c
	$(CC) $(CPPFLAGS) $(SANFLAGS) $(INCFLAGS) $(UNITY_DEFS) -c -fPIC $^ -o $@

$(BUILD_DIR)/%.o:	$(SUBMODULES_DIR)/mongoose/%.c
	$(CC) $(EXTERNAL_CPPFLAGS) $(SANFLAGS) $(INCFLAGS) $(UNITY_DEFS) -c -fPIC $^ -o $@

$(BUILD_DIR)/%.o:	$(SUBMODULES_DIR)/mxml/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(EXTERNAL_CPPFLAGS) $(SANFLAGS) $(INCFLAGS) -c -fPIC $^ -o $@
