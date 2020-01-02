# User defines
# DEFINES = SPACENAV
CPPFLAGS += ($(addprefix -D, $(DEFINES)))

# Project settings
PROJECT_ROOT=$(shell git rev-parse --show-toplevel)
GIT_HOOKS=${PROJECT_ROOT}/$(shell git config --get core.hooksPath)

# Common settings
# CC = ccache gcc
# CXX = ccache g++
CC = ccache clang
CXX = ccache clang++

CXXFLAGS = -fPIC -std=c++11 -Wall -Wextra
CXXFLAGS += -I src/libbp -I sub/pugixml/src -I sub/nanosvg/src -I ../../sub/CRCpp/inc
CXXFLAGS += -g -O0
CXXFLAGS += -Wpedantic 

# Octave has warnings, so we cannot add -Werror
IGNORE_ERROR_CXXFLAGS := ${CXXFLAGS}

CXXFLAGS += -Werror 
# CXXFLAGS += -fmax-errors=10 

QMAKE_CXXFLAGS = ${CXXFLAGS}
# QMAKE_CXXFLAGS_DEBUG = -O0
QMAKE_CONFIG = debug

LDFLAGS = -shared

# PROCESSOR_OPTS = -j$(shell nproc) -l$(shell nproc)

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

BLASTPIT_LIB = $(BUILDDIR)/libbp/libbp.so
# BLASTPIT_SRCS = src/libbp/blastpit.c # source files
# BLASTPIT_OBJS = $(BLASTPIT_SRCS:.cpp=.o)

BPCMD_SRCS = ${BUILDDIR}/libbp/_blastpit.so
# BPCMD_OUTPUT = ${BUILDDIR}/testchamber/blastpytest.py
BPCMD_OUTPUT = $(BLASTPIT_LIB)

SVGCMD_SRCS:=sub/pugixml/src/pugixml.cpp src/libbp/svg.cpp src/libbp/svgcmd.cpp #$(wildcard *.cpp)
SVGCMD_OUTPUT=$(BUILDDIR)/libbp/svgcmd

OCTAVECMD_SRCS:=src/libbp/octavecmd.cpp
OCTAVECMD_OUTPUT=$(BUILDDIR)/libbp/octavecmd

FORMAT_FILES = src/libbp/*.{h,c,cpp}
FORMATEXTRA_FILES = src/lmos-tray/{lmos,lmos-tray,parser}.{hpp,cpp}
# FORMAT_FILES += src/bpgui/*.{h,c,cpp} src/testchamber/*/*.{h,cpp} src/lmos-tray/{lmos,lmos-tray,parser}.{h,cpp} src/lmos-tray/main.cpp

FORMAT_FILES_PYTHON = src/libbp/*.py src/inkscape/*.py src/freecad/*.py
FORMAT_FILES_XML = src/inkscape/*.inx


# Tup
ifneq ("$(wildcard .tup)","")
TUP_CONDITIONAL := tuptest
else
TUP_CONDITIONAL := tupbear
endif

.PHONY:	tup_conditional tuptest
tup_conditional:
	make $(TUP_CONDITIONAL)
tuptest:
	make -C src/libbp
tupbear:
	make clean
	${GIT_HOOKS}/ctags >/dev/null 2>&1 &
	tup init
	tup generate res/ebuild/tup-make.sh
	bear tup
	make tuptest
standalone:
	make clean
	tup init
	tup generate res/ebuild/tup-make.sh


# Common build instructions
#
# See GNU Make manual 10.5.3 "Automatic Variables"

.PHONY:	all clean format doc blastpit

# We use bear to build the json if missing, otherwise build as normal (test)
ifneq ("$(wildcard compile_commands.json)","")
CONDITIONAL := test
else
CONDITIONAL := bear
endif

conditional:
	make $(CONDITIONAL)

test:	$(BPCMD_OUTPUT) #all
	tup init
	make tuptest
	# @echo -e "\nRunning Blastpit tests...\n"
	# @$(BUILDDIR)/testchamber/blastpit_test/blastpit_test -platform offscreen
	# @echo
	# @$(BUILDDIR)/testchamber/network_test/network_test -platform offscreen	
	# @echo
	# @$(BUILDDIR)/testchamber/parser_test/parser_test -platform offscreen
	@# @echo -e "\nRunning Python tests...\n"
	@# @src/testchamber/blastpy/blastpytest.py
	@# @echo "Running SVG tests..."
	@# @$(SVGCMD_OUTPUT) -t 
	@# @echo "Running Octave tests..."
	@# @$(OCTAVECMD_OUTPUT) 

all:	$(BPCMD_OUTPUT) #$(OCTAVECMD_OUTPUT) $(SVGCMD_OUTPUT) 
 
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
	rm -rf $(BUILDDIR)
	rm -f doc/man/man3/_home*
	rm -f compile_commands.json
	rm -f .git/tags .git/tagsextra
	find src -type f \( -name \*.o -o -name \*.so -o -name \*.so.* -o -name \*.stash -name \*.tup \) -delete -print
	find src/libbp -type f -name _x -delete -print
	rm -f src/libbp/lblastpit src/libbp/*_wrap*
	rm -rf .tup
	make -C src/libbp clean


tags:
	rm -f .git/tags .git/tagsextra
	.hooks/ctags

format:
	clang-format -style=file -i $(FORMAT_FILES) || /bin/true

formatextra:
	clang-format -style=file -i $(FORMATEXTRA_FILES) || /bin/true

formatpython:
	/bin/sh -c 'for file in $(FORMAT_FILES_XML); do xmllint --format --nsclean --output $$file $$file; done'
	autopep8 --global-config=$(PROJECT_ROOT)/res/cfg/pycodestyle --aggressive --aggressive -i $(FORMAT_FILES_PYTHON)
	flake8 --ignore=E402 --max-complexity=10 --show-source $(FORMAT_FILES_PYTHON)

unexpand:
	/bin/sh -c 'for file in $(FORMAT_FILES_PYTHON); do unexpand -t4 $$file > unexpanded.tmp; mv unexpanded.tmp $$file; done'

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

${BUILDDIR}/Makefile:
	mkdir -p $(BUILDDIR)
	/bin/sh -c 'cd $(BUILDDIR); qmake -qt=$(QT_VERSION) QMAKE_CXX="${CXX}" QMAKE_CXXFLAGS="${QMAKE_CXXFLAGS}" QMAKE_CXXFLAGS_DEBUG+="${QMAKE_CXXFLAGS_DEBUG}" CONFIG+="${QMAKE_CONFIG}" $(PWD)/src/blastpit.pro'

# $(BPCMD_OUTPUT): #$(BPCMD_SRCS)
	# mkdir -p $(BUILDDIR)/testchamber
	# cp src/testchamber/bpy/test.py ${BUILDDIR}/testchamber/
	# cp $(BUILDDIR)/libbp/_blastpit.so $(BUILDDIR)/testchamber/
	# echo cp src/libbp/blastpit.py $(BUILDDIR)/testchamber/

# $(BPCMD_SRCS): src/libbp/blastpit.i #$(BUILDDIR)/libbp/blastpit.o #$(BUILDDIR)/libbp/pugixml.o
# 	swig -python -o $(BUILDDIR)/libbp/blastpit_wrap.cxx src/libbp/blastpit.i 
# 	${CXX} ${QMAKE_CXXFLAGS} -fPIC -c $(BUILDDIR)/libbp/blastpit_wrap.cxx -Isrc/libbp $(shell python-config --includes) -I/usr/include/qt5 -I/usr/include/qt5/QtCore -I/usr/include/qt5/QtNetwork -o $(BUILDDIR)/libbp/blastpit_wrap.o
# 	${CXX} ${QMAKE_CXXFLAGS} -shared $(BUILDDIR)/libbp/blastpit_wrap.o $(BUILDDIR)/libbp/pugixml.o -o $(BUILDDIR)/libbp/_blastpit.so #$(BUILDDIR)/libbp/blastpit.o 

#$(BUILDDIR)/libbp/blastpit.o:	blastpit

$(BUILDDIR)/libbp/%.o:	src/libbp/%.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS)

$(BUILDDIR)/libbp/pugixml.o:	sub/pugixml/src/pugixml.cpp
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
	$(CXX) $(SVGCMD_SRCS) $(IGNORE_ERROR_CXXFLAGS) -L$(BUILDDIR)/libbp -o $@ -lcppunit -lbp


############
# Combined #
############

dummy:
	/bin/true

tup:
	# Tup variants (for separate build directories) don't work with bear
	make clean
	tup init
	bear tup


#$(BLASTPIT_LIB): blastpit #$(BLASTPIT_OBJS)
	#$(CXX) $(CXXFLAGS) ${LDFLAGS} -o $@ $^

# $(BLASTPIT_SRCS:.c=.d):%.d:%.c
	# $(CC) $(CFLAGS) -MM $< >$@

#include $(BLASTPIT_SRCS:.c=.d)

# Remember to remove the -rpath option when compiling the release version
#acamConnectTest: acamConnectTest.c libacamConnect.so
#	clang -L. -lcunit -lacamConnect -Wl,-rpath,. acamConnectTest.c -o acamConnectTest

