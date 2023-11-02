
tags:	
	@rm -f $(PROJECT_ROOT)/{.tags,.tagsextra} >/dev/null 2>/dev/null || /bin/true
	@$(MAKE) .tags

.tags:
	@(timeout 60.0s nice -n 19 ${GIT_HOOKS}/ctags >/dev/null 2>&1 &)

mxml_help:
	xdg-open "file:///$(SUBMODULES_DIR)/mxml/doc/mxml.html"


# Utilities

svg2bezier:	$(SRC_DIR)/scaffolding/svg2bezier.c
	# Relies on submodule nanosvg
	mkdir -p $(BUILD_DIR)
	$(CC) -I $(SUBMODULES_DIR) -o $(BUILD_DIR)/$@ -O3 $^ -lm

