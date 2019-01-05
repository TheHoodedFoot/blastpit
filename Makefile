# User defines
DEFINES = SPACENAV
CPPFLAGS += ($(addprefix -D, $(DEFINES)))

# Common settings
# CC = ccache gcc
# CXX = ccache g++
CC = ccache clang
CXX = ccache clang++

CXXFLAGS = -fPIC -std=c++11 -Wall -Wextra
CXXFLAGS += -I src/libblastpit -I sub/pugixml/src -I sub/nanosvg/src -I ../../sub/CRCpp/inc
CXXFLAGS += -g -O0
CXXFLAGS += -Wpedantic 

# Octave has warnings, so we cannot add -Werror
IGNORE_ERROR_CXXFLAGS := ${CXXFLAGS}

CXXFLAGS += -Werror 
# CXXFLAGS += -fmax-errors=10 

QMAKE_CXXFLAGS = ${CXXFLAGS}
QMAKE_CXXFLAGS_DEBUG = -O0
QMAKE_CONFIG = debug

LDFLAGS = -shared

PROCESSOR_OPTS = -j$(shell nproc) -l$(shell nproc)

GRAPHDIR = doc

QT_VERSION = qt5

CMD :=	$(shell whoami)
PWD :=	$(shell pwd)

BUILDDIR = build

# Test target
TEST_CMD ?= build/testchamber/parser_test/parser_test

# Debugging
DEBUG_COMMAND ?= $(shell head -n1 .debugcmd)
DEBUG_TARGET ?= $(shell tail -n1 .debugcmd)

# gdb
# TEST_GDBINIT ?= src/testchamber/.gdbinit
# DEBUG_COMMAND = gdb -tui -x $(TEST_GDBINIT) $(TEST_CMD)
# DEBUG_COMMAND = lldb $(shell head -n1 .lldbinit | sed s/#//)
# CXXFLAGS += -fstandalone-debug # for lldb

OCTAVE_INCDIR = -I$(shell octave-config -p OCTINCLUDEDIR) -I$(shell octave-config -p OCTINCLUDEDIR)/..
OCTAVE_LIBDIR = -L$(shell octave-config -p OCTLIBDIR)
OCTAVE_LIBS = -loctave -loctinterp

# Package-specific settings

BLASTPIT_LIB = $(BUILDDIR)/libblastpit/libblastpit.a
BLASTPIT_SRCS = src/libblastpit/blastpit.cpp # source files
BLASTPIT_OBJS = $(BLASTPIT_SRCS:.cpp=.o)

BPCMD_SRCS = ${BUILDDIR}/libblastpit/_blastpit.so
BPCMD_OUTPUT = ${BUILDDIR}/testchamber/blastpytest.py

SVGCMD_SRCS:=sub/pugixml/src/pugixml.cpp src/libblastpit/svg.cpp src/libblastpit/svgcmd.cpp #$(wildcard *.cpp)
SVGCMD_OUTPUT=$(BUILDDIR)/libblastpit/svgcmd

OCTAVECMD_SRCS:=src/libblastpit/octavecmd.cpp
OCTAVECMD_OUTPUT=$(BUILDDIR)/libblastpit/octavecmd

FORMAT_FILES = src/{libblastpit,blastpitgui}/*.{h,cpp} src/testchamber/*/*.{h,cpp} src/lmos-tray/{lmos,lmos-tray,parser}.{h,cpp} src/lmos-tray/main.cpp
FORMAT_FILES_PYTHON = src/blastpy/blastpy.py src/inkscape/*.py src/testchamber/blastpy/blastpytest.py # bin/*.py src/freecad/*.py src/freecad/Blastpit/*.py 
FORMAT_FILES_XML = src/inkscape/*.inx

# Common build instructions
#
# See GNU Make manual 10.5.3 "Automatic Variables"

.PHONY:	all clean format doc blastpit

# We use bear to build the json if missing, otherwise build as normal (test)
ifneq ("$(wildcard compile_commands.json)","")
BEAR := test
else
BEAR := bear
endif

conditional:
	make $(BEAR)

test:	$(BPCMD_OUTPUT) #all
	@echo -e "\nRunning Blastpit tests...\n"
	@$(BUILDDIR)/testchamber/blastpit_test/blastpit_test -platform offscreen
	@echo
	@$(BUILDDIR)/testchamber/network_test/network_test -platform offscreen	
	@echo
	@$(BUILDDIR)/testchamber/parser_test/parser_test -platform offscreen
	@echo -e "\nRunning Python tests...\n"
	@src/testchamber/blastpy/blastpytest.py
	@# echo "Running SVG tests..."
	@# $(SVGCMD_OUTPUT) -t 
	@# echo "Running Octave tests..."
	@# $(OCTAVECMD_OUTPUT) 

all:	$(SVGCMD_OUTPUT) $(BPCMD_OUTPUT) $(OCTAVECMD_OUTPUT)

doc:	doxygen tex

%.o:	%.cpp
	$(CXX) -MJ $@.json -c -o $@ $< $(CXXFLAGS)
	# To joing the json files together:
	#	sed -e '1s/^/[\n/' -e '$s/,$/\n]/' *.o.json > compile_commands.json
	#$(CXX) -c -o $@ $< $(CXXFLAGS)

%.ps:	%.gv
	dot -Tps $< > $@

%.pdf:	%.ps
	ps2pdf $< $@

# moc_%.cpp: %.h
# 	moc $(DEFINES) $(INCPATH) $< -o $@

graph:	$(patsubst %,$(GRAPHDIR)/%.pdf, bp bp_database)

clean:
	rm -f doc/manual/*.{aux,idx,log,ptc,toc,ilg,ind}
	rm -f doc/tex/*.{aux,idx,log,ptc,toc,ilg,ind}
	rm -f doc/tex/faq.{faq,out,pdf} doc/manual/manual.pdf doc/tex/snip.pdf
	rm -rf *.o $(BUILDDIR) *.so *~ *.pdf doc/html doc/man
	rm -rf build*
	rm -f doc/man/man3/_home*
	rm -f compile_commands.json

format:
	/bin/sh -c 'for file in $(FORMAT_FILES_XML); do xmllint --format --nsclean --output $$file $$file; done'
	clang-format -style=file -i $(FORMAT_FILES)
	autopep8 --aggressive --aggressive -i $(FORMAT_FILES_PYTHON)
	flake8 --ignore=E402 --max-complexity=10 --show-source $(FORMAT_FILES_PYTHON)

doxygen:
	@/bin/sh -c 'cd doc; doxygen; xdg-open html/index.html'

tex:
	/bin/sh -c 'cd doc/manual; xelatex manual; makeindex manual.idx -s imports/styleind.ist; xelatex manual; xelatex manual'

debug:
	$(DEBUG_COMMAND) $(DEBUG_TARGET)
	
run:
	$(TEST_CMD)

analyse:	clean ${BUILDDIR}/Makefile
		scan-build $(MAKE) -C ${BUILDDIR} $(PROCESSOR_OPTS)
		$(BROWSER) /tmp/scan-build*

viewdoc:	
		$(BROWSER) doc/html/index.html

viewpdf:	
		xdg-open doc/manual/manual.pdf

bear:
	# Generate YouCompleteMe
	make clean
	bear make all
	bin/fix_cc_json.py > compile_commands.json.fixed
	mv compile_commands.json.fixed compile_commands.json


# Package-specific build instructions

############
# Blastpit #
############

blastpit:	${BUILDDIR}/Makefile
		$(MAKE) -C ${BUILDDIR} $(PROCESSOR_OPTS)

$(BLASTPIT_LIB):	blastpit

${BUILDDIR}/Makefile:
	mkdir -p $(BUILDDIR)
	/bin/sh -c 'cd $(BUILDDIR); qmake -qt=$(QT_VERSION) QMAKE_CXX="${CXX}" QMAKE_CXXFLAGS="${QMAKE_CXXFLAGS}" QMAKE_CXXFLAGS_DEBUG+="${QMAKE_CXXFLAGS_DEBUG}" CONFIG+="${QMAKE_CONFIG}" $(PWD)/src/blastpit.pro'

$(BPCMD_OUTPUT): $(BPCMD_SRCS) $(BUILDDIR)/libblastpit/_blastpit.so #$(BUILDDIR)/libblastpit/blastpit.py
	mkdir -p $(BUILDDIR)/testchamber
	cp src/testchamber/blastpy/blastpytest.py ${BUILDDIR}/testchamber/
	ln -fs ../libblastpit/_blastpit.so $(BUILDDIR)/testchamber/
	ln -fs ../libblastpit/blastpit.py $(BUILDDIR)/testchamber/

$(BPCMD_SRCS): $(BLASTPIT_LIB) src/libblastpit/blastpit.i $(BUILDDIR)/libblastpit/blastpit.o $(BUILDDIR)/libblastpit/layer.o $(BUILDDIR)/libblastpit/path.o $(BUILDDIR)/libblastpit/text.o $(BUILDDIR)/libblastpit/pugixml.o
	swig -python -c++ -o $(BUILDDIR)/libblastpit/blastpit_wrap.cxx src/libblastpit/blastpit.i 
	${CXX} ${QMAKE_CXXFLAGS} -fPIC -c $(BUILDDIR)/libblastpit/blastpit_wrap.cxx -Isrc/libblastpit $(shell python-config --includes) -I/usr/include/qt5 -I/usr/include/qt5/QtCore -I/usr/include/qt5/QtNetwork -o $(BUILDDIR)/libblastpit/blastpit_wrap.o
	${CXX} ${QMAKE_CXXFLAGS} -shared $(BUILDDIR)/libblastpit/blastpit_wrap.o $(BUILDDIR)/libblastpit/blastpit.o $(BUILDDIR)/libblastpit/layer.o $(BUILDDIR)/libblastpit/path.o $(BUILDDIR)/libblastpit/text.o $(BUILDDIR)/libblastpit/pugixml.o -o $(BUILDDIR)/libblastpit/_blastpit.so

$(BUILDDIR)/blastpit/%.o:	src/libblastpit/%.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS)

$(BUILDDIR)/blastpit/pugixml.o:	sub/pugixml/src/pugixml.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS)

bpicons:
	convert -background none -define icon:auto-resize=256,128,64,48,32,16 src/img/blastpit.svg src/img/tentacle.ico
	convert -background none src/img/blastpit.svg src/img/tentacle.png && convert -background none src/img/blastpit.svg src/img/tripmine.png

$(BLASTPIT_LIB):	blastpit

##########
# Octave #
##########

# Octave has its own CXXFLAGS because -Werror fails
$(OCTAVECMD_OUTPUT): $(OCTAVECMD_SRCS) 
	mkdir -p $(BUILDDIR)
	$(CXX) $(OCTAVECMD_SRCS) $(IGNORE_ERROR_CXXFLAGS) -o $@ $(OCTAVE_LIBDIR) $(OCTAVE_INCDIR) -lcppunit $(OCTAVE_LIBS)

#######
# Svg #
#######

$(SVGCMD_OUTPUT): $(SVGCMD_SRCS) $(BLASTPIT_LIB)
	$(CXX) $(SVGCMD_SRCS) $(IGNORE_ERROR_CXXFLAGS) -L$(BUILDDIR)/libblastpit -o $@ -lcppunit -lblastpit


############
# Combined #
############

dummy:
	/bin/true



#$(BLASTPIT_LIB): blastpit #$(BLASTPIT_OBJS)
	#$(CXX) $(CXXFLAGS) ${LDFLAGS} -o $@ $^

# $(BLASTPIT_SRCS:.c=.d):%.d:%.c
	# $(CC) $(CFLAGS) -MM $< >$@

#include $(BLASTPIT_SRCS:.c=.d)

# Remember to remove the -rpath option when compiling the release version
#acamConnectTest: acamConnectTest.c libacamConnect.so
#	clang -L. -lcunit -lacamConnect -Wl,-rpath,. acamConnectTest.c -o acamConnectTest

